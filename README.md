# Data Acquisition

```bash
# Clone repo
git clone https://github.com/sfuphantom/DAQ.git
cd DAQ

# Install dependencies
sudo apt-get update
sudo apt-get install gpsd gpsd-clients i2c-tools
pip3 install -r requirements.txt

# Start GPS daemon
sudo systemctl stop gpsd.socket
sudo gpsd /dev/serial0 -F /var/run/gpsd.sock

# Start DAQ
python3 daq_controller.py
```
