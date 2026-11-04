/**
 * @file audit_metrics.c
 * @brief Implementacao do Modulo do Integrante 3 (Auditor e Analista de Metricas).
 */

#include "audit_metrics.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void audit_init(audit_metrics_t *metrics) {
    if (metrics == NULL) return;

    metrics->total_memory_accesses = 0;
    metrics->total_page_faults = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        metrics->faults_per_process[i] = 0;
    }
    metrics->peak_swap_temperature = BASE_SWAP_TEMP;
    metrics->total_io_cpu_cycles = 0;
    metrics->fragmentation_samples_sum = 0.0;
    metrics->fragmentation_sample_count = 0;
    metrics->accumulated_checksum = 0x811C9DC5u; /* Seed FNV-1 */
}

uint32_t audit_compute_page_table_hash(page_table_entry_t *page_table, uint32_t num_entries) {
    if (page_table == NULL) return 0;

    /* Algoritmo FNV-1a de 32 bits sobre os bits present e frame_number */
    uint32_t hash = 2166136261u;
    for (uint32_t i = 0; i < num_entries; i++) {
        uint32_t present_bit = page_table[i].present ? 1u : 0u;
        uint32_t frame_val = page_table[i].frame_number;
        uint32_t entry_data = (present_bit << 31) | (frame_val & 0x7FFFFFFF);

        /* Hash por byte da entrada */
        for (int b = 0; b < 4; b++) {
            uint8_t byte = (uint8_t)((entry_data >> (b * 8)) & 0xFF);
            hash ^= byte;
            hash *= 16777619u;
        }
    }
    return hash;
}

void audit_log_state_change(audit_metrics_t *metrics, 
                            page_table_entry_t *page_table, 
                            uint32_t num_entries, 
                            const char *reason) {
    
    if (metrics == NULL || page_table == NULL) return;

    uint32_t current_hash = audit_compute_page_table_hash(page_table, num_entries);
    
    /* Atualiza o hash acumulado historico */
    metrics->accumulated_checksum = (metrics->accumulated_checksum * 33) ^ current_hash;

    printf("[AUDITORIA ANTI-FRAUDE] Evento: %-19s | Hash Atual: 0x%08X | Checksum Acumulado: 0x%08X\n",
           reason, current_hash, metrics->accumulated_checksum);
}

void audit_check_fading_event(hardware_state_t *hw, 
                             page_table_entry_t *page_table, 
                             uint32_t num_entries, 
                             audit_metrics_t *metrics) {
    
    if (hw == NULL || page_table == NULL || metrics == NULL) return;

    /* A MMU executa fading a cada FADING_THRESHOLD (5) acessos globais */
    if (hw->global_access_count > 0 && (hw->global_access_count % FADING_THRESHOLD == 0)) {
        printf(">>> [FADING DO BIT R] Anomalia eletrica detectada no acesso global #%llu!\n",
               hw->global_access_count);
        audit_log_state_change(metrics, page_table, num_entries, "FADING_DETECTED");
    }
}

