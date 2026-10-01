import sklearn
from sklearn.datasets import load_iris
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

iris = load_iris(as_frame=True)
df=iris.frame.copy()

df["species"]=df["target"].map(dict(enumerate(iris.target_names)))
df.columns = ["sepal_length","sepal_width","petal_length","petal_width","target","species"]

fig, axes = plt.subplots(nrows=2, ncols=2, figsize=(10,10))
axes = axes.flatten()
sns.scatterplot(data=df,x="sepal_length",y="sepal_width",ax=axes[0],hue="species",s=70)
axes[0].set_title(f"Longitud del sepalo vs ancho del sepalo")
sns.scatterplot(data=df,x="sepal_length",y="petal_length",ax=axes[1],hue="species",s=70)
axes[1].set_title(f"Longitud del sepalo vs longitud del petalo")
sns.scatterplot(data=df,x="sepal_length",y="petal_width",ax=axes[2],hue="species",s=70)
axes[2].set_title(f"Longitud del sepalo vs ancho del petalo")
sns.scatterplot(data=df,x="sepal_width",y="petal_length",ax=axes[3],hue="species",s=70)
axes[3].set_title(f"Ancho del sepalo vs longitud del petalo")
plt.tight_layout()
plt.savefig(f"fullscatter.png",dpi=150)
plt.close()