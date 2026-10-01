import matplotlib.pyplot as plt
from sklearn.datasets import load_iris
from sklearn.cluster import KMeans

# Cargar el dataset Iris
iris = load_iris()
X = iris.data

# Aplicar K-means con 3 grupos
kmeans = KMeans(n_clusters=3, n_init=10, random_state=42)

# Entrenar y obtener el grupo de cada muestra
grupos = kmeans.fit_predict(X)

# Obtener los centroides finales
centroides = kmeans.cluster_centers_

# Mostrar resultados
print("Centroides finales:")
print(centroides)

print("\nNumero de iteraciones hasta converger:")
print(kmeans.n_iter_)

# Graficar los grupos
plt.figure(figsize=(8, 6))

for k in range(3):
    plt.scatter(
        X[grupos == k, 2],
        X[grupos == k, 3],
        label=f"Grupo {k + 1}"
    )

# Graficar los centroides finales
plt.scatter(
    centroides[:, 2],
    centroides[:, 3],
    marker="X",
    s=250,
    label="Centroides"
)

plt.xlabel("Longitud del petalo (cm)")
plt.ylabel("Ancho del petalo (cm)")
plt.title("K-means con Iris")
plt.legend()
plt.tight_layout()

# Guardar la grafica
plt.savefig("kmeans_iris.png", dpi=150)

# Mostrar la grafica
plt.show()