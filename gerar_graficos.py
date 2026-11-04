"""
Script para geracao dos graficos analiticos obrigatorios do Relatorio Tecnico.
Gera os graficos:
1. grafico_temperatura_ciclos.png (WSClock vs Swap: Estresse Termico e Ciclos)
2. grafico_fragmentacao_interna.png (Impacto de paginas assimetricas em slots rigidos de 8KB)
"""

import matplotlib.pyplot as plt
import numpy as np

def gerar_grafico_termico():
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 5))
    
    cenarios = ['Ineficiente\n(Sem Controle)', 'Eficiente\n(WSClock Termico)']
    temperaturas = [172.0, 52.0]
    cores_temp = ['#d9534f', '#5cb85c']
    
    # Grafico 1A: Temperatura Maxima do Swap
    bars1 = ax1.bar(cenarios, temperaturas, color=cores_temp, width=0.5, edgecolor='black', linewidth=1.2)
    ax1.axhline(y=65.0, color='red', linestyle='--', linewidth=2, label='Teto Thermal Throttling (65°C)')
    ax1.set_ylabel('Temperatura Maxima (°C)', fontsize=12, fontweight='bold')
    ax1.set_title('Temperatura Maxima Alcancada no Swap', fontsize=13, fontweight='bold')
    ax1.set_ylim(0, 195)
    ax1.grid(axis='y', linestyle=':', alpha=0.7)
    ax1.legend(loc='upper right', fontsize=10)
    
    for bar in bars1:
        yval = bar.get_height()
        ax1.text(bar.get_x() + bar.get_width()/2.0, yval + 4, f"{yval:.1f} °C", 
                 ha='center', va='bottom', fontsize=11, fontweight='bold')
    
    # Grafico 1B: Ciclos de CPU Desperdicados (Escala Logaritmica)
    ciclos = [4793492000, 2000]
    cores_ciclos = ['#d9534f', '#0275d8']
    bars2 = ax2.bar(cenarios, ciclos, color=cores_ciclos, width=0.5, edgecolor='black', linewidth=1.2)
    ax2.set_yscale('log')
    ax2.set_ylabel('Ciclos de Clock de E/S (Escala Log)', fontsize=12, fontweight='bold')
    ax2.set_title('Penalidade de CPU por Thermal Throttling', fontsize=13, fontweight='bold')
    ax2.grid(axis='y', linestyle=':', alpha=0.7)
    
    ax2.text(0, ciclos[0] * 1.5, f"4.79 Bilhoes\n(Penalidade 2^n)", ha='center', va='bottom', fontsize=10, fontweight='bold', color='#d9534f')
    ax2.text(1, ciclos[1] * 2.5, f"2.000 Ciclos\n(Custo Base)", ha='center', va='bottom', fontsize=10, fontweight='bold', color='#0275d8')
    
    plt.suptitle('Analise Comparativa: Estresse Termico e Impacto em Ciclos de Clock', fontsize=14, fontweight='bold')
    plt.tight_layout()
    plt.savefig('grafico_temperatura_ciclos.png', dpi=300)
    plt.close()
    print("[OK] Gerado grafico_temperatura_ciclos.png")

def gerar_grafico_fragmentacao():
    fig, ax = plt.subplots(figsize=(9, 5.5))
    
    categorias = [
        'Pagina Par (4KB)\nSlot 8KB [UUUUFFFF]',
        'Pagina Impar (8KB)\nUso Maximo [UUUUUUUU]',
        'Pagina Impar (8KB)\nUso Pequeno 1KB [UFFFFFFF]',
        'Media do Teste 4\n(Trace Teste 4)'
    ]
    taxas = [50.0, 0.0, 87.5, 68.75]
    cores = ['#f0ad4e', '#5cb85c', '#d9534f', '#0275d8']
    
    bars = ax.bar(categorias, taxas, color=cores, width=0.55, edgecolor='black', linewidth=1.2)
    ax.set_ylabel('Taxa de Fragmentacao Interna (%)', fontsize=12, fontweight='bold')
    ax.set_title('Desperdicio de Memoria por Fragmentacao Interna em Slots Rigidos de 8KB', fontsize=13, fontweight='bold')
    ax.set_ylim(0, 105)
    ax.grid(axis='y', linestyle=':', alpha=0.7)
    
    for bar in bars:
        yval = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2.0, yval + 2, f"{yval:.2f}%", 
                 ha='center', va='bottom', fontsize=11, fontweight='bold')
        
    plt.tight_layout()
    plt.savefig('grafico_fragmentacao_interna.png', dpi=300)
    plt.close()
    print("[OK] Gerado grafico_fragmentacao_interna.png")

if __name__ == '__main__':
    gerar_grafico_termico()
    gerar_fragmentacao() if 'gerar_fragmentacao' in locals() else gerar_grafico_fragmentacao()
