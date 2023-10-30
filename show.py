import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import sys

print(sys.argv)

def plot_kmeans(x, y, cluster, k, title):
	plt.figure()
	sns.scatterplot(x=x, 
		        y=y,
		        hue=cluster,
		        palette=sns.color_palette("hls", n_colors=k))
	plt.title(title)

df = pd.read_csv(sys.argv[1])
print(df)
for col in df.columns:
    print(f"'{col}'")

num_clusters = len(df.columns) - 2

x_col = df["x"]
y_col = df["y"]

for cluster_col in range(num_clusters):
    cluster_name = df.columns[cluster_col+2]
    cluster = df[cluster_name]
    k = len(cluster.unique())
    plot_kmeans(x_col, y_col, cluster, k, cluster_name)
    plt.savefig(f"{cluster_name}.png")

