import os
import sys
import pandas as pd
import matplotlib.pyplot as plt


ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

csv_path = os.path.join(ROOT, "memory_log.csv")

if not os.path.exists(csv_path):
    sys.exit("CSV not found: " + csv_path)

df = pd.read_csv(csv_path)

for column in ("timestamp", "pid", "name", "rss_kb"):
    if column not in df.columns:
        sys.exit("CSV is missing column: " + column)

# Use the sustained-growth workload run with the most samples.
work = df[df["name"] == "sustgrowth"]

if work.empty:
    sys.exit("No sustained-growth workload rows in " + csv_path)

pid = work["pid"].value_counts().idxmax()
data = work[work["pid"] == pid].sort_values("timestamp")

time = pd.to_datetime(data["timestamp"], unit="s")

plt.plot(time, data["rss_kb"])

plt.xlabel("Time")
plt.ylabel("RSS (KB)")
plt.title("Memory Growth")

out_dir = os.path.join(ROOT, "results", "graphs")
os.makedirs(out_dir, exist_ok=True)

out_path = os.path.join(out_dir, "rss_growth.png")

plt.savefig(out_path)

print("Graph created")