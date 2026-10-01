import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import fetch_openml
from sklearn.decomposition import PCA
from sklearn.cluster import KMeans
from sklearn.metrics import confusion_matrix, accuracy_score, adjusted_rand_score, silhouette_score
from scipy.stats import mode

# ==============================================================================
# 1. CARGA Y PREPROCESAMIENTO DEL DATASET MNIST (Matrícula impar)
# ==============================================================================
print("1/7. Cargando dataset MNIST (10,000 imágenes para optimizar velocidad)...")
mnist = fetch_openml('mnist_784', version=1, as_frame=False, parser='auto')
X_raw = mnist.data[:10000]
y_true = mnist.target[:10000].astype(int)

# Normalización de píxeles al rango [0, 1]
X_norm = X_raw / 255.0

# ==============================================================================
# 2. REDUCCIÓN DE DIMENSIONALIDAD CON PCA A 3D
# ==============================================================================
print("2/7. Aplicando PCA para reducir de 784 a 3 dimensiones...")
pca_3d = PCA(n_components=3, random_state=42)
X_pca3d = pca_3d.fit_transform(X_norm)

varianza = np.sum(pca_3d.explained_variance_ratio_) * 100
print(f"   - Varianza retenida con 3 componentes PCA: {varianza:.2f}%")

# ==============================================================================
# 3. DETERMINACIÓN DEL K ÓPTIMO (MÉTODOS DEL CODO)
# ==============================================================================
print("3/7. Calculando Método del Codo para valores de K de 1 a 14...")
inercias = []
k_valores = range(1, 15)

for k in k_valores:
    kmeans_temp = KMeans(n_clusters=k, n_init=10, random_state=42)
    kmeans_temp.fit(X_pca3d)
    inercias.append(kmeans_temp.inertia_)

plt.figure(figsize=(8, 5))
plt.plot(k_valores, inercias, marker='o', color='purple')
plt.xlabel("Número de clusters (K)")
plt.ylabel("Inercia (Suma de errores cuadráticos)")
plt.title("Método del Codo para K-Means en MNIST")
plt.grid(True)
plt.tight_layout()
plt.savefig("metodo_codo.png", dpi=150)
plt.show()

# ==============================================================================
# 4. CLUSTERING K-MEANS (K=10) Y GRÁFICA EN 3D
# ==============================================================================
print("4/7. Generando agrupamiento K-Means con K=10 y gráfico 3D...")
K_OPTIMO = 10
kmeans = KMeans(n_clusters=K_OPTIMO, n_init=10, random_state=42)
grupos = kmeans.fit_predict(X_pca3d)
centroides_3d = kmeans.cluster_centers_

fig = plt.figure(figsize=(10, 8))
ax = fig.add_subplot(111, projection='3d')

for k in range(K_OPTIMO):
    ax.scatter(
        X_pca3d[grupos == k, 0],
        X_pca3d[grupos == k, 1],
        X_pca3d[grupos == k, 2],
        label=f"Cluster {k}",
        alpha=0.4,
        s=10
    )

ax.scatter(
    centroides_3d[:, 0],
    centroides_3d[:, 1],
    centroides_3d[:, 2],
    marker="X",
    color="black",
    s=250,
    label="Centroides"
)

ax.set_xlabel("Componente PCA 1")
ax.set_ylabel("Componente PCA 2")
ax.set_zlabel("Componente PCA 3")
ax.set_title("Clustering 3D K-Means sobre MNIST (PCA)")
ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
plt.tight_layout()
plt.savefig("clustering_3d_mnist.png", dpi=150)
plt.show()

# ==============================================================================
# 5. MAPEO DE CLUSTERS A DÍGITOS REALES Y EVALUACIÓN
# ==============================================================================
print("5/7. Evaluando métricas del clustering...")
y_pred = np.zeros_like(grupos)

for k in range(K_OPTIMO):
    mascara = (grupos == k)
    if np.sum(mascara) > 0:
        digito_moda = mode(y_true[mascara], keepdims=True).mode[0]
        y_pred[mascara] = digito_moda

acc = accuracy_score(y_true, y_pred)
ari = adjusted_rand_score(y_true, grupos)
sil = silhouette_score(X_pca3d, grupos)

print("\n" + "="*40)
print("     RESULTADOS DE LA EVALUACIÓN")
print("="*40)
print(f" Accuracy de mapeo:        {acc * 100:.2f}%")
print(f" Adjusted Rand Index (ARI): {ari:.4f}")
print(f" Silhouette Score:          {sil:.4f}")
print("="*40 + "\n")

# ==============================================================================
# 6. VISUALIZACIÓN DE CENTROIDES RECONSTRUIDOS (28x28 PÍXELES)
# ==============================================================================
print("6/7. Reconstruyendo centroides a formato de imagen 28x28...")
centroides_784 = pca_3d.inverse_transform(centroides_3d)

plt.figure(figsize=(12, 5))
for k in range(K_OPTIMO):
    plt.subplot(2, 5, k + 1)
    plt.imshow(centroides_784[k].reshape(28, 28), cmap='gray')
    plt.title(f"Cluster {k}")
    plt.axis('off')

plt.suptitle("Centroides de K-Means Reconstruidos a Imágenes 28x28", fontsize=14)
plt.tight_layout()
plt.savefig("centroides_imagenes.png", dpi=150)
plt.show()

# ==============================================================================
# 7. MATRIZ DE CONFUSIÓN Y ANÁLISIS DE ERRORES
# ==============================================================================
print("7/7. Generando matriz de confusión y reporte por dígito...")
cm = confusion_matrix(y_true, y_pred)

plt.figure(figsize=(8, 6))
plt.imshow(cm, cmap='Blues')
plt.title("Matriz de Confusión: Dígito Real vs Dígito Mapeado")
plt.colorbar()
plt.xlabel("Dígito Asignado por Cluster")
plt.ylabel("Dígito Real (Etiqueta)")
plt.xticks(range(10))
plt.yticks(range(10))

for i in range(10):
    for j in range(10):
        plt.text(j, i, str(cm[i, j]), horizontalalignment="center",
                 color="white" if cm[i, j] > cm.max() / 2 else "black")

plt.tight_layout()
plt.savefig("matriz_confusion_mnist.png", dpi=150)
plt.show()

print("\n=== REPORTE DE CALIDAD POR DÍGITO ===")
for digito in range(10):
    total_real = np.sum(y_true == digito)
    correctos = cm[digito, digito]
    precision_digito = (correctos / total_real) * 100 if total_real > 0 else 0
    print(f"Dígito {digito}: {correctos}/{total_real} identificados correctamente ({precision_digito:.1f}%)")