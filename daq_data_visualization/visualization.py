import os
import sys
import json
import pandas as pd

# Usage: python visualization.py
# python visualization.py --simulate
try:
    from daq_local_config import DRIVE_ROOT
except Exception:
    DRIVE_ROOT = None

PROCESSED_DIR = os.path.join(DRIVE_ROOT, "processed") if DRIVE_ROOT else None
PLOTS_DIR = os.path.join(PROCESSED_DIR, "plots") if PROCESSED_DIR else None
SUMMARIES_DIR = os.path.join(PROCESSED_DIR, "summaries") if PROCESSED_DIR else None

# Thresholds for event markers (adjust as needed)
MIN_FLOW_LPM = 0.0
MAX_FLOW_LPM = 30.0
MAX_TEMP_1 = 80.0
MAX_TEMP_2 = 75.0
SLIP_THRESHOLD = 5.0  # km/h slip speed

SIM_TEST_DIR = os.path.join(os.path.dirname(__file__), "test_data")

from viz_utils import (
    WHEEL_COLS,
    TEMP_COLS,
    FLOW_COLS,
    ensure_dirs,
    load_processed_files,
    to_numeric,
    dt_ms,
    vehicle_speed,
    missing_report,
)
from viz_plots import (
    set_style,
    plot_timeline,
    plot_wheel_speeds,
    plot_slip,
    plot_cooling,
    plot_scatter,
    plot_histograms,
    plot_correlation_heatmap,
    plot_time_intensity_heatmap,
    plot_suspension,
)


def _summary(df, dt_ms):
    v = vehicle_speed(df)
    wheel_dev_max = None
    if all(col in df.columns for col in WHEEL_COLS):
        wheel_dev_max = float(df[WHEEL_COLS].max(axis=1).sub(df[WHEEL_COLS].min(axis=1)).max(skipna=True))
    summary = {
        "duration_s": round((df["timestamp_ms"].max() - df["timestamp_ms"].min()) / 1000.0, 2),
        "dt_ms": int(dt_ms),
        "vehicle_speed_max": float(v.max(skipna=True)) if v.notna().any() else None,
        "vehicle_speed_avg": float(v.mean(skipna=True)) if v.notna().any() else None,
        "wheel_speed_max_deviation": wheel_dev_max,
        "temp1_max": float(df["temp1_c"].max(skipna=True)) if "temp1_c" in df.columns else None,
        "temp2_max": float(df["temp2_c"].max(skipna=True)) if "temp2_c" in df.columns else None,
        "delta_t_max": float((df["temp1_c"] - df["temp2_c"]).max(skipna=True)) if all(col in df.columns for col in TEMP_COLS) else None,
        "flow_min_lpm": float(df["flow1_lpm"].min(skipna=True)) if "flow1_lpm" in df.columns else None,
        "flow_max_lpm": float(df["flow1_lpm"].max(skipna=True)) if "flow1_lpm" in df.columns else None,
    }

    missing = missing_report(df, WHEEL_COLS + TEMP_COLS + FLOW_COLS, dt_ms)
    summary["missing"] = missing

    if "fault_active" in df.columns:
        faults = df["fault_active"] == 1
        summary["fault_count"] = int((faults & ~faults.shift(1).fillna(False)).sum())
        summary["fault_time_s"] = round(float(faults.sum() * dt_ms / 1000.0), 2)
    return summary


def process_file(filepath):
    df = pd.read_csv(filepath)
    to_numeric(df, WHEEL_COLS + TEMP_COLS + FLOW_COLS)
    dt_ms_value = dt_ms(df)
    t = (df["timestamp_ms"] - df["timestamp_ms"].min()) / 1000.0

    run_name = os.path.splitext(os.path.basename(filepath))[0]
    run_dir = os.path.join(PLOTS_DIR, run_name)
    os.makedirs(run_dir, exist_ok=True)

    plot_timeline(df, t, run_dir)
    plot_wheel_speeds(df, t, run_dir)
    plot_slip(df, t, run_dir, SLIP_THRESHOLD)
    plot_cooling(df, t, run_dir, MIN_FLOW_LPM, MAX_FLOW_LPM)
    plot_scatter(df, run_dir)
    plot_histograms(df, run_dir)
    plot_correlation_heatmap(df, run_dir)
    plot_time_intensity_heatmap(df, run_dir)
    plot_suspension(df, t, run_dir)

    summary = _summary(df, dt_ms_value)
    summary_path = os.path.join(SUMMARIES_DIR, f"{run_name}.json")
    with open(summary_path, "w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)


def run():
    if "--simulate" in sys.argv:
        os.makedirs(SIM_TEST_DIR, exist_ok=True)
        sim_path = os.path.join(SIM_TEST_DIR, "simulated_run.csv")
        from sim_test import generate_simulated_run
        generate_simulated_run(sim_path)
        global PROCESSED_DIR, PLOTS_DIR, SUMMARIES_DIR
        PROCESSED_DIR = SIM_TEST_DIR
        PLOTS_DIR = os.path.join(PROCESSED_DIR, "plots")
        SUMMARIES_DIR = os.path.join(PROCESSED_DIR, "summaries")
    else:
        if DRIVE_ROOT is None:
            raise SystemExit("Missing DRIVE_ROOT. Set it in daq_local_config.py")
        if not os.path.isdir(PROCESSED_DIR):
            raise SystemExit(f"Missing processed dir: {PROCESSED_DIR}")

    set_style()
    ensure_dirs(PLOTS_DIR, SUMMARIES_DIR)
    files = load_processed_files(PROCESSED_DIR)
    for f in files:
        process_file(os.path.join(PROCESSED_DIR, f))


if __name__ == "__main__":
    run()
