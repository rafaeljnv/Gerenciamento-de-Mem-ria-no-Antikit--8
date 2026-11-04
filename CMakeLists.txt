cmake_minimum_required(VERSION 3.20)
project(trabalho_SO C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Includes locais
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 1. Executavel Principal do Simulador do SO
add_executable(simulador
        main.c
        hardware_specs.c
        hardware_specs.h
        mmu_manager.c
        mmu_manager.h
        page_replacement.c
        page_replacement.h
        audit_metrics.c
        audit_metrics.h
)

# 2. Executavel do Teste 1 (Validacao dos limites da traducao assimetrica)
add_executable(teste1
        teste1_limites.c
        hardware_specs.c
        hardware_specs.h
        mmu_manager.c
        mmu_manager.h
)

# 3. Executavel do Gerador de Traces
add_executable(gerar_traces
        gerar_traces.c
)

# Copia automatica dos arquivos de trace para o diretorio de build do CLion
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/memory_trace.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/trace_teste2.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/trace_teste3.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/trace_teste4.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
