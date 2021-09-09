# Data Acquisition

Set I2C to 400 kHz
```sh
sudo nano /boot/config.txt
```

Find the line that says `dtparam=i2c_arm=on` and change it to
```txt
dtparam=i2c_arm=on,i2c_arm_baudrate=400000
```

```sh
# Restart to apply new I2C setting
sudo reboot
```

```sh
# Clone repo
git clone https://github.com/sfuphantom/DAQ.git
cd DAQ

# Install dependencies
sudo apt-get update
sudo apt-get install gpsd gpsd-clients i2c-tools libatlas-base-dev
pip3 install -r requirements.txt

# Start GPS daemon
sudo systemctl stop gpsd.socket
sudo gpsd /dev/serial0 -F /var/run/gpsd.sock

# Start DAQ
python3 daq_controller.py
```
