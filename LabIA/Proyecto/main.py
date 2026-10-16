import difflib
import os
import queue
import re
import subprocess
import sys
import textwrap
import threading
import time
import unicodedata
from collections import Counter

import cv2
import easyocr
import numpy as np
import pyttsx3
import torch
import torch.nn as nn
from PIL import Image, ImageDraw, ImageFont
from torchvision import models, transforms

# ----------------------------------------------------------------------------
# Configuracion
# ----------------------------------------------------------------------------
ROOT = os.path.dirname(os.path.abspath(__file__))
MODEL_PATH = os.path.join(ROOT, "outputs", "modelo_productos.pth")
MEDICAMENTOS_EXTRA = os.path.join(ROOT, "medicamentos.txt")  # opcional, uno por linea

CLASES = ("alimentos", "medicamentos")
ROI_WIDTH = 400
ROI_HEIGHT = 400
DEVICE = torch.device("cpu")

N_CAPTURAS = 3            # fotogramas que se analizan por pulsacion
INTERVALO_CAPTURAS = 0.2  # segundos entre fotogramas
MIN_VOTOS = 2             # un dato debe repetirse en al menos 2 capturas
CONF_OCR_MIN = 0.30       # confianza minima de EasyOCR por deteccion
CONF_NOMBRE_ALIMENTO = 0.55
CONF_CLASE_MIN = 0.70     # por debajo, la clase se considera "no segura"

transform_inferencia = transforms.Compose(
    [
        transforms.ToPILImage(),
        transforms.Resize((224, 224)),
        transforms.ToTensor(),
    ]
)

# Nombres conocidos (sin acentos, minusculas). Se pueden ampliar con medicamentos.txt
MEDICAMENTOS = {
    "paracetamol", "acetaminofen", "ibuprofeno", "naproxeno", "diclofenaco",
    "ketorolaco", "ketoprofeno", "aspirina", "acido acetilsalicilico",
    "metamizol", "dipirona", "nimesulida", "tramadol", "loratadina",
    "cetirizina", "desloratadina", "clorfenamina", "hidroxicina", "ambroxol",
    "bromhexina", "dextrometorfano", "guaifenesina", "pseudoefedrina",
    "fenilefrina", "salbutamol", "omeprazol", "pantoprazol", "ranitidina",
    "loperamida", "butilhioscina", "metoclopramida", "amoxicilina",
    "azitromicina", "ciprofloxacino", "claritromicina", "cefalexina",
    "penicilina", "metronidazol", "trimetoprima", "sulfametoxazol",
    "metformina", "glibenclamida", "losartan", "enalapril", "captopril",
    "amlodipino", "atorvastatina", "simvastatina", "levotiroxina",
    "clonazepam", "diazepam", "alprazolam", "sertralina", "fluoxetina",
    "prednisona", "dexametasona", "betametasona", "melatonina",
    "clotrimazol", "nistatina", "fluconazol", "albendazol", "furosemida",
    "hidroclorotiazida", "carvedilol", "propranolol", "tempra", "tylenol",
    "advil", "motrin", "tabcin", "desenfriol", "sensibit", "dolac",
}

# Palabras de empaque que NUNCA son el nombre del producto
PALABRAS_VACIAS = {
    "oral", "tableta", "tabletas", "capsula", "capsulas", "comprimido",
    "comprimidos", "gragea", "grageas", "pastilla", "pastillas", "cada",
    "contiene", "contenido", "neto", "venta", "libre", "receta", "medica",
    "caja", "envase", "con", "para", "lote", "fabricado", "elaborado",
    "mexico", "sanitario", "registro", "reg", "ssa", "dosis", "adultos",
    "adulto", "ninos", "uso", "agite", "solucion", "suspension", "jarabe",
    "gotas", "analgesico", "antipiretico", "excipiente", "marca",
    "laboratorios", "laboratorio", "farmaceutica", "industria", "hecho",
    "via", "administracion", "tomar", "como", "indicaciones", "advertencias",
    "precauciones", "caducidad", "caduca", "fecha", "miligramos", "gramos",
    "mililitros", "pieza", "piezas", "sobre", "sobres", "cada", "una",
    "unas", "ingredientes", "informacion", "nutrimental", "porcion",
    "porciones", "producto", "consumir", "preferentemente", "antes",
    "conservese", "lugar", "fresco", "seco", "envasado", "importado",
    "distribuido", "tamano", "energia", "grasa", "grasas", "azucares",
    "sodio", "proteina", "proteinas", "carbohidratos", "total",
}

