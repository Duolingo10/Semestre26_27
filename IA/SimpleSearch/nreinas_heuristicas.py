# -*- coding: utf-8 -*-
"""
N-Reinas con diferentes heurísticas para A*

Compara múltiples estrategias de búsqueda heurística
"""

import urllib.request
import numpy as np
from matplotlib import pyplot as plt
import pandas as pd

url = "https://raw.githubusercontent.com/kyriox/baile/master/src/SimpleSearch.py"
urllib.request.urlretrieve(url, "SimpleSearch.py")

import SimpleSearch as sp

# ============================================================================
# DEFINICIÓN DEL ESTADO INICIAL
# ============================================================================

start = sp.node((2, 2, 2, 2, 2, 2), depth=0, parent=None)

def plot_tablero(state):
    n = len(state)
    tablero = [[(i+j)%2 for j in range(n)] for i in range(n)]
    fig, ax = plt.subplots(figsize=(5, 5))
    ax.imshow(tablero, cmap="gray")
    for r, c in enumerate(state):
        ax.text(r, c, "♕", ha="center", va="center", fontsize=30, color="red")
    plt.show()

plot_tablero(start.state)

# ============================================================================
# FUNCIÓN SUCESOR
# ============================================================================

def sucesorNR(nodo):
    S = []
    st, n = nodo.state, len(nodo.state)
    for i in range(n):
        adelante = list(st)
        atras = list(st)
        if st[i]+1 < n:
            adelante[i] = st[i]+1
            S.append(sp.node(tuple(adelante), depth=nodo.depth+1, parent=nodo))
        if st[i]-1 >= 0:
            atras[i] = st[i]-1
            S.append(sp.node(tuple(atras), depth=nodo.depth+1, parent=nodo))
    return S

# ============================================================================
# FUNCIÓN META
# ============================================================================

def contar_ataques(*nodos):
    """Cuenta el número de pares de reinas que se atacan mutuamente"""
    nodo = nodos[0]
    tb = nodo.state
    n = len(tb)
    ataques = 0
    for i in range(n-1):
        for j in range(i+1, n):
            # Ataque horizontal (misma fila)
            if tb[i] == tb[j]:
                ataques += 1
            # Ataque diagonal
            if abs(i-j) == abs(tb[i]-tb[j]):
                ataques += 1
    return ataques

def meta(*nodos):
    """Prueba si no hay ataques entre reinas"""
    st = nodos[0]
    return contar_ataques(st) == 0

# ============================================================================
# HEURÍSTICAS ALTERNATIVAS
# ============================================================================

def heuristica_simple(*nodos):
    """Heurística 1: Igual que contar_ataques (la actual)"""
    return contar_ataques(*nodos)

def heuristica_optimista(*nodos):
    """Heurística 2: Más optimista (subestima). 
    Solo cuenta ataques en las últimas columnas (subestima el problema real)"""
    nodo = nodos[0]
    tb = nodo.state
    n = len(tb)
    # Solo contar ataques a partir de la mitad
    ataques = 0
    for i in range(n//2, n-1):
        for j in range(i+1, n):
            if tb[i] == tb[j]:
                ataques += 1
            if abs(i-j) == abs(tb[i]-tb[j]):
                ataques += 1
    return ataques

def heuristica_pesimista(*nodos):
    """Heurística 3: Más pesimista (sobrestima).
    Suma ataques más una penalización por reinas en posiciones similares"""
    nodo = nodos[0]
    tb = nodo.state
    n = len(tb)
    ataques = contar_ataques(nodo)
    
    # Penalización adicional: reinas en posiciones similares (mayor densidad)
    penalizacion = 0
    for i in range(n-1):
        for j in range(i+1, n):
            distancia_columnas = abs(i - j)
            distancia_filas = abs(tb[i] - tb[j])
            if distancia_filas < distancia_columnas:
                penalizacion += 1
    
    return ataques + penalizacion * 0.5

def heuristica_cero(*nodos):
    """Heurística 4: Sin heurística (devuelve 0).
    Esto convierte A* en búsqueda por costo uniforme (como Dijkstra)"""
    return 0

def heuristica_conflictos_pendientes(*nodos):
    """Heurística 5: Cuenta conflictos solo con reinas en columnas posteriores.
    Más admisible para este problema específico."""
    nodo = nodos[0]
    tb = nodo.state
    n = len(tb)
    ataques = 0
    
    # Solo contar conflictos con reinas en columnas no asignadas todavía
    # (heurística más admisible)
    for i in range(n):
        for j in range(i+1, n):
            if tb[i] == tb[j]:
                ataques += 1
            if abs(i-j) == abs(tb[i]-tb[j]):
                ataques += 1
    
    # Dividir entre 2 porque cada conflicto se cuenta dos veces
    return ataques // 2 if ataques > 0 else 0

# ============================================================================
# BÚSQUEDAS CON DIFERENTES HEURÍSTICAS
# ============================================================================

print("Ejecutando búsquedas...")

# BFS y DFS (sin heurística)
bfs = sp.TreeSearch(start, sucesorNR, meta, strategy="bfs")
resultado_bfs = bfs.find()

dfs = sp.TreeSearch(start, sucesorNR, meta, strategy="dfs")
resultado_dfs = dfs.find()

# A* con diferentes heurísticas
a_star_simple = sp.TreeSearch(
    start, sucesorNR, meta, 
    strategy="a*", 
    heuristic=heuristica_simple
)
resultado_simple = a_star_simple.find()

a_star_optimista = sp.TreeSearch(
    start, sucesorNR, meta, 
    strategy="a*", 
    heuristic=heuristica_optimista
)
resultado_optimista = a_star_optimista.find()

a_star_pesimista = sp.TreeSearch(
    start, sucesorNR, meta, 
    strategy="a*", 
    heuristic=heuristica_pesimista
)
resultado_pesimista = a_star_pesimista.find()

a_star_cero = sp.TreeSearch(
    start, sucesorNR, meta, 
    strategy="a*", 
    heuristic=heuristica_cero
)
resultado_cero = a_star_cero.find()

a_star_conflictos = sp.TreeSearch(
    start, sucesorNR, meta, 
    strategy="a*", 
    heuristic=heuristica_conflictos_pendientes
)
resultado_conflictos = a_star_conflictos.find()

# ============================================================================
# COMPARACIÓN DE RESULTADOS
# ============================================================================

resultados = pd.DataFrame({
    "Búsqueda": [
        "BFS", 
        "DFS", 
        "A* (simple)",
        "A* (optimista)",
        "A* (pesimista)",
        "A* (sin heurística)",
        "A* (conflictos)"
    ],
    "Iteraciones": [
        bfs.iterations,
        dfs.iterations,
        a_star_simple.iterations,
        a_star_optimista.iterations,
        a_star_pesimista.iterations,
        a_star_cero.iterations,
        a_star_conflictos.iterations
    ]
})

print("\n" + "="*60)
print("COMPARACIÓN DE ESTRATEGIAS DE BÚSQUEDA")
print("="*60)
print(resultados.to_string(index=False))
print("="*60)

# Mostrar la solución encontrada
if resultado_simple:
    print(f"\n✓ Solución encontrada: {resultado_simple.state}")
    plot_tablero(resultado_simple.state)
else:
    print("\n✗ No se encontró solución")
