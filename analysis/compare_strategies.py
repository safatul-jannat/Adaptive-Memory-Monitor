import pandas as pd
import os

from metrics import *


DATA_FOLDER="../results/raw"


results=[]


for file in os.listdir(DATA_FOLDER):

    if file.endswith(".csv"):


        path=os.path.join(
            DATA_FOLDER,
            file
        )


        df=pd.read_csv(path)



        result={

            "file":file,

            "samples":
            total_samples(df),


            "average_risk":
            average_risk(df),


            "detection_latency":
            detection_latency(df)

        }


        results.append(result)



output=pd.DataFrame(results)



print(output)



output.to_csv(
"../results/tables/comparison.csv",
index=False
)
