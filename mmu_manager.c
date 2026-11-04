/**
 * @file mmu_manager.c
 * @brief Implementacao do Modulo do Integrante 1 (Arquiteto da MMU).
 * 
 * Contem as rotinas de conversao de enderecos virtuais com paginas assimetricas
 * e a manipulacao das tabelas de paginas e quadros fisicos da RAM.
 */

#include "mmu_manager.h"
#include <stdio.h>
#include <string.h>

void mmu_init_page_table(page_table_entry_t *page_table, uint32_t num_entries) {
    if (page_table == NULL) return;

    for (uint32_t i = 0; i < num_entries; i++) {
        page_table[i].frame_number = 0;
        page_table[i].present = false;
        page_table[i].referenced = false;
        page_table[i].modified = false;
        page_table[i].last_access_t = 0;
    }
}

void mmu_init_physical_ram(physical_frame_t *frames, uint32_t num_frames) {
    if (frames == NULL) return;

    for (uint32_t i = 0; i < num_frames; i++) {
        frames[i].occupied = false;
        frames[i].process_id = 0;
        frames[i].vpn = 0;
        frames[i].page_size = 0;
        frames[i].bytes_allocated = 0;
    }
}

bool mmu_translate_address(uint32_t virtual_address, 
                           uint32_t *out_vpn, 
                           uint32_t *out_offset, 
                           uint32_t *out_page_size) {
    
    /* Validacao dos limites do espaco virtual de 1 MB */
    if (virtual_address >= VIRTUAL_MEMORY_SIZE) {
        return false;
    }

    /* Cada par de paginas (par + impar) totaliza 4096 + 8192 = 12288 bytes */
    uint32_t pair_size = PAGE_SIZE_EVEN + PAGE_SIZE_ODD; /* 12288 bytes */
    uint32_t pair_index = virtual_address / pair_size;
    uint32_t remainder = virtual_address % pair_size;

    uint32_t vpn = 0;
    uint32_t offset = 0;
    uint32_t page_size = 0;

    if (remainder < PAGE_SIZE_EVEN) {
        /* Pertence a pagina com ID par (4 KB) */
        vpn = pair_index * 2;
        offset = remainder;
        page_size = PAGE_SIZE_EVEN;
    } else {
        /* Pertence a pagina com ID impar (8 KB) */
        vpn = (pair_index * 2) + 1;
        offset = remainder - PAGE_SIZE_EVEN;
        page_size = PAGE_SIZE_ODD;
    }

    /* Salvando valores nos ponteiros de saida */
    if (out_vpn != NULL) *out_vpn = vpn;
    if (out_offset != NULL) *out_offset = offset;
    if (out_page_size != NULL) *out_page_size = page_size;

    return true;
}

uint32_t mmu_get_physical_address(uint32_t frame_number, uint32_t offset) {
    return (frame_number * FRAME_SIZE) + offset;
}

int32_t mmu_find_free_frame(physical_frame_t *frames, uint32_t num_frames) {
    if (frames == NULL) return -1;

    for (uint32_t i = 0; i < num_frames; i++) {
        if (!frames[i].occupied) {
            return (int32_t)i;
        }
    }
    return -1; /* Nao ha frames livres na RAM */
}

void mmu_map_page(physical_frame_t *frames, 
                  page_table_entry_t *page_table, 
                  uint32_t frame_number, 
                  uint32_t process_id, 
                  uint32_t vpn,
                  uint32_t bytes_used) {
    
    if (frames == NULL || page_table == NULL || frame_number >= TOTAL_RAM_FRAMES) {
        return;
    }

    uint32_t page_size = (vpn % 2 == 0) ? PAGE_SIZE_EVEN : PAGE_SIZE_ODD;

    /* Configura o quadro fisico */
    frames[frame_number].occupied = true;
    frames[frame_number].process_id = process_id;
    frames[frame_number].vpn = vpn;
    frames[frame_number].page_size = page_size;
    frames[frame_number].bytes_allocated = (bytes_used > 0) ? bytes_used : page_size;

    /* Atualiza a entrada na tabela de paginas */
    page_table[vpn].frame_number = frame_number;
    page_table[vpn].present = true;
    page_table[vpn].referenced = true;
    page_table[vpn].modified = false;
}

void mmu_unmap_page(physical_frame_t *frames, 
                    page_table_entry_t *page_table, 
                    uint32_t frame_number) {
    
    if (frames == NULL || frame_number >= TOTAL_RAM_FRAMES) {
        return;
    }

    if (frames[frame_number].occupied) {
        uint32_t vpn = frames[frame_number].vpn;
        if (page_table != NULL) {
            page_table[vpn].present = false;
            page_table[vpn].referenced = false;
            page_table[vpn].modified = false;
        }

        frames[frame_number].occupied = false;
        frames[frame_number].process_id = 0;
        frames[frame_number].vpn = 0;
        frames[frame_number].page_size = 0;
        frames[frame_number].bytes_allocated = 0;
    }
}
