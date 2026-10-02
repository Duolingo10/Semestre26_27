# 1_curva_validacion_knn.py
# Guarda una Curva de validación (Accuracy vs k) con StandardScaler
# Seed: 42. Carga Iris y divide en train (120) y test (30).
# La curva se calcula con validación cruzada sobre el conjunto de entrenamiento.
import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.neighbors import KNeighborsClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import Pipeline

RANDOM_STATE = 42

# 1) Datos y split 120/30
X, y = load_iris(return_X_y=True)
X_train, X_test, y_train, y_test = train_test_split(
    X, y, train_size=120, test_size=30, stratify=y, random_state=RANDOM_STATE
)

# 2) Rango de k
k_values = list(range(1, 12))

# 3) Pipeline KNN con StandardScaler
def make_pipeline(k):
    return Pipeline([
        ("scaler", StandardScaler()),
        ("knn", KNeighborsClassifier(n_neighbors=k))
    ])

# 4) Validación cruzada en train (5 folds)
cv_scores_mean = []
cv_scores_std = []

for k in k_values:
    pipe = make_pipeline(k)
    scores = cross_val_score(pipe, X_train, y_train, cv=5, scoring="accuracy", n_jobs=-1)
    cv_scores_mean.append(scores.mean())
    cv_scores_std.append(scores.std())

# 5) Gráfica Accuracy vs k (validación)
plt.figure(figsize=(8,5))
plt.plot(k_values, cv_scores_mean, marker="o")
plt.fill_between(k_values,
                 np.array(cv_scores_mean) - np.array(cv_scores_std),
                 np.array(cv_scores_mean) + np.array(cv_scores_std),
                 alpha=0.15)
plt.title("Curva de validación: Accuracy vs k (5-fold CV, con StandardScaler)")
plt.xlabel("Número de vecinos (k)")
plt.ylabel("Accuracy (validación cruzada)")
plt.xticks(k_values)
plt.grid(True, linestyle="--", alpha=0.4)
out_png = "1_curva_validacion_knn.png"
plt.tight_layout()
plt.savefig(out_png, dpi=120)
print(f"[OK] Gráfica guardada en: {out_png}")