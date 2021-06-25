import json
import logging
from threading import Lock, Thread
from time import sleep
from datetime import datetime

# API Libraries
import gps


class gps_handler(object):
    def __init__(self, controller):
        self._controller = controller
        self._gpsd = gps.gps(mode=gps.WATCH_ENABLE)
        self._gps_poll_thread = Thread(target=self.gps_poll)
        self._gps_thread = Thread(target=self.gps_thread)
        self._gps_poll_thread.start()
        self._gps_thread.start()

    def gps_poll(self):
        while True:
            next(self._gpsd)

    def gps_thread():
        while True:
            dt = datetime.utcnow()
            gps_data = {
                "lat": gpsd.fix.latitude,
                "lon": gpsd.fix.longitude,
                "altHAE": gpsd.fix.altHAE,
                "speed": gpsd.fix.speed,
            }
            self._controller.data_queue.put((dt, "gps", gps_data))
            sleep(1)
