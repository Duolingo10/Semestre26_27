"""
PRACTICA 3: K-MEANS CON DIGITOS Y METRICAS DE EVALUACION

"""

import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.datasets import load_digits
from sklearn.decomposition import PCA
from sklearn.cluster import KMeans
from sklearn.metrics import silhouette_score, adjusted_rand_score, confusion_matrix, classification_report
from scipy.stats import mode

# Parametros iniciales
K = 10
MAX_ITER = 100
TOL = 1e-4
ESTADO_ALEATORIO = 42

# 1. Cargar los datos de digits
digitos = load_digits()
X = digitos.data / 16.0  # Normalizacion de pixeles al rango [0, 1]
y = digitos.target
forma_imagen = digitos.images[0].shape

print("Conjunto de datos de digitos cargado correctamente.")
print("Muestras:", X.shape[0])
print("Caracteristicas originales:", X.shape[1])

# 2. Reduccion de dimensionalidad con PCA 
pca = PCA(n_components=2, random_state=ESTADO_ALEATORIO)
X_pca = pca.fit_transform(X)

# 3. Determinacion de K optimo con el metodo del Codo
inercia = []
ks = range(2, 13)

for k in ks:
    km = KMeans(
        n_clusters=k,
        n_init=10,
        max_iter=MAX_ITER,
        tol=TOL,
        random_state=ESTADO_ALEATORIO
    )
    km.fit(X_pca)
    inercia.append(km.inertia_)

plt.figure(figsize=(8, 6))
plt.plot(ks, inercia, marker="o", linestyle="--", color="b")
plt.xlabel("Numero de grupos (K)")
plt.ylabel("Inercia (Suma de distancias al cuadrado)")
plt.title("Metodo del codo para determinar K optimo")
plt.xticks(ks)
plt.tight_layout()
plt.savefig("01_metodo_codo.png", dpi=150)
plt.show()

# 4. Agrupamiento K-means
modelo = KMeans(
    n_clusters=K,
    n_init=10,
    max_iter=MAX_ITER,
    tol=TOL,
    random_state=ESTADO_ALEATORIO
)

grupos = modelo.fit_predict(X_pca)
centroides_pca = modelo.cluster_centers_

print("\nResultados de K-means:")
print("Iteraciones hasta converger:", modelo.n_iter_)
print("Inercia final:", round(modelo.inertia_, 6))

# 5. Grafica de los 10 grupos
plt.figure(figsize=(10, 7))

for k in range(K):
    plt.scatter(
        X_pca[grupos == k, 0],
        X_pca[grupos == k, 1],
        label=f"Grupo {k + 1}",
        alpha=0.6,
        s=25
    )

plt.scatter(
    centroides_pca[:, 0],
    centroides_pca[:, 1],
    marker="X",
    s=200,
    color="black",
    label="Centroides"
)

# Etiquetas sobre cada centroide
for k in range(K):
    plt.text(
        centroides_pca[k, 0] + 0.1,
        centroides_pca[k, 1] + 0.1,
        f"C{k + 1}",
        fontsize=10,
        fontweight="bold"
    )

plt.xlabel("Componente Principal 1")
plt.ylabel("Componente Principal 2")
plt.title("K-means con Digitos")
plt.legend(bbox_to_anchor=(1.02, 1), loc="upper left")
plt.tight_layout()
plt.savefig("02_grafica_10_grupos.png", dpi=150)
plt.show()

# 6. Visualizacion de centroides
centroides_reconstruidos = pca.inverse_transform(centroides_pca)

fig, ejes = plt.subplots(2, 5, figsize=(10, 4))

for k, ax in enumerate(ejes.flat):
    img = centroides_reconstruidos[k].reshape(forma_imagen)
    ax.imshow(img, cmap="binary")
    ax.set_title(f"Grupo {k + 1}")
    ax.axis("off")

fig.suptitle("Centroides reconstruidos como imagenes", fontsize=14)
fig.tight_layout()
fig.savefig("03_centroides_imagenes.png", dpi=150)
plt.show()

# 7. Mapeo de grupos a digitos reales y Matriz de Confusion
y_pred_mapeado = np.zeros_like(grupos)

for k in range(K):
    mascara = (grupos == k)
    if np.sum(mascara) > 0:
        digito_moda = mode(y[mascara], keepdims=True).mode[0]
        y_pred_mapeado[mascara] = digito_moda

matriz_conf = confusion_matrix(y, y_pred_mapeado)

plt.figure(figsize=(8, 6))
sns.heatmap(matriz_conf, annot=True, fmt="d", cmap="Blues", xticklabels=range(10), yticklabels=range(10))
plt.xlabel("Digito Predicho (Mapeado)")
plt.ylabel("Digito Real")
plt.title("Matriz de Confusion: Digito Real vs Predicho")
plt.tight_layout()
plt.savefig("04_matriz_confusion.png", dpi=150)
plt.show()

# 8. Analisis de digitos mal clasificados
indices_errores = np.where(y != y_pred_mapeado)[0]
total_muestras = len(y)
total_errores = len(indices_errores)
tasa_error = (total_errores / total_muestras) * 100

print("\n" + "=" * 50)
print("ANALISIS DE ERRORES DE CLASIFICACION")
print("=" * 50)
print("Total de imagenes evaluadas :", total_muestras)
print("Imagenes mal clasificadas   :", total_errores, f"({tasa_error:.2f}%)")

parejas_confusion = {}
for idx in indices_errores:
    par = (int(y[idx]), int(y_pred_mapeado[idx]))
    parejas_confusion[par] = parejas_confusion.get(par, 0) + 1

parejas_ordenadas = sorted(parejas_confusion.items(), key=lambda item: item[1], reverse=True)

print("\nTop 5 Confusiones mas Frecuentes (Real -> Predicho):")
for (d_real, d_pred), conteo in parejas_ordenadas[:5]:
    print(f" - Digito Real '{d_real}' clasificado como '{d_pred}': {conteo} casos")

# Grafica de muestras con error
np.random.seed(ESTADO_ALEATORIO)
muestras_error = np.random.choice(indices_errores, size=min(10, total_errores), replace=False)

fig, ejes = plt.subplots(2, 5, figsize=(12, 5))

for i, ax in enumerate(ejes.flat):
    idx = muestras_error[i]
    img = X[idx].reshape(forma_imagen)
    d_real = y[idx]
    d_pred = y_pred_mapeado[idx]
    
    ax.imshow(img, cmap="magma")
    ax.set_title(f"Real: {d_real} | Pred: {d_pred}")
    ax.axis("off")

fig.suptitle("Analisis Visual: Muestras de Digitos Mal Clasificados", fontsize=13)
fig.tight_layout()
fig.savefig("05_digitos_mal_clasificados.png", dpi=150)
plt.show()

# 9. Metricas de evaluacion global y reporte por grupo
punt_silueta = silhouette_score(X_pca, grupos)
ind_rand_ajust = adjusted_rand_score(y, grupos)

print("\n" + "=" * 50)
print("EVALUACION GLOBAL Y REPORTE POR GRUPO")
print("=" * 50)
print("Puntuacion de calidad interna:", round(punt_silueta, 6))
print("Indice Rand Ajustado - ARI (Calidad externa):", round(ind_rand_ajust, 6))

print("\nReporte de Calidad por Clase/Grupo Mapeado:")
print(classification_report(
    y, 
    y_pred_mapeado, 
    target_names=[f"Dígito {d}" for d in range(10)], 
    zero_division=0
))