/**
 * @file audit_metrics.h
 * @brief Modulo do Integrante 3 (Auditor e Analista de Metricas).
 * 
 * Parser de trace de memoria, modulo anti-fraude com checksum
 * da tabela de paginas, monitoramento do fading do bit R,
 * mapa visual ASCII de fragmentacao e gerador de relatorio final.
 */

#ifndef AUDIT_METRICS_H
#define AUDIT_METRICS_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware_specs.h"
#include "mmu_manager.h"

#define MAX_PROCESSES 16

/**
 * @brief Estrutura que armazena os dados consolidados de uma linha de trace.
 */
typedef struct {
    uint32_t timestamp;        /* Timestamp virtual do acesso */
    uint32_t process_id;      /* ID do processo requisitante */
    uint32_t virtual_address; /* Endereco virtual acessado */
    bool is_write;            /* true para Escrita (W), false para Leitura (R) */
    uint32_t bytes_requested; /* Quantidade de bytes solicitados (opcional para teste 4) */
} trace_entry_t;

/**
 * @brief Estrutura para consolidacao de todas as metricas da execucao.
 */
typedef struct {
    uint64_t total_memory_accesses;        /* Total global de acessos a memoria */
    uint64_t total_page_faults;           /* Total global de Page Faults */
    uint64_t faults_per_process[MAX_PROCESSES]; /* Page Faults discriminados por processo */
    float peak_swap_temperature;          /* Temperatura maxima atingida pelo Swap */
    uint64_t total_io_cpu_cycles;         /* Ciclos de CPU gastos exclusivamente com E/S */
    double fragmentation_samples_sum;     /* Acumulador de amostras de fragmentacao */
    uint32_t fragmentation_sample_count;  /* Quantidade de medicoes de fragmentacao realizadas */
    uint32_t accumulated_checksum;        /* Checksum acumulado de auditoria da tabela */
} audit_metrics_t;

/**
 * @brief Inicializa o coletor de metricas e auditoria.
 * @param metrics Ponteiro para estrutura de metricas.
 */
void audit_init(audit_metrics_t *metrics);

/**
 * @brief Calcula a soma de verificacao (checksum) atual da tabela de paginas.
 * 
 * Avalia todos os bits present e frame_number das entradas da tabela.
 * 
 * @param page_table Vetor de entradas da tabela de paginas.
 * @param num_entries Quantidade de entradas (171).
 * @return uint32_t Valor do hash/checksum calculado.
 */
uint32_t audit_compute_page_table_hash(page_table_entry_t *page_table, uint32_t num_entries);

/**
 * @brief Atualiza e audita o estado da tabela de paginas, exibindo log de seguranca.
 * 
 * @param metrics Ponteiro para estrutura de metricas.
 * @param page_table Tabela de paginas.
 * @param num_entries Quantidade de entradas.
 * @param reason Motivo da alteracao (ex: "PAGE_FAULT_MAPPED", "FADING_EVENT").
 */
void audit_log_state_change(audit_metrics_t *metrics, 
                            page_table_entry_t *page_table, 
                            uint32_t num_entries, 
                            const char *reason);

/**
 * @brief Detecta e loga a ocorrencia do Fading do bit R apos acessos da MMU.
 * 
 * @param hw Estado atual do hardware.
 * @param page_table Tabela de paginas.
 * @param num_entries Quantidade de entradas.
 * @param metrics Metricas de auditoria.
 */
void audit_check_fading_event(hardware_state_t *hw, 
                             page_table_entry_t *page_table, 
                             uint32_t num_entries, 
                             audit_metrics_t *metrics);

/**
 * @brief Exibe no terminal o mapa ASCII de ocupacao da RAM e calcula fragmentacao.
 * Formato exemplificado no enunciado: [UUUUFFFF] (U = Usado, F = Fragmentado).
 * 
 * @param frames Vetor de quadros fisicos da RAM.
 * @param num_frames Quantidade total de frames (32).
 * @param metrics Estrutura de metricas para acumular estatisticas.
 * @param step_number Numero da linha/passo atual do trace.
 */
void audit_render_ascii_fragmentation(physical_frame_t *frames, 
                                      uint32_t num_frames, 
                                      audit_metrics_t *metrics, 
                                      uint64_t step_number);

/**
 * @brief Faz o parse de uma linha do arquivo memory_trace.txt.
 * Suporta formatos:
 *  - "timestamp pid virtual_address R/W"
 *  - "timestamp pid virtual_address R/W bytes_used"
 * 
 * @param line String lida do arquivo.
 * @param entry Ponteiro para salvar os dados extraidos.
 * @return true se a linha foi interpretada com sucesso, false caso contrario.
 */
bool trace_parse_line(const char *line, trace_entry_t *entry);

/**
 * @brief Imprime o relatorio final no formato estrito exigido pelo enunciado.
 * 
 * @param metrics Estrutura com todas as metricas coletadas.
 * @param hw Estado final do hardware Antikit-8.
 */
void audit_print_final_report(audit_metrics_t *metrics, hardware_state_t *hw);

#endif /* AUDIT_METRICS_H */