VOCAB_UNIDADES = {
    "tabletas", "tableta", "capsulas", "capsula", "comprimidos",
    "comprimido", "grageas", "pastillas", "miligramos", "mililitros",
}

UNIDAD_CANON = {
    "mg": "mg", "miligramo": "mg", "miligramos": "mg",
    "mcg": "mcg", "ug": "mcg",
    "g": "g", "gramo": "g", "gramos": "g",
    "kg": "kg",
    "ml": "ml", "mililitro": "ml", "mililitros": "ml",
    "l": "l", "litro": "l", "litros": "l",
    "ui": "UI",
}
UNIDADES_MEDICINA = {"mg", "mcg", "g", "ml", "UI"}
UNIDADES_ALIMENTO = {"g", "kg", "ml", "l"}

VOZ_UNIDADES = {
    "mg": "miligramos", "mcg": "microgramos", "g": "gramos", "kg": "kilogramos",
    "ml": "mililitros", "l": "litros", "UI": "unidades internacionales",
}
FORMA_VISIBLE = {
    "tableta": "tableta", "capsula": "cápsula", "comprimido": "comprimido",
    "gragea": "gragea", "pastilla": "pastilla", "sobre": "sobre",
    "pieza": "pieza", "ampolleta": "ampolleta", "parche": "parche",
}

RE_MEDIDA = re.compile(
    r"(?<![\d.,])(\d+(?:[.,]\d+)?)\s*"
    r"(miligramos?|mililitros?|gramos?|litros?|mcg|ug|mg|ml|kg|ui|g|l)\b"
)
RE_CANTIDAD = re.compile(
    r"(?<![\d.,])(\d{1,4})\s*"
    r"(tabletas?|capsulas?|comprimidos?|grageas?|pastillas?|sobres?|piezas?|"
    r"ampolletas?|parches?)\b"
)
RE_CADUCIDAD = re.compile(
    r"(?:cad(?:uca|ucidad)?|exp(?:ira)?|venc\w*)\.?\s*[:\-]?\s*"
    r"((?:\d{1,2}\s*[/\-. ]\s*)?\d{1,2}\s*[/\-. ]\s*\d{2,4}|[a-z]{3}\s*[/\-. ]?\s*\d{2,4})"
)


# ----------------------------------------------------------------------------
# Utilidades de texto
# ----------------------------------------------------------------------------
def normalizar(texto):
    """Pasa el texto a minusculas y le quita acentos y tildes (ñ -> n).
    Sirve para comparar palabras sin que 'Cápsula' y 'capsula' se vean distintas.
    """
    nfd = unicodedata.normalize("NFD", str(texto))
    return "".join(c for c in nfd if unicodedata.category(c) != "Mn").lower()


def cargar_medicamentos():
    """Arma la lista de medicamentos conocidos: la lista incluida en el codigo
    mas los nombres de 'medicamentos.txt' (opcional, uno por linea).
    """
    conocidos = {normalizar(m) for m in MEDICAMENTOS}
    if os.path.isfile(MEDICAMENTOS_EXTRA):
        with open(MEDICAMENTOS_EXTRA, encoding="utf-8") as f:
            for linea in f:
                linea = normalizar(linea.strip())
                if linea:
                    conocidos.add(linea)
    return sorted(conocidos)


LISTA_MEDICAMENTOS = cargar_medicamentos()
LISTA_VACIAS = sorted(PALABRAS_VACIAS)


