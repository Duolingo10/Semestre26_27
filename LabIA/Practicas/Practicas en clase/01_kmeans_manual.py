

"""
PRACTICA K-MEANS CON IRIS

Para que los programas 02, 05 y 06 sean comparables, los tres usan
exactamente los mismos centroides iniciales: muestras 0, 70 y 140
del dataset Iris.

PARAMETROS PARA EJECUCION REPRODUCIBLE

Los siguientes valores garantizan que los tres programas comiencen
exactamente en los mismos centroides. Esta seccion es informativa;
los valores ya estan fijados en el programa.

INDICES_INICIALES = [0, 70, 140]
K = 3
MAX_ITER = 100
TOL = 1e-4

Si posteriormente se desea experimentar con inicializacion aleatoria,
se puede sustituir:

centroides_iniciales = X[INDICES_INICIALES].copy()

por una seleccion aleatoria. En ese caso, para hacerla reproducible
puede utilizarse:

import random
RANDOM_STATE = 42
random.seed(RANDOM_STATE)
np.random.seed(RANDOM_STATE)
indices = np.random.choice(len(X), K, replace=False)
centroides_iniciales = X[indices].copy()
"""

import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import load_iris
from sklearn.cluster import KMeans

K = 3
MAX_ITER = 100
TOL = 1e-4
INDICES_INICIALES = [0, 70, 140]

X = load_iris().data
centroides_iniciales = X[INDICES_INICIALES].copy()

# K-means manual
centroides_manual = centroides_iniciales.copy()

for iteracion in range(MAX_ITER):

    distancias = np.linalg.norm(
        X[:, None, :] - centroides_manual[None, :, :],
        axis=2
    )

    grupos_manual = np.argmin(distancias, axis=1)

    nuevos = centroides_manual.copy()

    for k in range(K):
        puntos = X[grupos_manual == k]
        if len(puntos) > 0:
            nuevos[k] = puntos.mean(axis=0)

    cambio = np.linalg.norm(nuevos - centroides_manual)
    centroides_manual = nuevos

    if cambio < TOL:
        break

# Reasignacion final con los centroides convergidos
distancias = np.linalg.norm(
    X[:, None, :] - centroides_manual[None, :, :],
    axis=2
)
grupos_manual = np.argmin(distancias, axis=1)

inercia_manual = sum(
    np.sum((X[grupos_manual == k] - centroides_manual[k]) ** 2)
    for k in range(K)
)

# K-means con scikit-learn usando exactamente los mismos centroides
modelo = KMeans(
    n_clusters=K,
    init=centroides_iniciales,
    n_init=1,
    max_iter=MAX_ITER,
    tol=TOL
)

grupos_sklearn = modelo.fit_predict(X)
centroides_sklearn = modelo.cluster_centers_

print("Centroides iniciales:")
print(centroides_iniciales)

print("\nK-means manual")
print("Iteraciones:", iteracion + 1)
print("Inercia:", round(inercia_manual, 6))
print("Centroides:")
print(centroides_manual)

print("\nK-means con scikit-learn")
print("Iteraciones:", modelo.n_iter_)
print("Inercia:", round(modelo.inertia_, 6))
print("Centroides:")
print(centroides_sklearn)

# Grafica manual
plt.figure(figsize=(8, 6))

for k in range(K):
    plt.scatter(
        X[grupos_manual == k, 2],
        X[grupos_manual == k, 3],
        label=f"Grupo {k + 1}"
    )

plt.scatter(
    centroides_manual[:, 2],
    centroides_manual[:, 3],
    marker="X",
    s=200,
    label="Centroides"
)

plt.xlabel("Longitud del petalo (cm)")
plt.ylabel("Ancho del petalo (cm)")
plt.title("K-means manual")
plt.legend()
plt.tight_layout()
plt.savefig("05a_comparacion_manual.png", dpi=150)
plt.show()

# Grafica sklearn
plt.figure(figsize=(8, 6))

for k in range(K):
    plt.scatter(
        X[grupos_sklearn == k, 2],
        X[grupos_sklearn == k, 3],
        label=f"Grupo {k + 1}"
    )

plt.scatter(
    centroides_sklearn[:, 2],
    centroides_sklearn[:, 3],
    marker="X",
    s=200,
    label="Centroides"
)

plt.xlabel("Longitud del petalo (cm)")
plt.ylabel("Ancho del petalo (cm)")
plt.title("K-means con scikit-learn")
plt.legend()
plt.tight_layout()
plt.savefig("05b_comparacion_sklearn.png", dpi=150)
plt.show()

# Grafica conjunta
fig, axes = plt.subplots(
    1, 2,
    figsize=(14, 6),
    sharex=True,
    sharey=True
)

for k in range(K):
    axes[0].scatter(
        X[grupos_manual == k, 2],
        X[grupos_manual == k, 3],
        label=f"Grupo {k + 1}"
    )

axes[0].scatter(
    centroides_manual[:, 2],
    centroides_manual[:, 3],
    marker="X",
    s=200,
    label="Centroides"
)

axes[0].set_title(
    f"K-means manual\nInercia = {inercia_manual:.3f}"
)
axes[0].set_xlabel("Longitud del petalo (cm)")
axes[0].set_ylabel("Ancho del petalo (cm)")
axes[0].legend()

for k in range(K):
    axes[1].scatter(
        X[grupos_sklearn == k, 2],
        X[grupos_sklearn == k, 3],
        label=f"Grupo {k + 1}"
    )

axes[1].scatter(
    centroides_sklearn[:, 2],
    centroides_sklearn[:, 3],
    marker="X",
    s=200,
    label="Centroides"
)

axes[1].set_title(
    f"K-means con scikit-learn\nInercia = {modelo.inertia_:.3f}"
)
axes[1].set_xlabel("Longitud del petalo (cm)")
axes[1].legend()

fig.suptitle(
    "Comparacion con los mismos centroides iniciales",
    fontsize=15
)

fig.tight_layout()
fig.savefig(
    "05c_comparacion_conjunta.png",
    dpi=150,
    bbox_inches="tight"
)

plt.show()
'''
Centroides iniciales:
[[5.1 3.5 1.4 0.2]
 [5.9 3.2 4.8 1.8]
 [6.7 3.1 5.6 2.4]]

K-means manual
Iteraciones: 4
Inercia: 78.851441
Centroides:
[[5.006      3.428      1.462      0.246     ]
 [5.9016129  2.7483871  4.39354839 1.43387097]
 [6.85       3.07368421 5.74210526 2.07105263]]

K-means con scikit-learn
Iteraciones: 4
Inercia: 78.851441
Centroides:
[[5.006      3.428      1.462      0.246     ]
 [5.9016129  2.7483871  4.39354839 1.43387097]
 [6.85       3.07368421 5.74210526 2.07105263]]
'''

