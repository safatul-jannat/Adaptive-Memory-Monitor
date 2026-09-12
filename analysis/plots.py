import pandas as pd
import matplotlib.pyplot as plt


df=pd.read_csv(
"../results/raw/adaptive_sustained_growth.csv"
)


plt.plot(
df["rss_kb"]
)


plt.xlabel("Sample")
plt.ylabel("RSS KB")

plt.title(
"Memory Growth"
)


plt.savefig(
"../results/graphs/rss_growth.png"
)


print("Graph created")