def limpiar_texto(texto):
    """Prepara el texto crudo del OCR para buscar datos con regex:
    - normaliza (minusculas, sin acentos)
    - corrige errores tipicos de dosis ('5OO mq' -> '500 mg')
    - separa numeros de letras ('500mg' -> '500 mg')
    - corrige palabras de unidades mal leidas ('Tebletas' -> 'tabletas')
    Conserva puntos y comas para no romper decimales como 0.5 mg.
    """
    t = normalizar(texto)
    t = re.sub(r"\b\d[\do]*\b", lambda m: m.group().replace("o", "0"), t)  # 5OO -> 500
    t = re.sub(r"(?<=\d)\s*(?:mq|rng|m9|mgs)\b", " mg", t)  # 500 mq -> 500 mg
    t = re.sub(r"[^a-z0-9\s.,/%\-:]", " ", t)
    t = re.sub(r"(\d)([a-z])", r"\1 \2", t)  # 500mg -> 500 mg
    t = re.sub(r"\s+", " ", t).strip()

    corregidos = []
    for tok in t.split():
        solo = re.sub(r"[^a-z]", "", tok)
        if len(solo) >= 6 and solo == tok:
            cerca = difflib.get_close_matches(solo, VOCAB_UNIDADES, n=1, cutoff=0.8)
            if cerca:
                tok = cerca[0]
        corregidos.append(tok)
    return " ".join(corregidos)


def es_palabra_vacia(tok):
    """True si la palabra es de empaque (oral, tabletas, cada, venta...) y por lo
    tanto NO puede ser el nombre del producto. Tolera errores de OCR.
    """
    if tok in PALABRAS_VACIAS:
        return True
    return bool(difflib.get_close_matches(tok, LISTA_VACIAS, n=1, cutoff=0.8))


def parece_palabra(tok):
    """Filtra ruido del OCR (ej. 'ALIYIO'): exige longitud minima, proporcion
    razonable de vocales y rechaza rachas raras de consonantes o vocales.
    """
    if len(tok) < 5 or not tok.isalpha():
        return False
    vocales = sum(c in "aeiou" for c in tok)
    ratio = vocales / len(tok)
    if not 0.25 <= ratio <= 0.60:
        return False
    if re.search(r"[bcdfghjklmnpqrstvwxyz]{5,}", tok):
        return False
    if re.search(r"[aeiou]{3,}", tok):
        return False
    if re.search(r"(.)\1\1", tok):
        return False
    return True


# ----------------------------------------------------------------------------
# Extraccion de datos
# ----------------------------------------------------------------------------
def buscar_medida(texto, permitidas):
    """Busca la primera cantidad con unidad (500 mg, 250 ml...) dentro del texto.
    Solo acepta las unidades indicadas en 'permitidas' y valores razonables.
    Devuelve por ejemplo '500 mg' o '' si no encuentra nada.
    """
    for m in RE_MEDIDA.finditer(texto):
        unidad = UNIDAD_CANON.get(m.group(2))
        if unidad not in permitidas:
            continue
        valor = m.group(1).replace(",", ".")
        try:
            if float(valor) <= 0 or float(valor) > 10000:
                continue
        except ValueError:
            continue
        return f"{valor} {unidad}"
    return ""


def buscar_cantidad(texto):
    """Busca el numero de piezas del envase ('21 tabletas', '1 capsula').
    Pone el plural o singular correcto segun el numero.
    """
    for m in RE_CANTIDAD.finditer(texto):
        n = int(m.group(1))
        if not 1 <= n <= 1000:
            continue
        base = m.group(2).rstrip("s")
        base = FORMA_VISIBLE.get(base, base)
        plural = "" if n == 1 else ("es" if base.endswith("r") else "s")
        return f"{n} {base}{plural}"
    return ""


def buscar_caducidad(texto):
    """Busca una fecha de caducidad precedida de CAD, EXP, caduca o vence.
    Devuelve la fecha normalizada (ej. '05/2027') o ''.
    """
    m = RE_CADUCIDAD.search(texto)
    if not m:
        return ""
    return re.sub(r"\s*([/\-.])\s*", r"\1", m.group(1)).strip().replace(" ", "/")


def buscar_nombre_medicina(detecciones):
    """Busca el nombre del medicamento comparando las palabras del OCR contra la
    lista de conocidos (con tolerancia a errores). Devuelve el mejor candidato
    segun similitud x confianza del OCR, o '' si ninguno coincide.
    """
    mejor = None
    for texto, prob in detecciones:
        for tok in re.findall(r"[a-z]{5,}", normalizar(texto)):
            cerca = difflib.get_close_matches(tok, LISTA_MEDICAMENTOS, n=1, cutoff=0.8)
            if not cerca:
                continue
            ratio = difflib.SequenceMatcher(None, tok, cerca[0]).ratio()
            puntaje = ratio * float(prob)
            if mejor is None or puntaje > mejor[0]:
                mejor = (puntaje, cerca[0])
    return mejor[1].title() if mejor else ""


