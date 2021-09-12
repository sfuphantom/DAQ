import signal

SystemState = {
    'ACTIVE': 'ACTIVE',
    'PAUSED': 'PAUSED',
    'SHUTDOWN': 'SHUTDOWN'
}

class ProcessManager:
    """
    Process Manager: Handles termination signals
    ...
    Attributes
    ----------
    controller : Object
        Controller object

    Methods
    -------
    _exit_gracefully()
      Set system state to shutdown
    """
    def __init__(self, controller):
        self.controller = controller

        # Connect callback to SIGINT and SIGTERM
        signal.signal(signal.SIGINT, self._exit_gracefully)
        signal.signal(signal.SIGTERM, self._exit_gracefully)

    def _exit_gracefully(self, *args):
        """Set system state to shutdown"""
        self.controller.state = SystemState['SHUTDOWN']
