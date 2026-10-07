import os
import shutil
import random

# Rutas de origen  y destino 
ruta_origen = r"C:\Users\juana\.cache\kagglehub\datasets\bhavikjikadara\grocery-store-dataset\versions\1"
ruta_train = r"dataset\train\alimentos"
ruta_val = r"dataset\val\alimentos"

# Crear carpetas
os.makedirs(ruta_train, exist_ok=True)
os.makedirs(ruta_val, exist_ok=True)

# Buscar todas las imágenes recursivamente en el dataset descargado
todas_las_imagenes = []
for raiz, _, archivos in os.walk(ruta_origen):
    for archivo in archivos:
        if archivo.lower().endswith(('.png', '.jpg', '.jpeg')):
            todas_las_imagenes.append(os.path.join(raiz, archivo))

# Verificar que hay imágenes en el dataset
if not todas_las_imagenes:
    print("No se encontraron imágenes en la ruta de origen.")
else:
    print(f"Se encontraron {len(todas_las_imagenes)} imágenes. Mezclando y copiando...")
    random.shuffle(todas_las_imagenes)

    # Copiar 300 para entrenamiento
    for i, img_path in enumerate(todas_las_imagenes[:300]):
        shutil.copy(img_path, os.path.join(ruta_train, f"alimento_train_{i}.jpg"))
    
    # Copiar 50 para validación
    for i, img_path in enumerate(todas_las_imagenes[300:350]):
        shutil.copy(img_path, os.path.join(ruta_val, f"alimento_val_{i}.jpg"))

    print("¡Listo! Las imágenes fueron copiadas a tus carpetas dataset/train/alimentos y dataset/val/alimentos.")