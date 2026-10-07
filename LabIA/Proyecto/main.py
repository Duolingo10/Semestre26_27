import os
import queue
import threading

import cv2
import easyocr
import pyttsx3
import torch
import torch.nn as nn
from torchvision import models, transforms

ROOT = os.path.dirname(os.path.abspath(__file__))
MODEL_PATH = os.path.join(ROOT, "outputs", "modelo_productos.pth")

CLASES = ("alimentos", "medicamentos")
ROI_WIDTH = 400
ROI_HEIGHT = 400
OCR_CONFIDENCE_MIN = 0.45
DEVICE = torch.device("cpu")

transform_inferencia = transforms.Compose(
    [
        transforms.ToPILImage(),
        transforms.Resize((224, 224)),
        transforms.ToTensor(),
    ]
)


def preprocesar_para_ocr(roi_frame):
    gris = cv2.cvtColor(roi_frame, cv2.COLOR_BGR2GRAY)
    return cv2.adaptiveThreshold(
        gris,
        255,
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
        cv2.THRESH_BINARY,
        31,
        9,
    )


def configurar_voz_espanol(engine):
    engine.setProperty("rate", 165)
    engine.setProperty("volume", 1.0)
    for voice in engine.getProperty("voices"):
        nombre = (voice.name or "").lower()
        identificador = (voice.id or "").lower()
        idiomas = " ".join(str(item).lower() for item in getattr(voice, "languages", []))
        es_espanol = (
            "spanish" in nombre
            or "espanol" in nombre
            or "español" in nombre
            or "es-mx" in identificador
            or "es-es" in identificador
            or "es_mx" in identificador
            or "es_es" in identificador
            or "sabina" in nombre
            or "helena" in nombre
            or "es" in idiomas
        )
        if es_espanol:
            engine.setProperty("voice", voice.id)
            break


def _coordenada_y(bbox):
    return min(punto[1] for punto in bbox)


def _coordenada_x(bbox):
    return min(punto[0] for punto in bbox)


def construir_frase_ocr(detecciones):
    validas = []
    for bbox, texto, confianza in detecciones:
        palabra = " ".join(str(texto).split())
        if palabra and float(confianza) > OCR_CONFIDENCE_MIN:
            validas.append((bbox, palabra, confianza))

    validas.sort(key=lambda item: (_coordenada_y(item[0]), _coordenada_x(item[0])))

    palabras_limpias = []
    vistas = set()
    for _, palabra, _ in validas:
        clave = palabra.casefold()
        if clave in vistas:
            continue
        vistas.add(clave)
        palabras_limpias.append(palabra)

    return " ".join(palabras_limpias).strip()


def extraer_state_dict(checkpoint):
    if not isinstance(checkpoint, dict):
        return checkpoint, list(CLASES)

    if "model_state_dict" in checkpoint:
        state_dict = checkpoint["model_state_dict"]
    elif "state_dict" in checkpoint:
        state_dict = checkpoint["state_dict"]
    else:
        state_dict = checkpoint

    clases = checkpoint.get("classes") or checkpoint.get("class_names")
    if clases:
        clases = [str(nombre).strip().lower() for nombre in clases]
    else:
        clases = list(CLASES)
    return state_dict, clases


def cargar_modelo(ruta):
    if not os.path.isfile(ruta):
        raise FileNotFoundError(
            f"No se encontro el modelo en {ruta}. Entrena primero con entrenar.py."
        )

    modelo = models.mobilenet_v3_small(weights=None)
    in_features = modelo.classifier[-1].in_features
    modelo.classifier[-1] = nn.Linear(in_features, 2)

    checkpoint = torch.load(ruta, map_location=DEVICE, weights_only=True)
    state_dict, clases = extraer_state_dict(checkpoint)

    if len(clases) != 2:
        clases = list(CLASES)

    modelo.load_state_dict(state_dict, strict=True)
    modelo.to(DEVICE)
    modelo.eval()
    return modelo, clases


def predecir_clase(modelo, clases, roi_bgr):
    if roi_bgr is None or roi_bgr.size == 0:
        return "desconocido", 0.0

    roi_rgb = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2RGB)
    tensor = transform_inferencia(roi_rgb).unsqueeze(0).to(DEVICE)
    with torch.no_grad():
        logits = modelo(tensor)
        probs = torch.softmax(logits, dim=1)[0]
        indice = int(torch.argmax(probs).item())
        confianza = float(probs[indice].item())
    nombre = clases[indice] if 0 <= indice < len(clases) else "desconocido"
    return nombre, confianza


def etiqueta_visible(clase):
    mapa = {
        "alimentos": "Alimento",
        "alimento": "Alimento",
        "medicamentos": "Medicamento",
        "medicamento": "Medicamento",
    }
    return mapa.get(str(clase).strip().lower(), str(clase).capitalize())


def iniciar_motor_voz():
    cola = queue.Queue(maxsize=1)
    stop_event = threading.Event()

    def trabajador():
        try:
            engine = pyttsx3.init()
            configurar_voz_espanol(engine)
        except Exception as exc:
            print(f"No se pudo iniciar pyttsx3: {exc}")
            return

        while not stop_event.is_set():
            try:
                frase = cola.get(timeout=0.25)
            except queue.Empty:
                continue
            if frase is None:
                break
            try:
                engine.say(frase)
                engine.runAndWait()
            except Exception as exc:
                print(f"Error de voz: {exc}")
        try:
            engine.stop()
        except Exception:
            pass

    hilo = threading.Thread(target=trabajador, daemon=True)
    hilo.start()
    return cola, stop_event