def buscar_nombre_alimento(detecciones):
    """Nombre generico para productos que no estan en la lista: toma hasta 2
    palabras con buena confianza OCR que no sean de empaque y que parezcan
    palabras reales. (Tambien se usa como 'nombre posible' de medicinas.)
    """
    candidatos = []
    vistos = set()
    for orden, (texto, prob) in enumerate(detecciones):
        if float(prob) < CONF_NOMBRE_ALIMENTO:
            continue
        for tok in re.findall(r"[a-z]+", normalizar(texto)):
            if tok in vistos or es_palabra_vacia(tok) or not parece_palabra(tok):
                continue
            vistos.add(tok)
            candidatos.append((float(prob) * len(tok), orden, tok))
    mejores = sorted(candidatos, reverse=True)[:2]
    mejores.sort(key=lambda c: c[1])
    return " ".join(c[2].title() for c in mejores)


def analizar_texto(detecciones):
    """Extrae todos los datos candidatos de UNA captura a partir de sus
    detecciones OCR [(texto, confianza), ...]: nombre, dosis, contenido,
    cantidad y caducidad.
    """
    texto = limpiar_texto(" ".join(t for t, _ in detecciones))
    return {
        "nombre_med": buscar_nombre_medicina(detecciones),
        "nombre_ali": buscar_nombre_alimento(detecciones),
        "dosis": buscar_medida(texto, UNIDADES_MEDICINA),
        "contenido": buscar_medida(texto, UNIDADES_ALIMENTO),
        "cantidad": buscar_cantidad(texto),
        "caducidad": buscar_caducidad(texto),
    }


def votar(resultados, campo):
    """Combina los resultados de varias capturas para un campo.
    Devuelve (valor, seguro): el valor mas repetido y si aparece en al menos
    MIN_VOTOS capturas (si no, se dira como 'posible').
    """
    valores = [r[campo] for r in resultados if r[campo]]
    if not valores:
        return "", False
    valor, cuantos = Counter(valores).most_common(1)[0]
    minimo = MIN_VOTOS if len(resultados) > 1 else 1
    return valor, cuantos >= minimo


# ----------------------------------------------------------------------------
# Modelo de clasificacion
# ----------------------------------------------------------------------------
def extraer_state_dict(checkpoint):
    """Saca los pesos y los nombres de clase de un checkpoint de PyTorch, sin
    importar si se guardo como state_dict directo, 'model_state_dict' o
    'state_dict'.
    """
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
    """Crea MobileNetV3-Small con 2 salidas, le carga los pesos entrenados desde
    'ruta' y lo deja en modo evaluacion (CPU). Devuelve (modelo, clases).
    """
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
    """Pasa UNA imagen por el modelo y devuelve (nombres de clase, probabilidades).
    Si la imagen esta vacia devuelve ('desconocido', 0.0).
    """
    if roi_bgr is None or roi_bgr.size == 0:
        return "desconocido", 0.0

    roi_rgb = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2RGB)
    tensor = transform_inferencia(roi_rgb).unsqueeze(0).to(DEVICE)
    with torch.no_grad():
        probs = torch.softmax(modelo(tensor), dim=1)[0]
    return [str(c) for c in clases], probs.tolist()


def etiqueta_visible(clase):
    """Convierte el nombre interno de la clase ('medicamentos') al texto que se
    muestra y se dice ('Medicamento').
    """
    mapa = {
        "alimentos": "Alimento",
        "alimento": "Alimento",
        "medicamentos": "Medicamento",
        "medicamento": "Medicamento",
    }
    return mapa.get(str(clase).strip().lower(), str(clase).capitalize())


