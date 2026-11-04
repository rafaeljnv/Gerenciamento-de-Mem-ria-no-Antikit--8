/**
 * @file mmu_manager.h
 * @brief Modulo do Integrante 1 (Arquiteto da MMU).
 * 
 * Responsavel pela traducao de enderecos virtuais para fisicos
 * considerando paginas assimetricas (pares 4KB, impares 8KB)
 * e gerenciamento dos quadros (frames) da memoria RAM fisica (256 KB).
 */

#ifndef MMU_MANAGER_H
#define MMU_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware_specs.h"

/* Quantidade total de paginas virtuais para cobrir 1 MB */
/* 85 duplas de (4KB + 8KB = 12KB) = 170 paginas + 1 pagina de 4KB (VPN 170) = 171 paginas */
#define TOTAL_VIRTUAL_PAGES 171

/* Tamanho de cada frame rigido na RAM fisica (8 KB para acomodar qualquer pagina) */
#define FRAME_SIZE 8192

/* Quantidade total de frames rigidos na RAM fisica de 256 KB */
#define TOTAL_RAM_FRAMES (PHYSICAL_RAM_SIZE / FRAME_SIZE) /* 32 frames */

/**
 * @brief Estrutura que representa o estado de um quadro (frame) fisico na RAM.
 */
typedef struct {
    bool occupied;          /* Indica se o frame esta em uso */
    uint32_t process_id;    /* ID do processo dono do frame */
    uint32_t vpn;           /* Numero da pagina virtual alocada no frame */
    uint32_t page_size;     /* Tamanho da pagina alocada (4096 ou 8192) */
    uint32_t bytes_allocated; /* Quantidade de bytes efetivamente utilizados pelo processo */
} physical_frame_t;

/**
 * @brief Inicializa a tabela de paginas de um processo com valores padrao.
 * @param page_table Vetor de entradas da tabela de paginas.
 * @param num_entries Quantidade total de entradas (171).
 */
void mmu_init_page_table(page_table_entry_t *page_table, uint32_t num_entries);

/**
 * @brief Inicializa os quadros da memoria RAM fisica.
 * @param frames Vetor de frames fisicos.
 * @param num_frames Quantidade total de frames (32).
 */
void mmu_init_physical_ram(physical_frame_t *frames, uint32_t num_frames);

/**
 * @brief Traduz um endereco virtual em VPN (Virtual Page Number) e Offset.
 * Respeita a regra de paginacao assimetrica: pares = 4KB, impares = 8KB.
 * 
 * @param virtual_address Endereco virtual requisitado (0 a 1048575).
 * @param out_vpn Ponteiro de saida para a VPN calculada.
 * @param out_offset Ponteiro de saida para o offset calculado dentro da pagina.
 * @param out_page_size Ponteiro de saida para o tamanho da pagina correspondente.
 * @return true se o endereco for valido (dentro de 1MB), false caso contrario.
 */
bool mmu_translate_address(uint32_t virtual_address, 
                           uint32_t *out_vpn, 
                           uint32_t *out_offset, 
                           uint32_t *out_page_size);

/**
 * @brief Calcula o endereco fisico na RAM a partir do frame e offset.
 * 
 * @param frame_number Numero do quadro fisico (0 a 31).
 * @param offset Deslocamento dentro da pagina.
 * @return uint32_t Endereco fisico calculado.
 */
uint32_t mmu_get_physical_address(uint32_t frame_number, uint32_t offset);

/**
 * @brief Procura por um quadro fisico livre na RAM.
 * @param frames Vetor de frames fisicos.
 * @param num_frames Quantidade total de frames.
 * @return Indice do frame livre (0 a num_frames - 1), ou -1 se a RAM estiver cheia.
 */
int32_t mmu_find_free_frame(physical_frame_t *frames, uint32_t num_frames);

/**
 * @brief Mapeia uma pagina virtual em um quadro fisico da RAM.
 * 
 * @param frames Vetor de frames fisicos.
 * @param page_table Tabela de paginas do processo.
 * @param frame_number Indice do frame fisico.
 * @param process_id ID do processo.
 * @param vpn Numero da pagina virtual.
 * @param bytes_used Quantidade de bytes usados pela pagina (para calculo de fragmentacao).
 */
void mmu_map_page(physical_frame_t *frames, 
                  page_table_entry_t *page_table, 
                  uint32_t frame_number, 
                  uint32_t process_id, 
                  uint32_t vpn,
                  uint32_t bytes_used);

/**
 * @brief Desmapeia uma pagina da RAM (utilizado durante substituicao de pagina).
 * 
 * @param frames Vetor de frames fisicos.
 * @param page_table Tabela de paginas do processo dono da pagina vitima.
 * @param frame_number Indice do frame fisico a ser liberado.
 */
void mmu_unmap_page(physical_frame_t *frames, 
                    page_table_entry_t *page_table, 
                    uint32_t frame_number);

#endif /* MMU_MANAGER_H */
