
import sklearn
from sklearn.datasets import load_breast_cancer
from sklearn.model_selection import train_test_split
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns


# ANALISIS DEL DATASET


cancer = load_breast_cancer(as_frame=True)
df = cancer.frame.copy()

df["diagnosis"] = df["target"].map(dict(enumerate(cancer.target_names)))

df.columns = [
    "mean_radius", "mean_texture", "mean_perimeter", "mean_area",
    "mean_smoothness", "mean_compactness", "mean_concavity",
    "mean_concave_points", "mean_symmetry", "mean_fractal_dimension",
    "radius_error", "texture_error", "perimeter_error", "area_error",
    "smoothness_error", "compactness_error", "concavity_error",
    "concave_points_error", "symmetry_error", "fractal_dimension_error",
    "worst_radius", "worst_texture", "worst_perimeter", "worst_area",
    "worst_smoothness", "worst_compactness", "worst_concavity",
    "worst_concave_points", "worst_symmetry", "worst_fractal_dimension",
    "target", "diagnosis"
]

print("scikit-learn:", sklearn.__version__)

print("\nDimensiones:", df.shape)
print("\nPrimeras filas:", df.head())
print("\nTipos:", df.dtypes)
print("\nValores Nulos:", df.isna().sum())
print("\nDuplicados:", df.duplicated().sum())
print("\nEstadistica descriptiva:", df.describe())

print("\nCantidad por diagnostico:")
print(df["diagnosis"].value_counts())

print("\nEstadistica por diagnostico:")
print(
    df.groupby("diagnosis")[
        ["mean_radius", "mean_texture", "mean_perimeter", "mean_area"]
    ].agg(["mean", "std", "min", "max"]).round(3)
)


# CODIGO #2: GRAFICAS DE VISUALIZACION


features = [
    "mean_radius",
    "mean_texture",
    "mean_perimeter",
    "mean_area"
]

plt.figure(figsize=(12, 8))

for i, col in enumerate(features, start=1):
    plt.subplot(2, 2, i)
    sns.histplot(
        data=df,
        x=col,
        hue="diagnosis",
        kde=True,
        element="step"
    )
    plt.title(f"Histograma: {col}")

plt.tight_layout()
plt.savefig("01_histogramas.png", dpi=150)
plt.show()


fig, axes = plt.subplots(nrows=2, ncols=2, figsize=(12, 10))
axes = axes.flatten()

sns.scatterplot(
    data=df,
    x="mean_radius",
    y="mean_texture",
    hue="diagnosis",
    s=70,
    ax=axes[0]
)
axes[0].set_title("Radio promedio vs Textura promedio")

sns.scatterplot(
    data=df,
    x="mean_perimeter",
    y="mean_area",
    hue="diagnosis",
    s=70,
    ax=axes[1]
)
axes[1].set_title("Perimetro promedio vs Area promedio")

sns.scatterplot(
    data=df,
    x="mean_radius",
    y="mean_perimeter",
    hue="diagnosis",
    s=70,
    ax=axes[2]
)
axes[2].set_title("Radio promedio vs Perimetro promedio")

sns.scatterplot(
    data=df,
    x="mean_texture",
    y="mean_area",
    hue="diagnosis",
    s=70,
    ax=axes[3]
)
axes[3].set_title("Textura promedio vs Area promedio")

plt.tight_layout()
plt.savefig("02_scatterplots.png", dpi=150)
plt.show()


# CODIGO #3: DIVISION EN TRAIN Y TEST

X = df[features].copy()
Y = df["diagnosis"].copy()

X_train, X_test, Y_train, Y_test = train_test_split(
    X,
    Y,
    test_size=0.2,
    random_state=42,
    stratify=Y
)

print("\nDimensiones del conjunto de entrenamiento:", X_train.shape)
print("\nDimensiones del conjunto de prueba:", X_test.shape)

print("\nClases en entrenamiento:")
print(Y_train.value_counts().sort_index())

print("\nClases en prueba:")
print(Y_test.value_counts().sort_index())

train_distribution = Y_train.value_counts(normalize=True).sort_index()
print("\nDistribucion de clases en entrenamiento:")
print(train_distribution)

test_distribution = Y_test.value_counts(normalize=True).sort_index()
print("\nDistribucion de clases en prueba:")
print(test_distribution)

X_train.to_csv("X_train.csv", index=False)
X_test.to_csv("X_test.csv", index=False)
Y_train.to_csv("Y_train.csv", index=False)
Y_test.to_csv("Y_test.csv", index=False)
