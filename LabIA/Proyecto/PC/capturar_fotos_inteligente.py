import cv2
import os

# ==============================================================================
# CONFIGURACIÓN
# ==============================================================================
# Cambia este nombre según el producto actual:
# "paracetamol", "ibuprofeno", "atun_lata", "leche_entera"
NOMBRE_PRODUCTO = "paracetamol" 

# Define si estás guardando fotos para entrenamiento ("train") o validación ("val")
MODO = "train"  

DIR_SALIDA = os.path.join("dataset", MODO, NOMBRE_PRODUCTO)
os.makedirs(DIR_SALIDA, exist_ok=True)

# Umbrales de validación de calidad
UMBRAL_DESENFOQUE = 80.0  # Varianza mínima del Laplaciano
BRILLO_MIN = 40           # Control de oscuridad
BRILLO_MAX = 220          # Control de sobreexposición/reflejo

cap = cv2.VideoCapture(0)
# Cuenta las fotos que ya existen en la carpeta para no sobrescribir
contador = len([f for f in os.listdir(DIR_SALIDA) if f.endswith('.jpg')])

print("==================================================")
print(f" Capturando para [{MODO.upper()}]: {NOMBRE_PRODUCTO}")
print(" Presiona 's' para tomar foto")
print(" Presiona 'q' para salir")
print("==================================================")

while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: No se pudo acceder a la cámara.")
        break

    # Convertir a escala de grises para analizar nitidez y luz
    gris = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    enfoque = cv2.Laplacian(gris, cv2.CV_64F).var()
    brillo = gris.mean()

    es_nitida = enfoque >= UMBRAL_DESENFOQUE
    brillo_correcto = BRILLO_MIN <= brillo <= BRILLO_MAX

    # Feedback en pantalla
    if not es_nitida:
        estado_texto = "¡IMAGEN BORROSA! Sostén firme"
        color = (0, 0, 255)  # Rojo
    elif not brillo_correcto:
        estado_texto = "AJUSTA LA LUZ (Reflejo/Oscuro)"
        color = (0, 165, 255)  # Naranja
    else:
        estado_texto = "NITIDEZ ÓPTIMA - Presiona 's'"
        color = (0, 255, 0)  # Verde

    # Copia limpia para la interfaz gráfica
    display = frame.copy()
    cv2.putText(display, f"Estado: {estado_texto}", (20, 40), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
    cv2.putText(display, f"Guardadas en {MODO}: {contador}", (20, 80), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)

    cv2.imshow("Captura de Dataset - VisioCare AI", display)
    key = cv2.waitKey(1) & 0xFF

    if key == ord('s'):
        if es_nitida and brillo_correcto:
            contador += 1
            ruta_foto = os.path.join(DIR_SALIDA, f"{NOMBRE_PRODUCTO}_{contador}.jpg")
            cv2.imwrite(ruta_foto, frame)  # Se guarda la captura original limpia
            print(f"✓ Guardada foto #{contador} en {DIR_SALIDA}")
        else:
            print(f"✗ Rechazada. Enfoque: {enfoque:.1f} (mín {UMBRAL_DESENFOQUE}) | Brillo: {brillo:.1f}")

    elif key == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()