void audit_render_ascii_fragmentation(physical_frame_t *frames, 
                                      uint32_t num_frames, 
                                      audit_metrics_t *metrics, 
                                      uint64_t step_number) {
    
    if (frames == NULL) return;

    printf("\n================ [MAPA ASCII DA MEMORIA RAM - ACESSO #%llu] ================\n", step_number);
    printf("Slot de 8KB: cada caractere representa 1KB ('U' = Usado, 'F' = Fragmentado, '.' = Livre)\n");

    uint64_t total_allocated_bytes = 0;
    uint64_t total_wasted_bytes = 0;
    uint32_t occupied_frames = 0;

    for (uint32_t i = 0; i < num_frames; i++) {
        char slot_representation[9];
        slot_representation[8] = '\0';

        if (!frames[i].occupied) {
            strcpy(slot_representation, "........");
        } else {
            occupied_frames++;
            total_allocated_bytes += FRAME_SIZE;

            uint32_t used_bytes = frames[i].bytes_allocated;
            if (used_bytes > FRAME_SIZE) used_bytes = FRAME_SIZE;
            
            uint32_t wasted = FRAME_SIZE - used_bytes;
            total_wasted_bytes += wasted;

            uint32_t used_kb = (used_bytes + 1023) / 1024; /* Arredondamento superior para KB */
            if (used_kb > 8) used_kb = 8;

            for (uint32_t k = 0; k < 8; k++) {
                if (k < used_kb) {
                    slot_representation[k] = 'U';
                } else {
                    slot_representation[k] = 'F';
                }
            }
        }

        printf("Frame[%02u]: [%s]  ", i, slot_representation);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }

    double current_fragmentation = 0.0;
    if (total_allocated_bytes > 0) {
        current_fragmentation = ((double)total_wasted_bytes / (double)total_allocated_bytes) * 100.0;
    }

    if (metrics != NULL) {
        metrics->fragmentation_samples_sum += current_fragmentation;
        metrics->fragmentation_sample_count++;
    }

    printf("Quadros Ocupados: %u/%u | Desperdicio Interno Atual: %.2f%%\n",
           occupied_frames, num_frames, current_fragmentation);
    printf("==============================================================================\n\n");
}

bool trace_parse_line(const char *line, trace_entry_t *entry) {
    if (line == NULL || entry == NULL) return false;

    /* Ignora espacos em branco iniciais */
    while (*line && isspace((unsigned char)*line)) {
        line++;
    }

    /* Linhas vazias ou comentarios com # */
    if (*line == '\0' || *line == '#' || *line == '\n' || *line == '\r') {
        return false;
    }

    char op_char = 'R';
    uint32_t bytes_arg = 0;

    int parsed = sscanf(line, "%u %u %u %c %u", 
                        &entry->timestamp, 
                        &entry->process_id, 
                        &entry->virtual_address, 
                        &op_char, 
                        &bytes_arg);

    if (parsed >= 4) {
        entry->is_write = (op_char == 'W' || op_char == 'w');
        entry->bytes_requested = (parsed == 5) ? bytes_arg : 0;
        return true;
    }

    return false;
}

void audit_print_final_report(audit_metrics_t *metrics, hardware_state_t *hw) {
    if (metrics == NULL || hw == NULL) return;

    /* Calcula fragmentacao media */
    double avg_fragmentation = 0.0;
    if (metrics->fragmentation_sample_count > 0) {
        avg_fragmentation = metrics->fragmentation_samples_sum / metrics->fragmentation_sample_count;
    }

    /* Ciclos gastos em E/S = Total de ciclos de hardware menos os acessos simples de 1 ciclo a MMU */
    uint64_t io_cycles = 0;
    if (hw->total_cpu_cycles >= hw->global_access_count) {
        io_cycles = hw->total_cpu_cycles - hw->global_access_count;
    }

    float final_peak_temp = (metrics->peak_swap_temperature > hw->swap_temperature) ?
                            metrics->peak_swap_temperature : hw->swap_temperature;

    printf("\n");
    printf("============================================================\n");
    printf("         RELATORIO FINAL DE EXECUCAO - ANTIKIT-8 OS        \n");
    printf("============================================================\n");
    printf(" - O numero total de acessos a memoria: %llu\n", metrics->total_memory_accesses);
    printf(" - O numero total de Page Faults gerados (global): %llu\n", metrics->total_page_faults);
    for (int p = 0; p < MAX_PROCESSES; p++) {
        if (metrics->faults_per_process[p] > 0) {
            printf("     * Processo PID %d: %llu page faults\n", p, metrics->faults_per_process[p]);
        }
    }
    printf(" - A Temperatura Maxima alcancada pelo Swap: %.2f C\n", final_peak_temp);
    printf(" - O total de Ciclos de Clock desperdicados em operacoes de E/S: %llu ciclos\n", io_cycles);
    printf(" - A porcentagem media de Fragmentacao Interna da memoria RAM fisica: %.2f%%\n", avg_fragmentation);
    printf(" - Checksum de Auditoria Anti-Fraude Final: 0x%08X\n", metrics->accumulated_checksum);
    printf("============================================================\n\n");
}
