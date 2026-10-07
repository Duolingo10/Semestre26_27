import kagglehub
import os
import shutil
import random

print("1. Descargando cajas de medicamentos...")
ruta_descarga = kagglehub.dataset_download("melikeklahc/yolocilin-medicine-box-detection")
print(f"Descarga completa en: {ruta_descarga}")

ruta_train = "dataset/train/medicamentos"
ruta_val = "dataset/val/medicamentos"
os.makedirs(ruta_train, exist_ok=True)
os.makedirs(ruta_val, exist_ok=True)

# Ignorar los archivos .txt y agarrar solo las imágenes
imagenes = []
for raiz, _, archivos in os.walk(ruta_descarga):
    for arch in archivos:
        if arch.lower().endswith(('.png', '.jpg', '.jpeg')):
            imagenes.append(os.path.join(raiz, arch))

if not imagenes:
    print("Error: No se encontraron imágenes en el dataset.")
else:
    print(f"¡Éxito! Se encontraron {len(imagenes)} imágenes de empaques. Extrayendo muestra...")
    random.shuffle(imagenes)
    
    limite_train = min(300, int(len(imagenes) * 0.8))
    
    for i, img in enumerate(imagenes[:limite_train]):
        shutil.copy(img, os.path.join(ruta_train, f"medicina_train_{i}.jpg"))
        
    for i, img in enumerate(imagenes[limite_train:limite_train+50]):
        shutil.copy(img, os.path.join(ruta_val, f"medicina_val_{i}.jpg"))

    print(f"Listo. {limite_train} empaques a train y {min(len(imagenes)-limite_train, 50)} a val.")