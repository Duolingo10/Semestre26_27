import os
import numpy as np
import pandas as pd
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split

iris = load_iris()
df = pd.DataFrame(data=iris.data, columns=iris.feature_names)
df['species'] = iris.target
df['species'] = df['species'].map({0: 'setosa', 1: 'versicolor', 2: 'virginica'})

X = df.drop('species', axis=1).copy()  # Todas las columnas menos 'species'
Y = df['species'].copy()               # La columna objetivo

X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size=0.2, random_state=42)

print(f"Dimensiones del conjunto de entrenamiento: {X_train.shape}")  # Corregido
print(f"Dimensiones del conjunto de prueba: {X_test.shape}")          # Corregido

print("\nPrimeras filas del conjunto de entrenamiento:")
print(Y_train.value_counts().sort_index())

print("\nPrimeras filas del conjunto de prueba:")
print(Y_test.value_counts().sort_index())

train_distribution = Y_train.value_counts(normalize=True).sort_index()
print("\nDistribución de clases en el conjunto de entrenamiento:")
print(train_distribution)

test_distribution = Y_test.value_counts(normalize=True).sort_index()
print("\nDistribución de clases en el conjunto de prueba:")
print(test_distribution)

X_train.to_csv("X_train.csv")
X_test.to_csv("X_test.csv")
Y_train.to_csv("Y_train.csv")
Y_test.to_csv("Y_test.csv")