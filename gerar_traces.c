/**
 * @file gerar_traces.c
 * @brief Utilitario para gerar os arquivos de trace de memoria para os testes obrigatorios.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Funcao auxiliar para calcular o endereco base de qualquer VPN */
uint32_t get_vpn_base_address(uint32_t vpn) {
    uint32_t pair_idx = vpn / 2;
    uint32_t base = pair_idx * 12288;
    if (vpn % 2 != 0) {
        base += 4096;
    }
    return base;
}

void gerar_trace_teste2(void) {
    FILE *f = fopen("trace_teste2.txt", "w");
    if (!f) return;

    fprintf(f, "# TRACE DO TESTE 2: ESTRESSE TERMICO E ESCALONAMENTO (WSCLOCK VS SWAP)\n");
    fprintf(f, "# Fase 1: Preenchimento completo dos 32 frames da RAM (VPN 0 a 31)\n");
    
    /* Preenche os 32 frames da RAM com paginas modificadas (W) */
    for (uint32_t vpn = 0; vpn < 32; vpn++) {
        uint32_t addr = get_vpn_base_address(vpn);
        uint32_t t = vpn * 2; /* timestamps 0, 2, 4, ..., 62 */
        fprintf(f, "%u 1 %u W\n", t, addr);
    }

    fprintf(f, "# Fase 2: No timestamp virtual 200, forcar 10 novas paginas consecutivas\n");
    for (uint32_t vpn = 32; vpn < 42; vpn++) {
        uint32_t addr = get_vpn_base_address(vpn);
        fprintf(f, "200 1 %u R\n", addr);
    }

    fclose(f);
    printf("[OK] Gerado trace_teste2.txt com sucesso!\n");
}

void gerar_trace_teste3(void) {
    FILE *f = fopen("trace_teste3.txt", "w");
    if (!f) return;

    fprintf(f, "# TRACE DO TESTE 3: AUDITORIA DO FADING DO BIT R A CADA 5 ACESSOS\n");
    /* Gera 25 acessos para provocar exatamente 5 ocorrencias de Fading (a cada 5 acessos) */
    for (uint32_t i = 0; i < 25; i++) {
        uint32_t vpn = i % 10;
        uint32_t addr = get_vpn_base_address(vpn) + 128;
        fprintf(f, "%u 1 %u %c\n", i * 10, addr, (i % 3 == 0) ? 'W' : 'R');
    }

    fclose(f);
    printf("[OK] Gerado trace_teste3.txt com sucesso!\n");
}

void gerar_trace_teste4(void) {
    FILE *f = fopen("trace_teste4.txt", "w");
    if (!f) return;

    fprintf(f, "# TRACE DO TESTE 4: INSPECAO VISUAL DE FRAGMENTACAO INTERNA\n");
    fprintf(f, "# Paginas pares (4KB) usam 4096 bytes -> [UUUUFFFF] (4KB usados, 4KB fragmentados no slot de 8KB)\n");
    fprintf(f, "# Paginas impares (8KB) usam apenas 1024 bytes -> [UFFFFFFF] (1KB usado, 7KB fragmentados)\n");
    
    for (uint32_t i = 0; i < 20; i++) {
        uint32_t vpn = i;
        uint32_t addr = get_vpn_base_address(vpn);
        uint32_t bytes_used = (vpn % 2 == 0) ? 4096 : 1024;
        fprintf(f, "%u 1 %u R %u\n", i * 5, addr, bytes_used);
    }

    fclose(f);
    printf("[OK] Gerado trace_teste4.txt com sucesso!\n");
}

void gerar_memory_trace_padrao(void) {
    FILE *f = fopen("memory_trace.txt", "w");
    if (!f) return;

    fprintf(f, "# TRACE GERAL DE MEMORIA DO ANTIKIT-8 OS\n");
    fprintf(f, "# Formato: timestamp pid endereco_virtual R/W [bytes_utilizados]\n");
    
    /* Carga de trabalho de dois processos (PID 1 e PID 2) com 100 acessos variados */
    uint32_t timestamp = 0;
    for (uint32_t step = 0; step < 80; step++) {
        uint32_t pid = (step % 2) + 1;
        uint32_t vpn = (step * 3) % 45;
        uint32_t addr = get_vpn_base_address(vpn) + ((step * 64) % 2048);
        char op = (step % 4 == 0) ? 'W' : 'R';
        uint32_t bytes = (vpn % 2 == 0) ? 4096 : ((step % 3 == 0) ? 2048 : 6144);

        timestamp += (step % 5 == 0) ? 20 : 2;
        fprintf(f, "%u %u %u %c %u\n", timestamp, pid, addr, op, bytes);
    }

    fclose(f);
    printf("[OK] Gerado memory_trace.txt padrao com sucesso!\n");
}

int main(void) {
    printf("Gerando traces de teste para o Antikit-8...\n");
    gerar_trace_teste2();
    gerar_trace_teste3();
    gerar_trace_teste4();
    gerar_memory_trace_padrao();
    printf("Todos os arquivos de trace foram gerados com sucesso!\n");
    return 0;
}
