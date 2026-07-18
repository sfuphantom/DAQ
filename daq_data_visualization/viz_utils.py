import math
import os
import pandas as pd

WHEEL_COLS = ["speed_fl_kmh", "speed_fr_kmh", "speed_rl_kmh", "speed_rr_kmh"]
TEMP_COLS = ["temp1_c", "temp2_c"]
FLOW_COLS = ["flow1_lpm", "flow2_lpm"]
TIMESTAMP_COLS = ["timestamp_ms", "critical_timestamp_ms", "chassis_timestamp_ms", "wheel_speed_timestamp_ms"]

DRIVEN_WHEELS = ["speed_rl_kmh", "speed_rr_kmh"]
NON_DRIVEN_WHEELS = ["speed_fl_kmh", "speed_fr_kmh"]


def ensure_dirs(plots_dir, summaries_dir):
    os.makedirs(plots_dir, exist_ok=True)
    os.makedirs(summaries_dir, exist_ok=True)


def load_processed_files(processed_dir):
    files = []
    for f in os.listdir(processed_dir):
        if f.endswith(".csv"):
            files.append(f)
    return sorted(set(files))


def to_numeric(df, cols):
    ensure_timestamp_ms(df)
    for col in cols:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")


def ensure_timestamp_ms(df):
    if "timestamp_ms" in df.columns:
        df["timestamp_ms"] = pd.to_numeric(df["timestamp_ms"], errors="coerce")
        return

    for col in TIMESTAMP_COLS[1:]:
        if col in df.columns:
            df["timestamp_ms"] = pd.to_numeric(df[col], errors="coerce")
            return


def dt_ms(df):
    ensure_timestamp_ms(df)
    if "timestamp_ms" not in df.columns or len(df) < 2:
        return 1000
    diffs = df["timestamp_ms"].diff().dropna()
    diffs = diffs[diffs > 0]
    if diffs.empty:
        return 1000
    return int(round(diffs.median()))


def vehicle_speed(df):
    if all(col in df.columns for col in WHEEL_COLS):
        return df[WHEEL_COLS].mean(axis=1)
    if "speed_kmh" in df.columns:
        return df["speed_kmh"]
    return pd.Series([math.nan] * len(df))


def slip_speed(df):
    if not all(col in df.columns for col in DRIVEN_WHEELS + NON_DRIVEN_WHEELS):
        return None
    driven = df[DRIVEN_WHEELS].mean(axis=1)
    non_driven = df[NON_DRIVEN_WHEELS].mean(axis=1)
    return driven - non_driven


def missing_report(df, cols, dt_ms_value):
    report = {}
    for col in cols:
        if col not in df.columns:
            continue
        s = df[col]
        missing_pct = float(s.isna().mean()) * 100.0
        longest_gap = 0
        current = 0
        for val in s.isna():
            if val:
                current += 1
                longest_gap = max(longest_gap, current)
            else:
                current = 0
        report[col] = {
            "missing_pct": round(missing_pct, 2),
            "longest_gap_ms": int(longest_gap * dt_ms_value),
        }
    return report
