/**
 * @file page_replacement.h
 * @brief Modulo do Integrante 2 (Engenheiro de Algoritmos).
 * 
 * Implementacao do algoritmo de substituicao de paginas WSClock
 * adaptado com controle termico do dispositivo de Swap, evitando
 * o Thermal Throttling (> 65 C) e minimizando ciclos de CPU.
 */

#ifndef PAGE_REPLACEMENT_H
#define PAGE_REPLACEMENT_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware_specs.h"
#include "mmu_manager.h"

/* Limite padrao de idade (tau) do Working Set Clock */
#define DEFAULT_TAU 30

/**
 * @brief Estrutura de estado do algoritmo de substituicao WSClock.
 */
typedef struct {
    uint32_t clock_hand;       /* Ponteiro circular para os quadros da RAM */
    uint32_t tau;              /* Limite de tempo do conjunto de trabalho */
    bool naive_thermal_mode;   /* Se true, desativa a protecao termica (para demonstrar cenario ineficiente do Teste 2) */
    uint32_t total_page_outs;  /* Total de operacoes de escrita em disco realizadas */
    float max_temp_recorded;   /* Temperatura maxima registrada durante a execucao */
} wsclock_state_t;

/**
 * @brief Inicializa o estado do algoritmo de substituicao WSClock.
 * @param ws Estado do WSClock.
 * @param tau Janela de tempo de referencia (tau).
 * @param naive_mode Se true, simula substituicao ingenua sem controle termico.
 */
void wsclock_init(wsclock_state_t *ws, uint32_t tau, bool naive_mode);

/**
 * @brief Seleciona uma pagina vitima para substituicao e trata o Swap termico.
 * 
 * Percorre os quadros fisicos da RAM avaliando os bits R (referenciado) e M (modificado),
 * bem como a idade da pagina (current_time - last_access_t > tau). Prioriza paginas limpas
 * e gerencia o resfriamento do Swap antes de escrever paginas sujas para nao estourar 65 C.
 * 
 * @param ws Estado do WSClock.
 * @param hw Estado do hardware (Swap, ciclos, temperatura).
 * @param frames Vetor de frames fisicos da RAM.
 * @param num_frames Quantidade total de frames (32).
 * @param page_table Tabela de paginas do processo atual.
 * @param current_timestamp Timestamp virtual atual do acesso.
 * @return uint32_t Indice do quadro fisico (frame) escolhido como vitima.
 */
uint32_t wsclock_select_victim(wsclock_state_t *ws, 
                              hardware_state_t *hw, 
                              physical_frame_t *frames, 
                              uint32_t num_frames, 
                              page_table_entry_t *page_table, 
                              uint32_t current_timestamp);

/**
 * @brief Executa o tratamento completo do Page Fault com substituicao de pagina.
 * 
 * @param ws Estado do WSClock.
 * @param hw Estado do hardware.
 * @param frames Vetor de frames fisicos.
 * @param page_table Tabela de paginas do processo.
 * @param process_id ID do processo que requisitou a memoria.
 * @param vpn Numero da pagina virtual que causou a falha.
 * @param bytes_used Quantidade de bytes alocados na pagina.
 * @param current_timestamp Timestamp virtual do acesso.
 * @return uint32_t Indice do frame onde a nova pagina foi alocada.
 */
uint32_t handle_page_fault(wsclock_state_t *ws, 
                           hardware_state_t *hw, 
                           physical_frame_t *frames, 
                           page_table_entry_t *page_table, 
                           uint32_t process_id, 
                           uint32_t vpn, 
                           uint32_t bytes_used, 
                           uint32_t current_timestamp);

#endif /* PAGE_REPLACEMENT_H */
