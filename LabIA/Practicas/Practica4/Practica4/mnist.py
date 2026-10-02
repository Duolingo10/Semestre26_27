import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import fetch_openml
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA
from sklearn.neighbors import KNeighborsClassifier
from sklearn.pipeline import Pipeline
from sklearn.metrics import accuracy_score, confusion_matrix, ConfusionMatrixDisplay
from collections import Counter

RANDOM_STATE = 42

# 1) Datos: 12,000 imágenes de MNIST
print("Cargando dataset MNIST...")
mnist = fetch_openml('mnist_784', version=1, as_frame=False, parser='auto')
X_raw = mnist.data[:12000]
y_true = mnist.target[:12000].astype(int)

# Dividir en entrenamiento y prueba (80/20)
X_train, X_test, y_train, y_test = train_test_split(
    X_raw, y_true, test_size=0.2, stratify=y_true, random_state=RANDOM_STATE
)


# Proyección PCA

print("Generando proyección PCA...")
scaler_temp = StandardScaler()
pca_temp = PCA(n_components=40, random_state=RANDOM_STATE)
X_train_pca = pca_temp.fit_transform(scaler_temp.fit_transform(X_train))

plt.figure(figsize=(8, 6))
scatter = plt.scatter(X_train_pca[:, 0], X_train_pca[:, 1], c=y_train, cmap='tab10', alpha=0.6, s=10)
plt.colorbar(scatter, ticks=range(10), label='Dígito Real')
plt.title("1. Proyección PCA de Entrenamiento")
plt.xlabel("Componente Principal 1")
plt.ylabel("Componente Principal 2")
plt.tight_layout()
out_png_1 = "1_pca.png"
plt.savefig(out_png_1, dpi=120)

# 2) Función para crear el Pipeline (Escalado -> PCA -> KNN)
def make_pipeline(k):
    return Pipeline([
        ("scaler", StandardScaler()),
        ("pca", PCA(n_components=40, random_state=RANDOM_STATE)),
        ("knn", KNeighborsClassifier(n_neighbors=k))
    ])

# 3) Rango de k mencionado por la práctica
k_values = [1, 3, 5, 7, 9, 11, 15]
test_scores = []

print("Evaluando valores de k...")
for k in k_values:
    pipe = make_pipeline(k)
    pipe.fit(X_train, y_train)
    y_pred_k = pipe.predict(X_test)
    acc = accuracy_score(y_test, y_pred_k)
    test_scores.append(acc)
    print(f"k={k} -> Accuracy en prueba: {acc:.4f}")

# Gráfica Accuracy vs k 
plt.figure(figsize=(8,5))
plt.plot(k_values, test_scores, marker="o", color="teal")
plt.title("2. Exactitud vs Número de Vecinos k (Pipeline con StandardScaler y PCA)")
plt.xlabel("Número de vecinos (k)")
plt.ylabel("Accuracy (Datos de prueba)")
plt.xticks(k_values)
plt.grid(True, linestyle="--", alpha=0.4)
out_png_2 = "2_exactitud_vs_k.png"
plt.tight_layout()
plt.savefig(out_png_2, dpi=120)

# 4) Evaluación con k=3 
print("\nClasificando con k=3...")
best_pipe = make_pipeline(3)
best_pipe.fit(X_train, y_train)
y_pred = best_pipe.predict(X_test)

# Aciertos vs Errores por Dígito

aciertos = [np.sum((y_pred == d) & (y_test == d)) for d in range(10)]
errores = [np.sum((y_pred != d) & (y_test == d)) for d in range(10)]
x_pos = np.arange(10)
ancho = 0.4

plt.figure(figsize=(10, 6))
plt.bar(x_pos - ancho/2, aciertos, ancho, label='Correctos', color='forestgreen')
plt.bar(x_pos + ancho/2, errores, ancho, label='Incorrectos', color='crimson')
plt.xticks(x_pos, range(10))
plt.title("3. Clasificación por Dígito: Aciertos vs Errores (k=3)")
plt.xlabel("Dígito Real")
plt.ylabel("Cantidad de Muestras")
plt.legend()
plt.grid(axis='y', linestyle='--', alpha=0.7)
out_png_3 = "3_aciertos_errores.png"
plt.tight_layout()
plt.savefig(out_png_3, dpi=120)

# 5) Matriz de confusión
cm = confusion_matrix(y_test, y_pred)
disp = ConfusionMatrixDisplay(confusion_matrix=cm)
fig, ax = plt.subplots(figsize=(8, 8))
disp.plot(values_format="d", cmap="Blues", ax=ax)
ax.set_xlabel("Dígito Predicho")
ax.set_ylabel("Dígito Real")
plt.title("4. Matriz de confusión - KNN (k=3)")
out_png_4 = "4_matriz_confusion.png"
plt.tight_layout()
plt.savefig(out_png_4, dpi=120)

# 6) Análisis Automático de Errores
total_muestras = len(y_test)
num_incorrectos = np.sum(y_pred != y_test)
error_porcentaje = (num_incorrectos / total_muestras) * 100

print(f"\n[RESULTADOS FINALES k=3]")
print(f"Muestras clasificadas correctamente: {total_muestras - num_incorrectos}")
print(f"Muestras mal clasificadas: {num_incorrectos}")
print(f"Porcentaje de Error: {error_porcentaje:.2f}%\n")

errores_pares = [(real, pred) for real, pred in zip(y_test, y_pred) if real != pred]
conteo_confusiones = Counter(errores_pares)
print("Las 10 confusiones más frecuentes (Real -> Predicho):")
for i, ((real, pred), frec) in enumerate(conteo_confusiones.most_common(10), 1):
    print(f" {i}. Real: {real} -> Predicho: {pred} ({frec} veces)")

# Imágenes mal clasificadas 
indices_errores = np.where(y_test != y_pred)[0]
num_mostrar = min(20, len(indices_errores))

plt.figure(figsize=(10, 8))
for i in range(num_mostrar):
    idx = indices_errores[i]
    imagen_original = X_test[idx].reshape(28, 28)
    plt.subplot(4, 5, i + 1)
    plt.imshow(imagen_original, cmap='gray_r')
    plt.title(f"R:{y_test[idx]} P:{y_pred[idx]}", color='red')
    plt.axis('off')

plt.suptitle("5. Ejemplos de Imágenes Mal Clasificadas", fontsize=14)
out_png_5 = "5_imagenes_errores.png"
plt.tight_layout()
plt.savefig(out_png_5, dpi=120)

print(f"\n[OK] Gráficas guardadas: {out_png_1}, {out_png_2}, {out_png_3}, {out_png_4}, {out_png_5}")