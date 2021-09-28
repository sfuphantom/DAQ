from __future__ import annotations

from typing import Any
from abc import ABC, abstractmethod
from datetime import datetime
from bisect import bisect_left

from mqtt_handler import MqttHandler
from utils.utils import round_json


MQTT_PRECISION = 3


def lookup(voltage: float, volt_lookup: list[float], unit_lookup: list[float]) -> float:
    # Out of range cases
    if voltage <= volt_lookup[0]:
        return unit_lookup[0]
    if voltage >= volt_lookup[-1]:
        return unit_lookup[-1]

    i = bisect_left(volt_lookup, voltage)
    # Linear interpolation
    k = (voltage - volt_lookup[i - 1]) / (volt_lookup[i] - volt_lookup[i - 1])
    return unit_lookup[i - 1] + ((unit_lookup[i] - unit_lookup[i - 1]) * k)


class DataProcessor(ABC):
    def __init__(
        self,
        sensor_name: str,
        mqtt_handler: MqttHandler,
        mqtt_topic: str,
        database_enable: bool = False,
        telemetry_enable: bool = False,
    ) -> None:
        self.sensor_name = sensor_name
        self.mqtt = mqtt_handler
        self.mqtt_topic = mqtt_topic
        self.database_enable = database_enable
        self.telemetry_enable = telemetry_enable

    def process(self, dt: datetime, data: Any) -> None:
        print(f"{self.sensor_name} {dt} {data}")
        if self.database_enable:
            self.database_insert(dt, data)
        if self.telemetry_enable:
            self.send_telemetry(dt, data)

    @abstractmethod
    def convert(self, value: Any) -> Any:
        return value

    @abstractmethod
    def database_insert(self, dt: datetime, data: dict) -> None:
        pass

    def send_telemetry(self, dt: datetime, data: dict) -> None:
        self.mqtt_handler.client.publish(
            self.mqtt_topic,
            payload=round_json(data, MQTT_PRECISION),
            qos=2,
            retain=False
        )


class GpsProcessor(DataProcessor):
    def __init__(
        self,
        mqtt_handler: MqttHandler,
        mqtt_topic: str,
    ) -> None:
        super().__init__(
            sensor_name="GPS",
            mqtt_handler=mqtt_handler,
            mqtt_topic=mqtt_topic,
        )

    def convert(self, value: dict[str, float | int]) -> dict[str, float | int]:
        return super().convert(value)

    def database_insert(self, dt: datetime, data: dict) -> None:
        pass

    def send_telemetry(self, dt: datetime, data: dict) -> None:
        return super().send_telemetry(dt, data)


class TireTemperatureProcessor(DataProcessor):
    TEMP_CORRECTION_FACTOR: float = 0.98
    TEMP_VOLT_LOOKUP: list[float] = [-1.35, -1.11, -0.86, -0.59, -0.31, 0.00, 0.32, 0.67, 1.03, 1.41, 1.81, 2.24, 2.68, 3.14, 3.62, 4.13, 4.66, 5.21, 5.78, 6.38, 7.00]
    TEMP_LOOKUP: list[float] = [0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100]

    def __init__(
        self,
        mqtt_handler: MqttHandler,
        mqtt_topic: str,
    ) -> None:
        super().__init__(
            sensor_name="TIRE_TEMPERATURE",
            mqtt_handler=mqtt_handler,
            mqtt_topic=mqtt_topic,
        )

    def process(self, dt: datetime, data: Any) -> None:
        values = list(map(self.convert, data))
        super().process(dt, values)

    def convert(self, value: float) -> float:
        v_ref = value - 1.01
        v_amp = v_ref / 101
        v_mv = v_amp * 1000
        v_corr = v_mv / self.TEMP_CORRECTION_FACTOR
        tcf = 1 + (30 - 25) * (-0.0045)
        v_offs = 0.32 * tcf
        ref = v_corr + v_offs
        tc = ref / tcf
        return lookup(tc, self.TEMP_VOLT_LOOKUP, self.TEMP_LOOKUP)

    def database_insert(self, dt: datetime, data: dict):
        pass

    def send_telemetry(self, dt: datetime, data: dict):
        return super().send_telemetry(dt, data)


class ShockTravelProcessor(DataProcessor):
    SHOCK_VOLT_LOOKUP = [0.003, 0.075, 0.345, 0.840, 1.413, 1.944, 2.505, 3.048, 3.618, 4.224, 4.827, 5.115, 5.241]
    SHOCK_LOOKUP = [0, 0.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9.5, 10]

    def __init__(
        self,
        mqtt_handler: MqttHandler,
        mqtt_topic: str,
    ) -> None:
        super().__init__(
            sensor_name="SHOCK_TRAVEL",
            mqtt_handler=mqtt_handler,
            mqtt_topic=mqtt_topic,
        )

    def process(self, dt: datetime, data: Any) -> None:
        values = list(map(self.convert, data))
        super().process(dt, values)

    def convert(self, data: dict):
        return lookup(data, self.SHOCK_VOLT_LOOKUP, self.SHOCK_LOOKUP)

    def database_insert(self, dt: datetime, data: dict):
        pass

    def send_telemetry(self, dt: datetime, data: dict):
        return super().send_telemetry(dt, data)