def clasificar_capturas(modelo, clases, capturas):
    """Clasifica todas las capturas y promedia sus probabilidades para decidir
    entre Alimento y Medicamento. Devuelve (etiqueta, confianza).
    """
    acumulado = np.zeros(len(clases))
    for roi in capturas:
        nombres, probs = predecir_clase(modelo, clases, roi)
        if isinstance(nombres, str):
            continue
        acumulado += np.array(probs)
    acumulado /= max(1, len(capturas))
    indice = int(np.argmax(acumulado))
    return etiqueta_visible(clases[indice]), float(acumulado[indice])


# ----------------------------------------------------------------------------
# OCR
# ----------------------------------------------------------------------------
def iniciar_ocr():
    """Crea el lector EasyOCR (espanol e ingles, CPU). Intenta modo offline y,
    si la version de EasyOCR no lo soporta, usa el modo normal.
    """
    try:
        return easyocr.Reader(["es", "en"], gpu=False, download_enabled=False)
    except TypeError:
        return easyocr.Reader(["es", "en"], gpu=False)


def variantes_ocr(roi_bgr):
    """Genera 2 versiones de la imagen para leerla mejor: gris ampliada x2 y
    otra con contraste mejorado (CLAHE). Evita binarizar a la fuerza.
    """
    gris = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2GRAY)
    gris = cv2.resize(gris, None, fx=2, fy=2, interpolation=cv2.INTER_CUBIC)
    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
    return [gris, clahe.apply(gris)]


def leer_texto(lector, roi_bgr):
    """Ejecuta el OCR sobre cada variante de la imagen, descarta lecturas de baja
    confianza y elimina textos repetidos (se queda con el de mayor confianza).
    Devuelve [(texto, confianza), ...].
    """
    mejores = {}
    for imagen in variantes_ocr(roi_bgr):
        for _bbox, texto, prob in lector.readtext(imagen):
            prob = float(prob)
            if prob < CONF_OCR_MIN:
                continue
            clave = normalizar(texto).strip()
            if len(clave) < 2:
                continue
            if clave not in mejores or prob > mejores[clave][1]:
                mejores[clave] = (str(texto).strip(), prob)
    return list(mejores.values())


# ----------------------------------------------------------------------------
# Mensajes
# ----------------------------------------------------------------------------
def texto_para_voz(texto):
    """Adapta el texto para que suene bien al hablarlo: 'mg' -> 'miligramos',
    'ml' -> 'mililitros', '0.5' -> '0 punto 5', etc.
    """
    def expandir(m):
        """Reemplaza la abreviatura de una medida por su nombre completo."""
        valor = m.group(1)
        unidad = VOZ_UNIDADES.get(m.group(2), m.group(2))
        return f"{valor} {unidad}"

    t = re.sub(r"(\d+(?:\.\d+)?) (mg|mcg|g|kg|ml|l|UI)\b", expandir, texto)
    return re.sub(r"(\d)\.(\d)", r"\1 punto \2", t)


def construir_mensajes(clase, confianza, datos, seguros):
    """Arma el mensaje final a partir de los datos votados.
    Devuelve (texto en pantalla, texto hablado). Decide si es Medicamento,
    Alimento o Producto, marca como 'posible' lo no confirmado y, si no se
    leyo nada, pide acercar el producto.
    """
    es_med = clase == "Medicamento" or bool(datos["nombre_med"])
    if es_med:
        etiqueta = "Medicamento"
    elif confianza < CONF_CLASE_MIN:
        etiqueta = "Producto"
    else:
        etiqueta = "Alimento"

    # Por ahora, todo alimento se anuncia solo como "Alimento"
    if etiqueta == "Alimento":
        return "Alimento", "Alimento"

    nombre = datos["nombre_med"] or datos["nombre_ali"]
    nombre_seguro = bool(datos["nombre_med"]) or seguros["nombre_ali"]
    medida = datos["dosis"] or datos["contenido"]
    medida_segura = seguros["dosis"] or seguros["contenido"]

    if not (nombre or medida or datos["cantidad"]):
        msg = "No pude leer el producto. Acérquelo, evite reflejos y vuelva a intentar."
        return msg, msg

    partes = []
    if nombre:
        partes.append(nombre if nombre_seguro else f"posible {nombre}")
    else:
        partes.append("nombre no identificado")
    if medida:
        partes.append(medida if medida_segura else f"posible {medida}")
    else:
        partes.append("dosis no legible")
    pantalla = f"{etiqueta}: " + ", ".join(partes)

    if datos["cantidad"]:
        pantalla += ", " + datos["cantidad"]
    if datos["caducidad"]:
        pantalla += f". Caduca {datos['caducidad']}"
    return pantalla, texto_para_voz(pantalla)


