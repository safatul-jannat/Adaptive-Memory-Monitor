import pandas as pd
import random
from datetime import datetime, timedelta


strategies = [
    "fixed1",
    "fixed5",
    "fixed10",
    "adaptive"
]


workloads = [
    "normal",
    "memory_intensive",
    "sustained_growth",
    "bursty"
]


for strategy in strategies:

    for workload in workloads:

        rows=[]

        time=datetime.now()

        rss=100000

        for i in range(60):

            if workload=="normal":
                rss += random.randint(-1000,1000)

            elif workload=="memory_intensive":
                rss += random.randint(-5000,5000)

            elif workload=="sustained_growth":
                rss += 10000

            elif workload=="bursty":
                if i%10==0:
                    rss += 50000
                else:
                    rss -= 20000


            risk=min(100,max(0,int((rss-100000)/5000)))

            if risk <30:
                level="Normal"
            elif risk <60:
                level="Suspicious"
            elif risk <80:
                level="High"
            else:
                level="Critical"


            rows.append({

                "timestamp":time,
                "pid":1234,
                "process_name":workload,
                "rss_kb":rss,
                "vmsize_kb":rss+50000,
                "growth_rate":10,
                "sustained_growth":risk,
                "retention_score":risk,
                "historical_deviation":risk,
                "risk_score":risk,
                "risk_level":level,
                "sampling_interval":1,
                "strategy":strategy

            })


            time += timedelta(seconds=1)



        df=pd.DataFrame(rows)

        filename=f"../results/raw/{strategy}_{workload}.csv"

        df.to_csv(filename,index=False)

        print("created",filename)
