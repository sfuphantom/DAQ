import json
import logging
from threading import Lock, Thread
import time
from datetime import datetime, timezone

# API Libraries
import gps

from process_manager import SystemState

class gps_handler(object):
    def __init__(self, controller):
        self._controller = controller
        self._gpsd = gps.gps(mode=gps.WATCH_ENABLE)

        self.gps_state = SystemState['ACTIVE']

        self._gps_poll_thread = Thread(target=self.gps_poll)
        self._gps_thread = Thread(target=self.gps_thread)
        self._gps_poll_thread.start()
        self._gps_thread.start()

    def shutdown(self):
        self.gps_state = SystemState['SHUTDOWN']

    def pause(self, pause_status):
        self.gps_state = pause_status

    def gps_poll(self):
        while self.gps_state != SystemState['SHUTDOWN']:
            if self.gps_state == SystemState['PAUSED']:
                time.sleep(1)
            else:
                next(self._gpsd)

    def gps_thread(self):
        while self.gps_state != SystemState['SHUTDOWN']:
            if self.gps_state == SystemState['PAUSED']:
                time.sleep(0.1)
            else:
                dt = datetime.now(timezone.utc)
                gps_data = {
                    "lat": self._gpsd.fix.latitude,
                    "lon": self._gpsd.fix.longitude,
                    "alt": self._gpsd.fix.altitude,
                    "speed": self._gpsd.fix.speed,
                }
                self._controller.data_queue.put((dt, "gps", gps_data))
                time.sleep(1)
