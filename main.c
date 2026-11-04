/**
 * @file main.c
 * @brief Simulador Principal do Sistema Operacional para o chip Antikit-8.
 * 
 * Integra os modulos dos tres integrantes:

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "hardware_specs.h"
#include "mmu_manager.h"
#include "page_replacement.h"
#include "audit_metrics.h"

int main(int argc, char *argv[]) {
    const char *trace_filename = "memory_trace.txt";
    bool naive_mode = false;
    bool interactive_step = false;

    /* Leitura de parametros de linha de comando */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--naive") == 0) {
            naive_mode = true;
        } else if (strcmp(argv[i], "--step") == 0) {
            interactive_step = true;
        } else if (argv[i][0] != '-') {
            trace_filename = argv[i];
        }
    }

    printf("============================================================\n");
    printf("         INICIANDO SISTEMA OPERACIONAL ANTIKIT-8           \n");
    printf("============================================================\n");
    printf("Arquivo de trace: %s\n", trace_filename);
    printf("Modo de Substituicao: %s\n", naive_mode ? "INEFICIENTE (Sem Controle Termico)" : "EFICIENTE (WSClock Termico Inteligente)");
    if (interactive_step) {
        printf("Modo Interativo ativado (pausa a cada 10 acessos)\n");
    }
    printf("============================================================\n\n");

    /* 1. Inicializacao do Hardware Simulado */
    hardware_state_t hw;
    hardware_init(&hw);

    /* 2. Inicializacao dos Quadros Fisicos da RAM (256 KB = 32 slots de 8 KB) */
    physical_frame_t ram_frames[TOTAL_RAM_FRAMES];
    mmu_init_physical_ram(ram_frames, TOTAL_RAM_FRAMES);

    /* 3. Inicializacao das Tabelas de Paginas dos Processos */
    page_table_entry_t page_tables[MAX_PROCESSES][TOTAL_VIRTUAL_PAGES];
    for (int p = 0; p < MAX_PROCESSES; p++) {
        mmu_init_page_table(page_tables[p], TOTAL_VIRTUAL_PAGES);
    }

    /* 4. Inicializacao do Algoritmo de Substituicao WSClock */
    wsclock_state_t ws;
    wsclock_init(&ws, DEFAULT_TAU, naive_mode);

    /* 5. Inicializacao do Modulo de Auditoria e Metricas */
    audit_metrics_t metrics;
    audit_init(&metrics);

    /* Abertura do arquivo de trace de memoria */
    FILE *trace_file = fopen(trace_filename, "r");
    if (trace_file == NULL) {
        /* Fallback para execucao dentro de subpastas de build (como cmake-build-debug do CLion) */
        char alt_path[512];
        snprintf(alt_path, sizeof(alt_path), "../%s", trace_filename);
        trace_file = fopen(alt_path, "r");
        if (trace_file == NULL) {
            fprintf(stderr, "Erro ao abrir o arquivo de trace: '%s' (nem em '%s')\n", trace_filename, alt_path);
            return 1;
        }
        trace_filename = alt_path;
    }

    char line_buffer[256];
    uint64_t line_counter = 0;

    /* Processamento linha por linha do arquivo de trace */
    while (fgets(line_buffer, sizeof(line_buffer), trace_file) != NULL) {
        trace_entry_t entry;
        if (!trace_parse_line(line_buffer, &entry)) {
            continue; /* Pula linhas vazias ou comentarios */
        }

        line_counter++;
        metrics.total_memory_accesses++;

        /* Validacao do ID do processo */
        if (entry.process_id >= MAX_PROCESSES) {
            fprintf(stderr, "Aviso: PID %u excede o maximo suportado (%d). Ajustado para 0.\n",
                    entry.process_id, MAX_PROCESSES - 1);
            entry.process_id = 0;
        }

        /* Integrante 1: Traducao do endereco virtual e limites da paginacao assimetrica */
        uint32_t vpn = 0;
        uint32_t offset = 0;
        uint32_t page_size = 0;
        if (!mmu_translate_address(entry.virtual_address, &vpn, &offset, &page_size)) {
            fprintf(stderr, "[ERRO MMU] Endereco virtual 0x%X fora do limite de 1MB!\n", entry.virtual_address);
            continue;
        }

        /* Tentativa de acesso a memoria via MMU de hardware */
        bool hit = mmu_access_memory(&hw, 
                                     page_tables[entry.process_id], 
                                     TOTAL_VIRTUAL_PAGES, 
                                     entry.virtual_address, 
                                     entry.is_write, 
                                     entry.timestamp);

        /* Integrante 3: Auditoria do Barramento - Verificacao do Fading do Bit R */
        audit_check_fading_event(&hw, page_tables[entry.process_id], TOTAL_VIRTUAL_PAGES, &metrics);

        /* Se ocorreu Page Fault (MMU retornou false) */
        if (!hit) {
            metrics.total_page_faults++;
            metrics.faults_per_process[entry.process_id]++;

            /* Define bytes usados para calculo de fragmentacao interna */
            uint32_t bytes_used = (entry.bytes_requested > 0) ? entry.bytes_requested : page_size;

            /* Integrante 2: Tratamento da interrupcao de Page Fault com WSClock */
            uint32_t allocated_frame = handle_page_fault(&ws, 
                                                         &hw, 
                                                         ram_frames, 
                                                         page_tables[entry.process_id], 
                                                         entry.process_id, 
                                                         vpn, 
                                                         bytes_used, 
                                                         entry.timestamp);

            /* Integrante 3: Auditoria Anti-Fraude com hash acumulado */
            audit_log_state_change(&metrics, 
                                   page_tables[entry.process_id], 
                                   TOTAL_VIRTUAL_PAGES, 
                                   "PAGE_FAULT_RESOLVED");

            /* Apos mapear na RAM, refaz o acesso a MMU para concluir a instrucao */
            mmu_access_memory(&hw, 
                              page_tables[entry.process_id], 
                              TOTAL_VIRTUAL_PAGES, 
                              entry.virtual_address, 
                              entry.is_write, 
                              entry.timestamp);

            (void)allocated_frame;
        }

        /* Integrante 3: Visualizador ASCII de fragmentacao a cada 10 acessos */
        if (metrics.total_memory_accesses % 10 == 0) {
            audit_render_ascii_fragmentation(ram_frames, TOTAL_RAM_FRAMES, &metrics, metrics.total_memory_accesses);
            if (interactive_step) {
                printf("Pressione [ENTER] para continuar o trace...");
                getchar();
            }
        }
    }

    fclose(trace_file);

    /* Atualiza a temperatura de pico registrada */
    if (ws.max_temp_recorded > metrics.peak_swap_temperature) {
        metrics.peak_swap_temperature = ws.max_temp_recorded;
    }
    if (hw.swap_temperature > metrics.peak_swap_temperature) {
        metrics.peak_swap_temperature = hw.swap_temperature;
    }

    /* Integrante 3: Exibe o relatorio final consolidado */
    audit_print_final_report(&metrics, &hw);

    return 0;
}
