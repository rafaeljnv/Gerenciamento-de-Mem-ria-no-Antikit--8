# Simulador de Gerenciamento de Memória - Antikit-8

**Disciplina:** Sistemas Operacionais  
**Autor:** Rafael Jeronimo Nunes Vasconcelo (RA: 795416)  
**Referencia Teorica:** Capitulo 3 (Gerenciamento de Memoria) — Andrew S. Tanenbaum (*Sistemas Operacionais Modernos*)  

---

## 1. Descricao do Projeto

Implementacao de um simulador de gerenciamento de memoria em linguagem C para a arquitetura Antikit-8. O sistema modela a paginacao de memoria virtual, substituicao de paginas e auditoria de integridade sob restricoes fisicas de hardware.

### Modulos do Sistema
- **MMU e Paginacao Assimetrica (`mmu_manager.c` / `mmu_manager.h`):** Traducao de enderecos virtuais para espaco de 1 MB, alternando paginas pares de 4 KB e impares de 8 KB (total de 171 VPNs). Mapeamento em 32 quadros fisicos de 8 KB na memoria RAM (256 KB).
- **Algoritmo de Substituicao WSClock (`page_replacement.c` / `page_replacement.h`):** Algoritmo de relogio baseado em conjunto de trabalho (tau=30) com gestao termica do dispositivo de Swap para prevencao de thermal throttling (limite de 65 °C).
- **Auditoria e Metricas (`audit_metrics.c` / `audit_metrics.h`):** Deteccao de fading no barramento a cada 5 acessos globais, calculo de hash FNV-1a para verificacao de integridade da tabela de paginas, visualizacao ASCII de fragmentacao interna e relatorio consolidado.

---

## 2. Compilacao e Execucao

### Requisitos
- Compilador GCC (C99) ou CMake

### Compilacao via GCC
```powershell
gcc -Wall -Wextra -std=c99 hardware_specs.c mmu_manager.c page_replacement.c audit_metrics.c main.c -o simulador.exe
gcc -Wall -Wextra -std=c99 hardware_specs.c mmu_manager.c teste1_limites.c -o teste1.exe
gcc -Wall -Wextra -std=c99 gerar_traces.c -o gerar_traces.exe
```

### Execucao dos Testes
Para gerar os traces e rodar todos os testes de validacao:
```powershell
.\executar_testes.bat
```

---

## 3. Testes de Validacao

1. **Teste 1 — Limites da Traducao MMU (`teste1.exe`):** Validacao dos enderecos criticos de fronteira (12287, 12288 e 1048575).
2. **Teste 2 — Estresse Termico do Swap (`trace_teste2.txt`):** Comparacao entre substituicao sem controle termico (172 °C e penalidade exponencial em ciclos) e algoritmo eficiente (52 °C e 2.000 ciclos).
3. **Teste 3 — Deteccao de Fading e Integridade (`trace_teste3.txt`):** Monitoramento do zeramento inadvertido do bit R a cada 5 acessos e atualizacao do checksum FNV-1a.
4. **Teste 4 — Fragmentacao Interna (`trace_teste4.txt`):** Analise do impacto de paginas variaveis em slots fixos de 8 KB atraves de mapa ASCII.
