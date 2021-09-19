from enum import Enum, auto


class Sensor(Enum):
    GPS = auto()  # Global Positioning System
    IMU = auto()  # Inertial Measurement Unit
    TIRE_TEMPERATURE = auto()
    TIRE_TEMP = TIRE_TEMPERATURE
    SHOCK_TRAVEL = auto()
    DAMPER_TRAVEL = SHOCK_TRAVEL
    # WHEEL_SPEED = auto()
    # STEERING_ANGLE = auto()
    # BRAKE_PRESSURE = auto()
    # COOLANT_TEMPERATURE = auto()
    # COOLANT_TEMP = COOLANT_TEMPERATURE
    # COOLANT_PRESSURE = auto()
    # MOTOR_SPEED = auto()
    # APPS = auto()  # Accelerator Pedal Position Sensor
    # BSE = auto()  # Brake System Encoder
