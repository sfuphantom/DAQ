import os
import sys
import shutil
import pandas as pd

# Usage: python preprocessing.py /Volumes/DAQ_SD
SD_ROOT = sys.argv[1] if len(sys.argv) > 1 else None

try:
    from daq_local_config import DRIVE_ROOT
except Exception:
    DRIVE_ROOT = None

PROCESSED_DIR = os.path.join(DRIVE_ROOT, "processed") if DRIVE_ROOT else None

WHEEL_SPEED_COLS = ["speed_fl_kmh", "speed_fr_kmh", "speed_rl_kmh", "speed_rr_kmh"]
TEMP_COLS = ["temp1_c", "temp2_c"]
FLOW_COLS = ["flow1_lpm", "flow2_lpm"]
SUSP_COLS = ["susp1", "susp2", "susp3", "susp4"]
SD_SUSP_COLS = ["susp1_v", "susp2_v", "susp3_v", "susp4_v"]
SD_SUSP_ALIASES = dict(zip(SD_SUSP_COLS, SUSP_COLS))
SENSOR_TIMESTAMP_COLS = ["critical_timestamp_ms", "chassis_timestamp_ms", "wheel_speed_timestamp_ms"]
TIMESTAMP_COLS = ["timestamp_ms"] + SENSOR_TIMESTAMP_COLS

# Outlier thresholds
MAX_WHEEL_DELTA_KMH = 40.0  # per sample
MAX_TEMP_SLOPE_C_PER_S = 8.0
MAX_FLOW_SLOPE_LPM_PER_S = 20.0
SUSP_MIN = 0.0
SUSP_MAX = 4095.0

# sampling interval 
def _median_dt_ms(df):
    _ensure_timestamp_ms(df)
    if "timestamp_ms" not in df.columns or len(df) < 2:
        return 1000
    diffs = df["timestamp_ms"].diff().dropna()
    diffs = diffs[diffs > 0]
    if diffs.empty:
        return 1000
    return int(round(diffs.median()))

# creatting uniform timeline and re-indexing df 
def _reindex_uniform(df, step_ms):
    _ensure_timestamp_ms(df)
    start_ms = int(df["timestamp_ms"].min())
    end_ms = int(df["timestamp_ms"].max())
    full_index = range(start_ms, end_ms + 1, step_ms)
    df = df.set_index("timestamp_ms").reindex(full_index)
    df.index.name = "timestamp_ms"
    return df.reset_index()


def _ensure_timestamp_ms(df):
    for col in TIMESTAMP_COLS:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")

    if "timestamp_ms" in df.columns:
        return

    timestamp_cols = [col for col in SENSOR_TIMESTAMP_COLS if col in df.columns]
    if timestamp_cols:
        df["timestamp_ms"] = df[timestamp_cols].max(axis=1, skipna=True)


def _normalize_sd_schema(df):
    for sd_col, canonical_col in SD_SUSP_ALIASES.items():
        if sd_col not in df.columns:
            continue

        if canonical_col not in df.columns:
            df.rename(columns={sd_col: canonical_col}, inplace=True)
        else:
            df[canonical_col] = df[canonical_col].fillna(df[sd_col])
            df.drop(columns=[sd_col], inplace=True)


def _fill_short_gaps(series, max_gap_samples, method):
    if method == "interpolate":
        return series.interpolate(method="linear", limit=max_gap_samples, limit_direction="both")
    if method == "ffill":
        return series.ffill(limit=max_gap_samples)
    return series


def _remove_wheel_outliers(series):
    s = series.copy()
    delta = s.diff().abs()
    s[(s < 0) | (delta > MAX_WHEEL_DELTA_KMH)] = pd.NA
    return s


def _remove_slope_outliers(series, dt_ms, max_slope_per_s):
    s = series.copy()
    dt_s = max(dt_ms, 1) / 1000.0
    slope = s.diff().abs() / dt_s
    s[slope > max_slope_per_s] = pd.NA
    return s


def _remove_susp_outliers(series):
    s = series.copy()
    s[(s < SUSP_MIN) | (s > SUSP_MAX)] = pd.NA
    return s


def process_log(filepath):
    df = pd.read_csv(filepath)
    print("Reading log file ", filepath)
    _normalize_sd_schema(df)

    # type checking the num values 
    for col in TIMESTAMP_COLS + WHEEL_SPEED_COLS + TEMP_COLS + FLOW_COLS + SUSP_COLS:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")

    # creating uniform timeline 
    dt_ms = _median_dt_ms(df)
    df = _reindex_uniform(df, dt_ms)

    # Gap policy (short vs long depends on sample rate via dt_ms)
    # - Wheel speeds: interpolate if missing <= 2 samples
    # - Temps: forward-fill if missing <= 5 samples
    # - Flow rates: interpolate if missing <= 3 samples
    wheel_gap = 2
    temp_gap = 5
    flow_gap = 3
    susp_gap = 5

    for col in WHEEL_SPEED_COLS:
        if col in df.columns:
            df[col] = _remove_wheel_outliers(df[col])
            df[col] = _fill_short_gaps(df[col], wheel_gap, "interpolate")

    for col in TEMP_COLS:
        if col in df.columns:
            df[col] = _remove_slope_outliers(df[col], dt_ms, MAX_TEMP_SLOPE_C_PER_S)
            df[col] = _fill_short_gaps(df[col], temp_gap, "ffill")

    for col in FLOW_COLS:
        if col in df.columns:
            df[col] = _remove_slope_outliers(df[col], dt_ms, MAX_FLOW_SLOPE_LPM_PER_S)
            df[col] = _fill_short_gaps(df[col], flow_gap, "interpolate")

    for col in SUSP_COLS:
        if col in df.columns:
            df[col] = _remove_susp_outliers(df[col])
            df[col] = _fill_short_gaps(df[col], susp_gap, "interpolate")

    return df


def run_pipeline():
    if SD_ROOT is None:
        raise SystemExit("Usage: python preprocessing.py /path/to/SD")
    if DRIVE_ROOT is None:
        raise SystemExit("Missing DRIVE_ROOT. Set it in daq_local_config.py")

    unprocessed_dir = os.path.join(SD_ROOT, "unprocessed")
    os.makedirs(PROCESSED_DIR, exist_ok=True)
    raw_dir = os.path.join(PROCESSED_DIR, "raw")
    os.makedirs(raw_dir, exist_ok=True)
    if not os.path.isdir(unprocessed_dir):
        raise SystemExit(f"Missing unprocessed dir: {unprocessed_dir}")

    files = [f for f in os.listdir(unprocessed_dir) if f.endswith(".csv")]

    for file in files:
        full_path = os.path.join(unprocessed_dir, file)

        print(f"Processing {file}")
        df_processed = process_log(full_path)

        output_path = os.path.join(PROCESSED_DIR, file)
        df_processed.to_csv(output_path, index=False)

        shutil.move(full_path, os.path.join(raw_dir, file))

    print("No more unprocessed logs.")
    print("Processing complete.")


if __name__ == "__main__":
    run_pipeline()
