import pandas as pd


# 1. Total number of samples

def total_samples(df):

    return len(df)



# 2. Average monitoring risk

def average_risk(df):

    return df["risk_score"].mean()



# 3. Risk level counting

def risk_distribution(df):

    return df["risk_level"].value_counts()



# 4. Detection latency

# Detection event:
# first time process reaches High risk

def detection_latency(df):

    high = df[df["risk_level"].isin(
        ["High", "Critical"]
    )]


    if high.empty:
        return None


    start = pd.to_datetime(
        df.iloc[0]["timestamp"]
    )


    detected = pd.to_datetime(
        high.iloc[0]["timestamp"]
    )


    latency = (
        detected - start
    ).total_seconds()


    return latency



# 5. Number of samples per second

def samples_per_second(df, duration):

    if duration == 0:
        return 0


    return len(df)/duration


def total_samples(df):

    return len(df)



def average_risk(df):

    return df["risk_score"].mean()



def risk_distribution(df):

    return df["risk_level"].value_counts()



def detection_latency(df):

    high=df[df["risk_level"]=="High"]

    if len(high)==0:
        return None

    start=df.iloc[0]["timestamp"]

    detected=high.iloc[0]["timestamp"]

    return (
        pd.to_datetime(detected)
        -
        pd.to_datetime(start)
    ).seconds
