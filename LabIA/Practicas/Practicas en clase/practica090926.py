import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.datasets import load_iris

datos=load_iris(as_frame=True)
nombre = "Iris"
df = datos.data.copy()
matriz_correlacion = df.corr()
print(matriz_correlacion, round(2))

plt.figure(figsize=(12, 9))
tamano_letra = 12
sns.heatmap(matriz_correlacion, annot=True, cmap='coolwarm', fmt='.2f', annot_kws={"size": tamano_letra})

sns.heatmap(matriz_correlacion, annot=True, cmap='coolwarm', fmt='.2f', annot_kws={"size": tamano_letra})

plt.title(f'Matriz de correlación de {nombre}')
plt.xticks(rotation=45, ha='right', fontsize=tamano_letra)
plt.yticks(rotation=0, fontsize=tamano_letra)
plt.tight_layout()
plt.savefig(f'Practicas/matriz_correlacion_{nombre}.png')

