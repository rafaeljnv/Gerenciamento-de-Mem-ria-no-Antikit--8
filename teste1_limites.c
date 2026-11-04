/**
 * @file teste1_limites.c
 * @brief Teste 1: Validacao da Traducao Assimetrica (O Teste dos Limites).
 * 
 * Verifica os limites de paginacao assimetrica:
 *  - 12287   -> VPN 1, Offset 8191 (Fim da pagina de 8KB)
 *  - 12288   -> VPN 2, Offset 0    (Inicio da pagina de 4KB)
 *  - 1048575 -> VPN 170, Offset 4095 (Ultimo byte de 1MB sem estouro)
 */

#include <stdio.h>
#include <stdbool.h>
#include "mmu_manager.h"

int main(void) {
    printf("============================================================\n");
    printf("     TESTE 1: VALIDACAO DA TRADUCAO ASSIMETRICA (LIMITES)   \n");
    printf("============================================================\n\n");

    uint32_t addresses_to_test[] = {12287, 12288, 1048575};
    int num_tests = sizeof(addresses_to_test) / sizeof(addresses_to_test[0]);

    for (int i = 0; i < num_tests; i++) {
        uint32_t addr = addresses_to_test[i];
        uint32_t vpn = 0;
        uint32_t offset = 0;
        uint32_t page_size = 0;

        bool ok = mmu_translate_address(addr, &vpn, &offset, &page_size);

        if (ok) {
            printf("Endereco Virtual: %7u (0x%05X) -> VPN: %3u (%s, %u KB) | Offset: %4u\n",
                   addr, addr, vpn, (vpn % 2 == 0) ? "PAR  " : "IMPAR", page_size / 1024, offset);
        } else {
            printf("Endereco Virtual: %7u -> FORA DOS LIMITES DE 1 MB!\n", addr);
        }
    }

    printf("\n--- VERIFICACAO AUTOMATIZADA DOS LIMITES ---\n");

    uint32_t v1, o1, s1;
    mmu_translate_address(12287, &v1, &o1, &s1);
    if (v1 == 1 && o1 == 8191) {
        printf("[OK] Endereco 12287 mapeado corretamente: VPN 1, Offset 8191 (Fim de 8KB).\n");
    } else {
        printf("[FALHA] Endereco 12287 incorreto: VPN %u, Offset %u (Esperado: 1, 8191)\n", v1, o1);
    }

    uint32_t v2, o2, s2;
    mmu_translate_address(12288, &v2, &o2, &s2);
    if (v2 == 2 && o2 == 0) {
        printf("[OK] Endereco 12288 mapeado corretamente: VPN 2, Offset 0 (Inicio de 4KB).\n");
    } else {
        printf("[FALHA] Endereco 12288 incorreto: VPN %u, Offset %u (Esperado: 2, 0)\n", v2, o2);
    }

    uint32_t v3, o3, s3;
    mmu_translate_address(1048575, &v3, &o3, &s3);
    if (v3 == (TOTAL_VIRTUAL_PAGES - 1) && o3 == 4095) {
        printf("[OK] Endereco 1048575 mapeado corretamente: Ultima VPN (%u), Offset 4095 sem estouro.\n", v3);
    } else {
        printf("[FALHA] Endereco 1048575 incorreto: VPN %u, Offset %u (Esperado: %u, 4095)\n",
               v3, o3, TOTAL_VIRTUAL_PAGES - 1);
    }

    printf("============================================================\n");
    return 0;
}