def encolar_voz(cola, frase):
    if frase is None:
        try:
            cola.put_nowait(None)
        except queue.Full:
            pass
        return
    if not frase:
        return
    try:
        while True:
            cola.get_nowait()
    except queue.Empty:
        pass
    try:
        cola.put_nowait(frase)
    except queue.Full:
        pass


def iniciar_ocr():
    try:
        return easyocr.Reader(["es", "en"], gpu=False, download_enabled=False)
    except TypeError:
        return easyocr.Reader(["es", "en"], gpu=False)


def dibujar_texto(frame, texto, origen, color, escala=0.65):
    cv2.putText(
        frame,
        texto,
        origen,
        cv2.FONT_HERSHEY_SIMPLEX,
        escala,
        (0, 0, 0),
        4,
        cv2.LINE_AA,
    )
    cv2.putText(
        frame,
        texto,
        origen,
        cv2.FONT_HERSHEY_SIMPLEX,
        escala,
        color,
        2,
        cv2.LINE_AA,
    )


def recortar_roi(frame, x1, y1, x2, y2):
    alto, ancho = frame.shape[:2]
    rx1 = max(0, min(x1, ancho - 1))
    ry1 = max(0, min(y1, alto - 1))
    rx2 = max(rx1 + 1, min(x2, ancho))
    ry2 = max(ry1 + 1, min(y2, alto))
    return frame[ry1:ry2, rx1:rx2].copy(), (rx1, ry1, rx2, ry2)


def analizar_captura(modelo, clases, lector, roi_bgr, estado, cola_voz):
    try:
        clase, confianza = predecir_clase(modelo, clases, roi_bgr)
        clasificacion = etiqueta_visible(clase)
        try:
            detecciones = lector.readtext(preprocesar_para_ocr(roi_bgr))
            texto = construir_frase_ocr(detecciones)
        except Exception as exc:
            print(f"Error de OCR: {exc}")
            texto = ""

        if not texto:
            texto = "sin texto"

        frase = f"Clasificacion: {clasificacion}. Texto detectado: {texto}"
        print(frase)
        with estado["lock"]:
            estado["analizando"] = False
            estado["clase"] = clasificacion
            estado["confianza"] = confianza
            estado["texto"] = texto
            estado["mensaje"] = f"{clasificacion} | {texto}"
        encolar_voz(
            cola_voz,
            f"Clasificación: {clasificacion}. Texto detectado: {texto}",
        )
    except Exception as exc:
        print(f"Error al analizar la captura: {exc}")
        with estado["lock"]:
            estado["analizando"] = False
            estado["mensaje"] = "Error al analizar. Intenta de nuevo."


def main():
    modelo, clases = cargar_modelo(MODEL_PATH)
    print(f"Modelo cargado en CPU. Clases: {clases}")

    try:
        lector = iniciar_ocr()
    except Exception as exc:
        raise RuntimeError(
            "EasyOCR no pudo iniciar en modo offline. "
            "Ejecuta una vez con internet para cachear los modelos."
        ) from exc

    cola_voz, stop_voz = iniciar_motor_voz()

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        stop_voz.set()
        raise RuntimeError("No se pudo abrir la webcam en el indice 0.")

    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

    frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)) or 640
    frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT)) or 480
    roi_width = min(ROI_WIDTH, frame_width)
    roi_height = min(ROI_HEIGHT, frame_height)
    x1 = (frame_width - roi_width) // 2
    y1 = (frame_height - roi_height) // 2
    x2 = x1 + roi_width
    y2 = y1 + roi_height

    estado = {
        "lock": threading.Lock(),
        "analizando": False,
        "clase": "",
        "confianza": 0.0,
        "texto": "",
        "mensaje": "Espacio: analizar  |  q: salir",
    }

    try:
        while True:
            ret, frame = cap.read()
            if not ret or frame is None:
                print("No se pudo leer un frame de la camara.")
                break

            roi_frame, (rx1, ry1, rx2, ry2) = recortar_roi(frame, x1, y1, x2, y2)

            with estado["lock"]:
                analizando = estado["analizando"]
                clase_actual = estado["clase"]
                confianza_actual = estado["confianza"]
                texto_ocr = estado["texto"]
                mensaje = estado["mensaje"]

            cv2.rectangle(frame, (rx1, ry1), (rx2, ry2), (0, 255, 0), 3)
            dibujar_texto(
                frame,
                "Coloque el producto aqui",
                (rx1, max(30, ry1 - 12)),
                (0, 255, 0),
                0.7,
            )
            dibujar_texto(frame, mensaje, (20, 40), (0, 255, 255), 0.6)

            if clase_actual:
                dibujar_texto(
                    frame,
                    f"Clase: {clase_actual} ({confianza_actual:.2f})",
                    (20, 75),
                    (0, 255, 255),
                )
            if texto_ocr:
                texto_pantalla = texto_ocr if len(texto_ocr) <= 60 else texto_ocr[:57] + "..."
                dibujar_texto(frame, f"OCR: {texto_pantalla}", (20, 110), (255, 255, 255), 0.55)

            cv2.imshow("VisioCare AI", frame)
            tecla = cv2.waitKey(1) & 0xFF

            if tecla == ord("q"):
                break

            if tecla == 32 and not analizando:
                captura = roi_frame.copy()
                with estado["lock"]:
                    estado["analizando"] = True
                    estado["mensaje"] = "Analizando captura..."
                hilo = threading.Thread(
                    target=analizar_captura,
                    args=(modelo, clases, lector, captura, estado, cola_voz),
                    daemon=True,
                )
                hilo.start()
    finally:
        cap.release()
        cv2.destroyAllWindows()
        stop_voz.set()
        encolar_voz(cola_voz, None)


if __name__ == "__main__":
    main()
