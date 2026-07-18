import os
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd
from viz_utils import (
    WHEEL_COLS,
    TEMP_COLS,
    vehicle_speed,
    slip_speed,
)


def set_style():
    sns.set_theme(style="whitegrid", context="talk")


def plot_timeline(df, t, run_dir):
    fig, ax = plt.subplots(figsize=(12, 6))
    v = vehicle_speed(df)
    if "flow1_lpm" in df.columns:
        sns.lineplot(x=t, y=df["flow1_lpm"], ax=ax, label="flow1_lpm")
    if "flow2_lpm" in df.columns:
        sns.lineplot(x=t, y=df["flow2_lpm"], ax=ax, label="flow2_lpm")
    if "temp1_c" in df.columns:
        sns.lineplot(x=t, y=df["temp1_c"], ax=ax, label="temp1")
    if "temp2_c" in df.columns:
        sns.lineplot(x=t, y=df["temp2_c"], ax=ax, label="temp2")
    sns.lineplot(x=t, y=v, ax=ax, label="vehicle_speed")

    if "fault_active" in df.columns:
        fault = df["fault_active"] == 1
        ax.fill_between(t, ax.get_ylim()[0], ax.get_ylim()[1], where=fault, color="red", alpha=0.1)

    ax.set_title("Run Overview Timeline")
    ax.set_xlabel("Time (s)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(os.path.join(run_dir, "timeline.png"))
    plt.close(fig)


def plot_wheel_speeds(df, t, run_dir):
    fig, ax = plt.subplots(figsize=(12, 6))
    for col in WHEEL_COLS:
        if col in df.columns:
            sns.lineplot(x=t, y=df[col], ax=ax, label=col)
    sns.lineplot(x=t, y=vehicle_speed(df), ax=ax, label="vehicle_speed")
    ax.set_title("Wheel Speeds + Vehicle Speed")
    ax.set_xlabel("Time (s)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(os.path.join(run_dir, "wheel_speeds.png"))
    plt.close(fig)


def plot_slip(df, t, run_dir, slip_threshold):
    slip = slip_speed(df)
    if slip is None:
        return
    fig, ax = plt.subplots(figsize=(12, 4))
    sns.lineplot(x=t, y=slip, ax=ax, label="slip_speed (driven - non-driven)")
    ax.axhline(slip_threshold, color="red", linestyle="--", linewidth=1)
    ax.axhline(-slip_threshold, color="red", linestyle="--", linewidth=1)
    ax.set_title("Slip Speed")
    ax.set_xlabel("Time (s)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(os.path.join(run_dir, "slip_speed.png"))
    plt.close(fig)


def plot_cooling(df, t, run_dir, min_flow, max_flow):
    fig, ax = plt.subplots(figsize=(12, 6))
    if "flow1_lpm" in df.columns:
        sns.lineplot(x=t, y=df["flow1_lpm"], ax=ax, label="flow1_lpm")
    if "flow2_lpm" in df.columns:
        sns.lineplot(x=t, y=df["flow2_lpm"], ax=ax, label="flow2_lpm")
    ax.axhline(min_flow, color="red", linestyle="--", linewidth=1)
    ax.axhline(max_flow, color="red", linestyle="--", linewidth=1)
    ax.set_title("Cooling Flow with Thresholds")
    ax.set_xlabel("Time (s)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(os.path.join(run_dir, "cooling_flow.png"))
    plt.close(fig)

    if all(col in df.columns for col in TEMP_COLS):
        fig, ax = plt.subplots(figsize=(12, 4))
        delta_t = df["temp1_c"] - df["temp2_c"]
        sns.lineplot(x=t, y=delta_t, ax=ax, label="delta_t")
        ax.set_title("Delta T (temp1 - temp2)")
        ax.set_xlabel("Time (s)")
        ax.legend()
        fig.tight_layout()
        fig.savefig(os.path.join(run_dir, "delta_t.png"))
        plt.close(fig)


def plot_scatter(df, run_dir):
    v = vehicle_speed(df)
    if "flow1_lpm" in df.columns:
        fig, ax = plt.subplots(figsize=(5, 5))
        sns.scatterplot(x=v, y=df["flow1_lpm"], ax=ax, s=12, alpha=0.5)
        ax.set_xlabel("vehicle_speed")
        ax.set_ylabel("flow1_lpm")
        ax.set_title("Flow vs Vehicle Speed")
        fig.tight_layout()
        fig.savefig(os.path.join(run_dir, "flow_vs_speed.png"))
        plt.close(fig)

    if all(col in df.columns for col in TEMP_COLS):
        delta_t = df["temp1_c"] - df["temp2_c"]
        fig, ax = plt.subplots(figsize=(5, 5))
        sns.scatterplot(x=v, y=delta_t, ax=ax, s=12, alpha=0.5)
        ax.set_xlabel("vehicle_speed")
        ax.set_ylabel("delta_t")
        ax.set_title("Delta T vs Vehicle Speed")
        fig.tight_layout()
        fig.savefig(os.path.join(run_dir, "delta_t_vs_speed.png"))
        plt.close(fig)

    if "flow1_lpm" in df.columns and "temp1_c" in df.columns:
        fig, ax = plt.subplots(figsize=(5, 5))
        sns.scatterplot(x=df["temp1_c"], y=df["flow1_lpm"], ax=ax, s=12, alpha=0.5)
        ax.set_xlabel("temp1_c")
        ax.set_ylabel("flow1_lpm")
        ax.set_title("Flow vs Temp1")
        fig.tight_layout()
        fig.savefig(os.path.join(run_dir, "flow_vs_temp.png"))
        plt.close(fig)


def plot_histograms(df, run_dir):
    if "flow1_lpm" in df.columns:
        fig, ax = plt.subplots(figsize=(6, 4))
        sns.histplot(df["flow1_lpm"].dropna(), bins=40, ax=ax)
        ax.set_title("Flow Histogram")
        fig.tight_layout()
        fig.savefig(os.path.join(run_dir, "flow_hist.png"))
        plt.close(fig)

    if all(col in df.columns for col in TEMP_COLS):
        delta_t = (df["temp1_c"] - df["temp2_c"]).dropna()
        fig, ax = plt.subplots(figsize=(6, 4))
        sns.histplot(delta_t, bins=40, ax=ax)
        ax.set_title("Delta T Histogram")
        fig.tight_layout()
        fig.savefig(os.path.join(run_dir, "delta_t_hist.png"))
        plt.close(fig)

    if all(col in df.columns for col in WHEEL_COLS):
        diffs = df[WHEEL_COLS].diff().abs().max(axis=1).dropna()
        fig, ax = plt.subplots(figsize=(6, 4))
        sns.histplot(diffs, bins=40, ax=ax)
        ax.set_title("Wheel Speed Delta Histogram")
        fig.tight_layout()
        fig.savefig(os.path.join(run_dir, "wheel_delta_hist.png"))
        plt.close(fig)

def plot_suspension(df, t, run_dir):
    cols = ["susp1", "susp2", "susp3", "susp4"]
    if not any(col in df.columns for col in cols):
        return
    fig, ax = plt.subplots(figsize=(12, 6))
    for col in cols:
        if col in df.columns:
            sns.lineplot(x=t, y=df[col], ax=ax, label=col)
    ax.set_title("Suspension Sensors")
    ax.set_xlabel("Time (s)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(os.path.join(run_dir, "suspension.png"))
    plt.close(fig)


def plot_correlation_heatmap(df, run_dir):
    cols = [
        "temp1_c",
        "temp2_c",
        "flow1_lpm",
        "flow2_lpm",
        "speed_kmh",
        "speed_fl_kmh",
        "speed_fr_kmh",
        "speed_rl_kmh",
        "speed_rr_kmh",
        "susp1",
        "susp2",
        "susp3",
        "susp4",
    ]
    cols = [c for c in cols if c in df.columns]
    if len(cols) < 2:
        return
    corr = df[cols].corr()
    fig, ax = plt.subplots(figsize=(10, 8))
    sns.heatmap(corr, ax=ax, cmap="coolwarm", center=0, annot=False, linewidths=0.5)
    ax.set_title("Correlation Heatmap")
    fig.tight_layout()
    fig.savefig(os.path.join(run_dir, "correlation_heatmap.png"))
    plt.close(fig)


def plot_time_intensity_heatmap(df, run_dir):
    cols = [
        "temp1_c",
        "temp2_c",
        "flow1_lpm",
        "flow2_lpm",
        "speed_kmh",
        "susp1",
        "susp2",
        "susp3",
        "susp4",
    ]
    cols = [c for c in cols if c in df.columns]
    if len(cols) < 2:
        return

    data = df[cols].copy()
    data = (data - data.mean()) / data.std(ddof=0)
    data = data.T

    fig, ax = plt.subplots(figsize=(12, 6))
    sns.heatmap(data, ax=ax, cmap="viridis", cbar=True)
    ax.set_ylabel("Sensor")
    ax.set_xlabel("Time Index")
    ax.set_title("Time vs Sensor Intensity (z-score)")
    fig.tight_layout()
    fig.savefig(os.path.join(run_dir, "time_sensor_heatmap.png"))
    plt.close(fig)
