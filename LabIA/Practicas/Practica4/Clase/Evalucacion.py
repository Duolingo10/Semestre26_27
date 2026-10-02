# 3_evaluacion_test_mejor_modelo.py
# Usa GridSearchCV (mismo espacio de búsqueda) para hallar el mejor modelo
# y evaluarlo en el set de test: accuracy, F1-macro, reporte; guarda matriz de confusión.
# Seed: 42. Carga Iris y divide en train (120) y test (30).
import matplotlib.pyplot as plt
import numpy as np
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split, GridSearchCV
from sklearn.neighbors import KNeighborsClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import Pipeline
from sklearn.metrics import accuracy_score, f1_score, classification_report, confusion_matrix, ConfusionMatrixDisplay

RANDOM_STATE = 42

# 1) Datos y split 120/30
X, y = load_iris(return_X_y=True)
X_train, X_test, y_train, y_test = train_test_split(
    X, y, train_size=120, test_size=30, stratify=y, random_state=RANDOM_STATE
)

# 2) GridSearchCV (idéntico al script 2)
pipe = Pipeline([
    ("scaler", StandardScaler()),
    ("knn", KNeighborsClassifier())
])

param_grid = {
    "knn__n_neighbors": list(range(1, 30)),
    "knn__weights": ["uniform", "distance"],
    "knn__p": [1, 2]
}

grid = GridSearchCV(
    estimator=pipe,
    param_grid=param_grid,
    scoring="accuracy",
    cv=5,
    n_jobs=-1,
    refit=True
)
grid.fit(X_train, y_train)

best_model = grid.best_estimator_
print("[MEJOR ESTIMADOR]")
print(best_model)
print("[MEJORES PARÁMETROS]")
print(grid.best_params_)
print(f"[MEJOR SCORE CV] {grid.best_score_:.4f}\n")

# 3) Evaluación en test
y_pred = best_model.predict(X_test)
acc = accuracy_score(y_test, y_pred)
f1m = f1_score(y_test, y_pred, average="macro")
print(f"Accuracy (test): {acc:.4f}")
print(f"F1-macro (test): {f1m:.4f}")
print("\n[Clasification Report]\n")
print(classification_report(y_test, y_pred))

# 4) Matriz de confusión
cm = confusion_matrix(y_test, y_pred)
disp = ConfusionMatrixDisplay(confusion_matrix=cm)
disp.plot(values_format="d")
plt.title("Matriz de confusión - Mejor KNN (con StandardScaler)")
out_png = "3_matriz_confusion_knn.png"
plt.tight_layout()
plt.savefig(out_png, dpi=120)
print(f"\n[OK] Matriz de confusión guardada en: {out_png}")