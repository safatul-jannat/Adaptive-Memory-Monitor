import pandas as pd

from metrics import *


FILE="../results/raw/adaptive_sustained_growth.csv"


df=pd.read_csv(FILE)


print("====================")
print("Memory Monitor Analysis")
print("====================")


print("\nTotal Samples:")
print(
    total_samples(df)
)



print("\nAverage Risk:")
print(
    average_risk(df)
)



print("\nRisk Distribution:")
print(
    risk_distribution(df)
)



print("\nDetection Latency:")
print(
    detection_latency(df),
    "seconds"
)
