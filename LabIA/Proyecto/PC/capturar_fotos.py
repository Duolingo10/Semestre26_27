import cv2
import os

# ==============================================================================
# CONFIGURACIÓN
# ==============================================================================
NOMBRE_PRODUCTO = "paracetamol" # "paracetamol", "ibuprofeno", "atun_lata", "leche_entera"
MODO = "train"                  # "train" para entrenamiento, "val" para validación

DIR_SALIDA = os.path.join("dataset", MODO, NOMBRE_PRODUCTO)
os.makedirs(DIR_SALIDA, exist_ok=True)

# Umbrales de calidad
UMBRAL_DESENFOQUE = 80.0
BRILLO_MIN = 40
BRILLO_MAX = 220

cap = cv2.VideoCapture(0)
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

    gris = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    enfoque = cv2.Laplacian(gris, cv2.CV_64F).var()
    brillo = gris.mean()

    es_nitida = enfoque >= UMBRAL_DESENFOQUE
    brillo_correcto = BRILLO_MIN <= brillo <= BRILLO_MAX

    if not es_nitida:
        estado_texto = "¡IMAGEN BORROSA! Sostén firme"
        color = (0, 0, 255)
    elif not brillo_correcto:
        estado_texto = "AJUSTA LA LUZ (Reflejo/Oscuro)"
        color = (0, 165, 255)
    else:
        estado_texto = "NITIDEZ ÓPTIMA - Presiona 's'"
        color = (0, 255, 0)

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
            cv2.imwrite(ruta_foto, frame)
            print(f"✓ Guardada foto #{contador} en {DIR_SALIDA}")
        else:
            print(f"✗ Rechazada. Enfoque: {enfoque:.1f} | Brillo: {brillo:.1f}")

    elif key == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()