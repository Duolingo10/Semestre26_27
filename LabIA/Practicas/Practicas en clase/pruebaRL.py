import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import load_breast_cancer
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score

#1. Cargar dataset
data=load_breast_cancer()
X=data.data
y=data.target

#3. Dividir en entrenamiento y prueba
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.25, random_state=42
)

#3. Entrenar modelo 
modelo=LogisticRegression(max_iter=5000)
modelo.fit(X_train, y_train)

#4. Predicciones
y_pred=modelo.predict(X_test)

#5 Exactitud
accuracy=accuracy_score(y_test, y_pred)
print("Exactitud:", round(accuracy*100, 2),"%")

#6. Contar aciertos y errores
aciertos=np.sum(y_test == y_pred)
errores=np.sum(y_test != y_pred)

#7. Graficar resultados
labels = ['Aciertos', 'Errores']
valores = [aciertos, errores]
plt.figure(figsize=(6, 4))
plt.pie(
    valores,
    labels=labels,
    colors=['green', 'red'],
    autopct='%1.1f%%',
    startangle=90,
)
plt.title('Resultados de la clasificación - Breast Cancer')
plt.axis('equal')

#guardar grafica
plt.savefig('resultados_clasificacion.png')

# Mostrar grafica
plt.show()