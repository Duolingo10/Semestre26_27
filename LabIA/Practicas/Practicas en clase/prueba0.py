
import sklearn
from sklearn.datasets import load_iris
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

print("skLearn", sklearn.__version__)
iris = load_iris(as_frame=True)
df = iris.frame.copy()
df["species"] = df["target"].map(dict(enumerate(iris.target_names)))
df.columns = ["sepal_length", "sepal_width", "petal_length", "petal_width", "target", "species"]

print("\nDimensiones: ", df.shape)
print("\nPrimeras filas: ", df.head())
print("\ntipos: ", df.dtypes)
print("\nValores Nulos: ", df.isna().sum())
print("\nDuplicados: ", df.duplicated().sum())
print("\nEstadistica descritiva: ", df.describe())
print("\nEstadistica por especie\n: ", df.groupby("species").agg(["mean", "std", "min", "max"]).round(3))

features = ["sepal_length", "sepal_width", "petal_length", "petal_width"]

"""
for i, col in enumerate(features, start=1):
plt.figure(figsize=(7, 4.5))
sns.histplot(data=df, x=col, hue="species", kde=True, element="step")
#old x=features[1]
plt.title(f"figura {col}")
plt.tight_layout()
plt.savefig(f"{i:02d}_hist_{col}.png", dpi=150)
plt.show()
"""

plt.figure(figsize=(12, 8))

for i, col in enumerate(features, start=1):
    plt.subplot(2, 2, i)
sns.histplot(data=df, x=col, hue="species", kde=True, element="step")
#old x=features[1]
plt.title(f"figura {col}")

plt.tight_layout()
plt.savefig("01_histogramas.png", dpi=150)
plt.show()


plt.figure(figsize=(7.5, 4.5))
long_df = df.melt(id_vars="species", value_vars=features, var_name="feature", value_name="value")
sns.boxenplot(data=long_df, x="feature", y="value", hue="species")
#old x=features[1]
plt.title("Boxplots de caracterisiticas por especies")
plt.tight_layout()
plt.savefig("05_boxplots.png", dpi=150)
plt.show()


plt.figure(figsize=(12, 5))

plt.subplot(1, 2, 1)
sns.scatterplot(data=df, x="sepal_length", y="sepal_width", hue="species", s=70)
#old x=features[1]
plt.title("Longitud vs ancho del sepalo")

plt.subplot(1, 2, 2)
sns.scatterplot(data=df, x="petal_length", y="petal_width", hue="species", s=70)
#old x=features[1]
plt.title("Longitud vs ancho del petalo")

plt.tight_layout()
plt.savefig("06_07_scatterplots.png", dpi=150)
plt.show()