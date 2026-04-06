import os
import pandas as pd
import numpy as np

FLOW_SENSOR_HZ_PER_LPM = 6.6
FLOW_SENSOR_MAX_SIGNAL_V = 5.0
FLOW_SENSOR_MAX_FREQUENCY_HZ = 198.0


def generate_simulated_run(path, samples=600, dt_ms=100):
    t = pd.Series(range(0, samples * dt_ms, dt_ms), name="timestamp_ms")
    speed = (t / 1000.0).clip(upper=60)
    speed = speed + 2.0 * pd.Series(np.sin(t / 1500.0))
    flow1_hz = 80.0 + 15.0 * pd.Series(np.sin(t / 2000.0))
    flow2_hz = 76.0 + 12.0 * pd.Series(np.sin(t / 2200.0))
    flow1_lpm = flow1_hz / FLOW_SENSOR_HZ_PER_LPM
    flow2_lpm = flow2_hz / FLOW_SENSOR_HZ_PER_LPM
    flow1_v = np.clip((flow1_hz / FLOW_SENSOR_MAX_FREQUENCY_HZ) * FLOW_SENSOR_MAX_SIGNAL_V, 0.0, FLOW_SENSOR_MAX_SIGNAL_V)
    flow2_v = np.clip((flow2_hz / FLOW_SENSOR_MAX_FREQUENCY_HZ) * FLOW_SENSOR_MAX_SIGNAL_V, 0.0, FLOW_SENSOR_MAX_SIGNAL_V)

    df = pd.DataFrame(
        {
            "timestamp_ms": t,
            "temp1_c": 40 + 0.02 * t / 1000.0 + 2 * np.sin(t / 4000.0),
            "flow1_hz": flow1_hz,
            "flow2_hz": flow2_hz,
            "flow1_signal_v": flow1_v,
            "flow2_signal_v": flow2_v,
            "flow1_lpm": flow1_lpm,
            "flow2_lpm": flow2_lpm,
            "susp1": 2000 + 200 * np.sin(t / 300.0),
            "susp2": 2100 + 180 * np.sin(t / 280.0),
            "susp3": 2050 + 220 * np.sin(t / 320.0),
            "susp4": 1950 + 160 * np.sin(t / 310.0),
            "speed_fl_kmh": speed + np.random.normal(0, 0.5, size=len(t)),
            "speed_fr_kmh": speed + np.random.normal(0, 0.5, size=len(t)),
            "speed_rl_kmh": speed + np.random.normal(0, 0.8, size=len(t)),
            "speed_rr_kmh": speed + np.random.normal(0, 0.8, size=len(t)),
        }
    )
    df["speed_kmh"] = df[["speed_fl_kmh", "speed_fr_kmh", "speed_rl_kmh", "speed_rr_kmh"]].mean(axis=1)
    df.to_csv(path, index=False)
