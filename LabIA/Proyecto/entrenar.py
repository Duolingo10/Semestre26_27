import os

import matplotlib.pyplot as plt
import seaborn as sns
import torch
import torch.nn as nn
from sklearn.metrics import accuracy_score, confusion_matrix, precision_score, recall_score
from torch.utils.data import DataLoader
from torchvision import datasets, models, transforms

ROOT = os.path.dirname(os.path.abspath(__file__))
TRAIN_DIR = os.path.join(ROOT, "dataset", "train")
VAL_DIR = os.path.join(ROOT, "dataset", "val")
OUTPUT_DIR = os.path.join(ROOT, "outputs")
MODEL_PATH = os.path.join(OUTPUT_DIR, "modelo_productos.pth")
CM_PATH = os.path.join(OUTPUT_DIR, "matriz_confusion.png")

EPOCHS = 3
BATCH_SIZE = 8
LEARNING_RATE = 1e-3

device = torch.device("cpu")
print(f"Dispositivo de entrenamiento: {device}")

transformaciones = transforms.Compose(
    [
        transforms.Resize((224, 224)),
        transforms.ToTensor(),
    ]
)

train_dataset = datasets.ImageFolder(TRAIN_DIR, transform=transformaciones)
val_dataset = datasets.ImageFolder(VAL_DIR, transform=transformaciones)

num_clases = len(train_dataset.classes)
if num_clases != 2:
    raise RuntimeError(
        f"Se esperaban 2 clases actuales, pero ImageFolder encontro {num_clases}: {train_dataset.classes}"
    )
if train_dataset.classes != val_dataset.classes:
    raise RuntimeError(
        "Las carpetas de train y val no coinciden en clases: "
        f"{train_dataset.classes} vs {val_dataset.classes}"
    )

print(f"Clases: {train_dataset.classes}")
print(f"Imagenes de entrenamiento: {len(train_dataset)}")
print(f"Imagenes de validacion: {len(val_dataset)}")

train_loader = DataLoader(
    train_dataset,
    batch_size=BATCH_SIZE,
    shuffle=True,
    num_workers=0,
)
val_loader = DataLoader(
    val_dataset,
    batch_size=BATCH_SIZE,
    shuffle=False,
    num_workers=0,
)

modelo = models.mobilenet_v3_small(weights=models.MobileNet_V3_Small_Weights.DEFAULT)

for parametro in modelo.parameters():
    parametro.requires_grad = False

in_features = modelo.classifier[-1].in_features
modelo.classifier[-1] = nn.Linear(in_features, num_clases)
modelo = modelo.to(device)

criterio = nn.CrossEntropyLoss()
optimizador = torch.optim.Adam(modelo.classifier[-1].parameters(), lr=LEARNING_RATE)

print("Iniciando entrenamiento (3 epochs, CPU)...")
modelo.train()
for epoch in range(EPOCHS):
    perdida_acumulada = 0.0
    aciertos = 0
    total = 0

    for imagenes, etiquetas in train_loader:
        imagenes = imagenes.to(device)
        etiquetas = etiquetas.to(device)

        optimizador.zero_grad()
        logits = modelo(imagenes)
        perdida = criterio(logits, etiquetas)
        perdida.backward()
        optimizador.step()

        perdida_acumulada += perdida.item() * imagenes.size(0)
        predicciones = torch.argmax(logits, dim=1)
        aciertos += (predicciones == etiquetas).sum().item()
        total += etiquetas.size(0)

    perdida_media = perdida_acumulada / total
    accuracy_train = aciertos / total
    print(
        f"Epoch {epoch + 1}/{EPOCHS} | "
        f"loss: {perdida_media:.4f} | "
        f"accuracy train: {accuracy_train:.4f}"
    )

os.makedirs(OUTPUT_DIR, exist_ok=True)
torch.save(
    {
        "model_state_dict": modelo.state_dict(),
        "classes": train_dataset.classes,
        "class_to_idx": train_dataset.class_to_idx,
    },
    MODEL_PATH,
)
print(f"Modelo guardado en: {MODEL_PATH}")

print("Evaluando en validacion...")
modelo.eval()
y_true = []
y_pred = []

with torch.no_grad():
    for imagenes, etiquetas in val_loader:
        imagenes = imagenes.to(device)
        etiquetas = etiquetas.to(device)
        logits = modelo(imagenes)
        predicciones = torch.argmax(logits, dim=1)
        y_true.extend(etiquetas.cpu().tolist())
        y_pred.extend(predicciones.cpu().tolist())

accuracy = accuracy_score(y_true, y_pred)
precision = precision_score(
    y_true,
    y_pred,
    average="macro",
    zero_division=0,
)
recall = recall_score(
    y_true,
    y_pred,
    average="macro",
    zero_division=0,
)

print("=== Metricas de validacion ===")
print(f"Accuracy : {accuracy:.4f}")
print(f"Precision: {precision:.4f}")
print(f"Recall   : {recall:.4f}")

matriz = confusion_matrix(y_true, y_pred, labels=list(range(num_clases)))
plt.figure(figsize=(6, 5))
sns.heatmap(
    matriz,
    annot=True,
    fmt="d",
    cmap="YlOrRd",
    xticklabels=train_dataset.classes,
    yticklabels=train_dataset.classes,
)
plt.title("Matriz de confusion - Validacion")
plt.xlabel("Prediccion")
plt.ylabel("Etiqueta real")
plt.tight_layout()
plt.savefig(CM_PATH, dpi=150)
plt.close()
print(f"Matriz de confusion guardada en: {CM_PATH}")