# ----------------------------------------------------------------------------
# Voz (un solo hilo, cola de mensajes)
# ----------------------------------------------------------------------------
class Voz:
    """Cola de voz en un solo hilo.

    En Windows habla con System.Speech (PowerShell) en un proceso aparte, que es
    estable al repetirse. pyttsx3 queda como respaldo (y para otros sistemas).
    """

    def __init__(self):
        """Crea la cola de mensajes, el contador de pendientes y arranca el hilo
        que habla.
        """
        self.cola = queue.Queue()
        self._pendientes = 0
        self._lock = threading.Lock()
        self._usar_powershell = sys.platform == "win32"
        threading.Thread(target=self._trabajar, daemon=True).start()

    def decir(self, texto):
        """Agrega una frase a la cola para que se hable (no bloquea)."""
        with self._lock:
            self._pendientes += 1
        self.cola.put(texto)

    def ocupada(self):
        """True mientras haya frases pendientes o hablandose; se usa para bloquear
        Espacio y evitar que se solapen resultados.
        """
        with self._lock:
            return self._pendientes > 0

    def _hablar_powershell(self, texto):
        """Habla con System.Speech de Windows en un proceso aparte (estable al
        repetirse). Devuelve True si funciono.
        """
        script = (
            "Add-Type -AssemblyName System.Speech;"
            "$s = New-Object System.Speech.Synthesis.SpeechSynthesizer;"
            "$v = $s.GetInstalledVoices() | Where-Object "
            "{ $_.Enabled -and $_.VoiceInfo.Culture.Name -like 'es*' } "
            "| Select-Object -First 1;"
            "if ($v) { $s.SelectVoice($v.VoiceInfo.Name) };"
            "$s.Rate = 0;"
            "$s.Speak($env:VISIOCARE_TXT)"
        )
        entorno = dict(os.environ, VISIOCARE_TXT=texto)
        resultado = subprocess.run(
            ["powershell", "-NoProfile", "-NonInteractive", "-Command", script],
            env=entorno,
            capture_output=True,
            timeout=90,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
        if resultado.returncode != 0:
            err = resultado.stderr.decode("utf-8", errors="ignore").strip()
            print(f"PowerShell no pudo hablar: {err}")
            return False
        return True

    def _hablar_pyttsx3(self, texto):
        """Respaldo con pyttsx3: crea un motor nuevo por frase porque reutilizarlo
        en un hilo deja de sonar.
        """
        motor = pyttsx3.init()
        motor.setProperty("rate", 150)
        pistas = ("spanish", "español", "espanol", "es-", "es_", "mexico",
                  "sabina", "helena", "pablo", "paulina")
        for voz in motor.getProperty("voices"):
            ident = normalizar(f"{voz.id} {voz.name}")
            if any(p in ident for p in pistas):
                motor.setProperty("voice", voz.id)
                break
        motor.say(texto)
        motor.runAndWait()
        motor.stop()
        del motor

    def _trabajar(self):
        """Bucle del hilo de voz: toma frases de la cola y las habla una por una,
        probando PowerShell primero y pyttsx3 si falla.
        """
        while True:
            texto = self.cola.get()
            try:
                hablado = False
                if self._usar_powershell:
                    try:
                        hablado = self._hablar_powershell(texto)
                    except Exception as exc:
                        print(f"Error de voz (PowerShell): {exc}")
                    if not hablado:
                        self._usar_powershell = False
                if not hablado:
                    self._hablar_pyttsx3(texto)
            except Exception as exc:
                print(f"Error de voz: {exc}")
            finally:
                with self._lock:
                    self._pendientes -= 1


# ----------------------------------------------------------------------------
# Dibujo de texto con soporte de acentos y ñ (PIL)
# ----------------------------------------------------------------------------
_FUENTES = {}
_RUTAS_FUENTE = (
    "C:/Windows/Fonts/arial.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "arial.ttf",
    "DejaVuSans.ttf",
)


def obtener_fuente(tam):
    """Devuelve (con cache) una fuente TrueType que soporte acentos y ñ; si no
    encuentra ninguna usa la fuente por defecto de PIL.
    """
    if tam in _FUENTES:
        return _FUENTES[tam]
    fuente = None
    for ruta in _RUTAS_FUENTE:
        try:
            fuente = ImageFont.truetype(ruta, tam)
            break
        except OSError:
            continue
    if fuente is None:
        fuente = ImageFont.load_default()
    _FUENTES[tam] = fuente
    return fuente


def dibujar_textos(frame, items):
    """Dibuja varios textos sobre el frame usando PIL (para que se vean ñ y
    acentos, que cv2.putText no soporta). 'items' es una lista de
    (texto, (x, y), color_bgr, tamano).
    """
    img = Image.fromarray(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
    draw = ImageDraw.Draw(img)
    for texto, pos, color_bgr, tam in items:
        rgb = (color_bgr[2], color_bgr[1], color_bgr[0])
        draw.text(
            pos, texto, font=obtener_fuente(tam), fill=rgb,
            stroke_width=2, stroke_fill=(0, 0, 0),
        )
    frame[:] = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)


# ----------------------------------------------------------------------------
# Analisis (hilo aparte)
# ----------------------------------------------------------------------------
def analizar_captura(modelo, clases, lector, capturas, estado, voz):
    """Trabajo pesado en un hilo aparte (para no congelar la camara):
    1) clasifica las capturas, 2) hace OCR de cada una, 3) vota los datos,
    4) arma el mensaje, 5) lo guarda en 'estado' y lo manda a la voz.
    Al terminar siempre libera la bandera 'analizando'.
    """
    try:
        clase, confianza = clasificar_capturas(modelo, clases, capturas)

        # Un alimento seguro no necesita OCR (responde mas rapido)
        es_alimento_seguro = clase == "Alimento" and confianza >= CONF_CLASE_MIN
        leer = not es_alimento_seguro

        resultados = []
        if leer:
            for i, roi in enumerate(capturas, start=1):
                try:
                    detecciones = leer_texto(lector, roi)
                except Exception as exc:
                    print(f"Error de OCR: {exc}")
                    detecciones = []
                # Depuracion: lo que realmente lee el OCR en cada captura
                print(f"[OCR {i}] " + " | ".join(t for t, _ in detecciones))
                resultados.append(analizar_texto(detecciones))
        else:
            resultados.append(analizar_texto([]))

        datos, seguros = {}, {}
        for campo in resultados[0]:
            datos[campo], seguros[campo] = votar(resultados, campo)

        pantalla, hablado = construir_mensajes(clase, confianza, datos, seguros)
        if datos["nombre_med"]:
            clase = "Medicamento"

        print(pantalla)
        with estado["lock"]:
            estado["clase"] = clase
            estado["confianza"] = confianza
            estado["resultado"] = pantalla
        voz.decir(hablado)
    except Exception as exc:
        print(f"Error al analizar la captura: {exc}")
        with estado["lock"]:
            estado["resultado"] = "Error al analizar. Intenta de nuevo."
        voz.decir("Error al analizar. Intente de nuevo.")
    finally:
        with estado["lock"]:
            estado["analizando"] = False


# ----------------------------------------------------------------------------
# Camara
# ----------------------------------------------------------------------------
def abrir_camara():
    """Prueba los indices de camara (o los de la variable VISIOCARE_CAM) y
    devuelve el primero que abre. Usa DirectShow en Windows.
    """
    env = os.environ.get("VISIOCARE_CAM")
    indices = [int(x) for x in env.split(",")] if env else [2, 3, 4, 0, 1]
    for i in indices:
        if sys.platform == "win32":
            cap = cv2.VideoCapture(i, cv2.CAP_DSHOW)
        else:
            cap = cv2.VideoCapture(i)
        if cap.isOpened():
            return cap
        cap.release()
    raise RuntimeError(
        "No se encontro ninguna camara. Enciendan la camara de OBS "
        "o define VISIOCARE_CAM=indice."
    )


# ----------------------------------------------------------------------------
# Programa principal
# ----------------------------------------------------------------------------
def main():
    """Programa principal: carga modelo, OCR, voz y camara; muestra el video con
    el recuadro; al pulsar Espacio toma N capturas y lanza el analisis en un
    hilo; 'q' sale.
    """
    modelo, clases = cargar_modelo(MODEL_PATH)
    print(f"Modelo cargado en CPU. Clases: {clases}")

    try:
        lector = iniciar_ocr()
    except Exception as exc:
        raise RuntimeError(
            "EasyOCR no pudo iniciar en modo offline. "
            "Ejecuta una vez con internet para cachear los modelos."
        ) from exc

    cap = abrir_camara()
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
        "resultado": "",
    }
    voz = Voz()

    # Control de la toma de capturas (se hacen varias, espaciadas en el tiempo)
    capturando = False
    capturas = []
    t_ultima = 0.0

    try:
        while True:
            ret, frame = cap.read()
            if not ret or frame is None:
                print("No se pudo leer un frame de la camara.")
                break

            # Lee el estado compartido con el hilo de analisis (protegido con lock)
            with estado["lock"]:
                analizando = estado["analizando"]
                clase_actual = estado["clase"]
                confianza_actual = estado["confianza"]
                resultado = estado["resultado"]

            # Si el usuario pulso Espacio, guarda un recorte del recuadro cada
            # INTERVALO_CAPTURAS segundos; al juntar N_CAPTURAS lanza el analisis
            # en un hilo para que el video no se congele.
            if capturando and time.time() - t_ultima >= INTERVALO_CAPTURAS:
                alto, ancho = frame.shape[:2]
                cx1, cy1 = max(0, x1), max(0, y1)
                cx2, cy2 = min(ancho, x2), min(alto, y2)
                capturas.append(frame[cy1:cy2, cx1:cx2].copy())
                t_ultima = time.time()
                if len(capturas) >= N_CAPTURAS:
                    capturando = False
                    threading.Thread(
                        target=analizar_captura,
                        args=(modelo, clases, lector, list(capturas), estado, voz),
                        daemon=True,
                    ).start()
                    capturas = []

            # Mientras algo este en curso (capturar, analizar o hablar) se ignora Espacio
            ocupado = capturando or analizando or voz.ocupada()

            # Texto de estado que se muestra arriba en pantalla
            if capturando:
                estado_txt = "Capturando, mantenga quieto el producto..."
            elif analizando:
                estado_txt = "Analizando..."
            elif voz.ocupada():
                estado_txt = "Hablando..."
            else:
                estado_txt = "Espacio: analizar  |  q: salir"

            # Oscurece la franja superior para que el texto se lea sobre cualquier fondo
            banda = min(170, frame.shape[0])
            frame[:banda] = (frame[:banda] * 0.45).astype(np.uint8)

            # Dibuja el recuadro guia, el estado, la clase y el resultado
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 3)
            items = [
                ("Coloque el producto aquí", (x1 + 8, y2 - 34), (0, 255, 0), 24),
                (estado_txt, (20, 12), (0, 255, 255), 22),
            ]
            if clase_actual and clase_actual != "Alimento" and not ocupado:
                items.append(
                    (f"Clase: {clase_actual} ({confianza_actual:.2f})",
                     (20, 46), (0, 255, 255), 22)
                )
            if resultado and not capturando and not analizando:
                for i, linea in enumerate(textwrap.wrap(resultado, 48)[:3]):
                    items.append((linea, (20, 80 + i * 28), (255, 255, 255), 22))
            dibujar_textos(frame, items)

            cv2.imshow("Recomocedor de productos cotidianos", frame)
            tecla = cv2.waitKey(1) & 0xFF

            if tecla == ord("q"):
                break

            # Espacio (codigo 32): inicia una nueva lectura y avisa por voz
            if tecla == 32 and not ocupado:
                with estado["lock"]:
                    estado["analizando"] = True
                    estado["resultado"] = ""
                    estado["clase"] = ""
                capturas = []
                capturando = True
                t_ultima = 0.0
                voz.decir("Analizando")
    finally:
        cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()