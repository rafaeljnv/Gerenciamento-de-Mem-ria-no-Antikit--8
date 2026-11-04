@echo off
echo ============================================================
echo      SUITE DE TESTES DO SUBSISTEMA DE MEMORIA ANTIKIT-8
echo ============================================================
echo.

echo Compilando modulos do simulador e testes...
gcc -Wall -Wextra -std=c99 hardware_specs.c mmu_manager.c page_replacement.c audit_metrics.c main.c -o simulador.exe
gcc -Wall -Wextra -std=c99 hardware_specs.c mmu_manager.c teste1_limites.c -o teste1.exe
gcc -Wall -Wextra -std=c99 gerar_traces.c -o gerar_traces.exe

echo Gerando traces de teste atualizados...
gerar_traces.exe
echo.

echo ============================================================
echo [TESTE 1] Validacao da Traducao Assimetrica (Limites)
echo ============================================================
teste1.exe
echo.

echo ============================================================
echo [TESTE 2 - CENARIO INEFICIENTE] Stress Termico com Throttling
echo ============================================================
simulador.exe trace_teste2.txt --naive
echo.

echo ============================================================
echo [TESTE 2 - CENARIO EFICIENTE] WSClock com Protecao Termica
echo ============================================================
simulador.exe trace_teste2.txt
echo.

echo ============================================================
echo [TESTE 3] Auditoria do Fading do Bit R a cada 5 acessos
echo ============================================================
simulador.exe trace_teste3.txt
echo.

echo ============================================================
echo [TESTE 4] Inspecao Visual da Fragmentacao Interna (ASCII)
echo ============================================================
simulador.exe trace_teste4.txt
echo.

echo ============================================================
echo [EXECUCAO GERAL] Trace Padrao Multi-processo
echo ============================================================
simulador.exe memory_trace.txt
echo.

echo Todos os testes foram executados com sucesso!
pause
