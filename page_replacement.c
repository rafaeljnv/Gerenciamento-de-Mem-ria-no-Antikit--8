/**
 * @file page_replacement.c
 * @brief Implementacao do Modulo do Integrante 2 (Engenheiro de Algoritmos).
 * 
 * Contem a logica do WSClock adaptada com sensibilidade termica
 * para o subsistema de Swap do Antikythera-8.
 */

#include "page_replacement.h"
#include <stdio.h>
#include <stdlib.h>

void wsclock_init(wsclock_state_t *ws, uint32_t tau, bool naive_mode) {
    if (ws == NULL) return;

    ws->clock_hand = 0;
    ws->tau = (tau > 0) ? tau : DEFAULT_TAU;
    ws->naive_thermal_mode = naive_mode;
    ws->total_page_outs = 0;
    ws->max_temp_recorded = BASE_SWAP_TEMP;
}

uint32_t wsclock_select_victim(wsclock_state_t *ws, 
                              hardware_state_t *hw, 
                              physical_frame_t *frames, 
                              uint32_t num_frames, 
                              page_table_entry_t *page_table, 
                              uint32_t current_timestamp) {
    
    if (ws == NULL || frames == NULL || page_table == NULL || num_frames == 0) {
        return 0;
    }

    /* Cenário Ineficiente (para demonstracao do Teste 2 / codigo ruim sem protecao) */
    if (ws->naive_thermal_mode) {
        /* No modo ingenuo, busca o primeiro frame sujo ou qualquer frame sem escalonamento termico */
        for (uint32_t i = 0; i < num_frames; i++) {
            uint32_t idx = (ws->clock_hand + i) % num_frames;
            uint32_t vpn = frames[idx].vpn;
            if (page_table[vpn].modified) {
                ws->clock_hand = (idx + 1) % num_frames;
                return idx;
            }
        }
        uint32_t chosen = ws->clock_hand;
        ws->clock_hand = (ws->clock_hand + 1) % num_frames;
        return chosen;
    }

    /* --- CENARIO EFICIENTE: WSCLOCK COM PROTECAO TERMICA --- */
    int32_t best_clean_victim = -1;
    int32_t best_dirty_victim = -1;

    /* Primeira volta do ponteiro: varre procurando pagina limpa fora do Working Set */
    for (uint32_t i = 0; i < num_frames; i++) {
        uint32_t idx = (ws->clock_hand + i) % num_frames;
        if (!frames[idx].occupied) {
            ws->clock_hand = (idx + 1) % num_frames;
            return idx;
        }

        uint32_t vpn = frames[idx].vpn;

        /* Se o bit R == 1, damos uma segunda chance limpando o bit R */
        if (page_table[vpn].referenced) {
            page_table[vpn].referenced = false;
        } else {
            /* R == 0: Avalia a idade da pagina no Working Set */
            uint32_t age = (current_timestamp >= page_table[vpn].last_access_t) ? 
                           (current_timestamp - page_table[vpn].last_access_t) : 0;

            if (age > ws->tau) {
                /* A pagina esta fora do Working Set */
                if (!page_table[vpn].modified) {
                    /* Pagina limpa (M == 0): vitima perfeita! */
                    ws->clock_hand = (idx + 1) % num_frames;
                    return idx;
                } else {
                    /* Pagina suja (M == 1): guarda como candidata se nao houver limpa */
                    if (best_dirty_victim == -1) {
                        best_dirty_victim = (int32_t)idx;
                    }
                }
            } else {
                /* Pagina dentro do Working Set com R == 0 */
                if (!page_table[vpn].modified && best_clean_victim == -1) {
                    best_clean_victim = (int32_t)idx;
                }
            }
        }
    }

    /* Se encontrou uma pagina limpa qualquer, prefere ela a ter que gravar no disco */
    if (best_clean_victim != -1) {
        ws->clock_hand = ((uint32_t)best_clean_victim + 1) % num_frames;
        return (uint32_t)best_clean_victim;
    }

    /* Se precisa expulsar uma pagina suja */
    uint32_t chosen = (best_dirty_victim != -1) ? (uint32_t)best_dirty_victim : ws->clock_hand;
    ws->clock_hand = (chosen + 1) % num_frames;

    /* Controle Termico: Se a gravacao no Swap for estourar o limite de 65 C,
     * aplicamos resfriamento passivo preventivo antes de despejar */
    if (hw != NULL) {
        float projected_temp = hw->swap_temperature + 15.0f;
        if (projected_temp > MAX_SAFE_TEMP) {
            /* Calcula quanto tempo ocioso seria necessario para resfriar abaixo de 50 C */
            float needed_drop = projected_temp - 50.0f;
            if (needed_drop > 0) {
                uint32_t cooling_ms = (uint32_t)(needed_drop / 0.5f) + 1;
                hardware_cool_down(hw, cooling_ms);
            }
        }
    }

    return chosen;
}

uint32_t handle_page_fault(wsclock_state_t *ws, 
                           hardware_state_t *hw, 
                           physical_frame_t *frames, 
                           page_table_entry_t *page_table, 
                           uint32_t process_id, 
                           uint32_t vpn, 
                           uint32_t bytes_used, 
                           uint32_t current_timestamp) {
    
    if (frames == NULL || page_table == NULL) return 0;

    /* 1. Procura se ha um quadro livre na RAM fisica */
    int32_t free_frame = mmu_find_free_frame(frames, TOTAL_RAM_FRAMES);
    uint32_t target_frame = 0;

    if (free_frame != -1) {
        /* Há espaço livre: aloca direto sem precisar expulsar ninguém */
        target_frame = (uint32_t)free_frame;
    } else {
        /* RAM cheia: executa o algoritmo de substituicao WSClock */
        target_frame = wsclock_select_victim(ws, hw, frames, TOTAL_RAM_FRAMES, page_table, current_timestamp);

        /* Se o frame escolhido estava ocupado, desmapeia a pagina anterior */
        if (frames[target_frame].occupied) {
            uint32_t old_vpn = frames[target_frame].vpn;

            /* Se a pagina vitima estava modificada (suja), realiza o Page Out */
            if (page_table[old_vpn].modified) {
                if (hw != NULL) {
                    hardware_trigger_page_out(hw, current_timestamp);
                    if (ws != NULL) {
                        ws->total_page_outs++;
                        if (hw->swap_temperature > ws->max_temp_recorded) {
                            ws->max_temp_recorded = hw->swap_temperature;
                        }
                    }
                }
            }

            /* Desmapeia a pagina vitima da RAM */
            mmu_unmap_page(frames, page_table, target_frame);
        }
    }

    /* 2. Mapeia a nova pagina no quadro alocado */
    mmu_map_page(frames, page_table, target_frame, process_id, vpn, bytes_used);
    page_table[vpn].last_access_t = current_timestamp;

    return target_frame;
}
