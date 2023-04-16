#include <Arduino.h>
#include <ISensorDriver.h>

uint16_t baseTemp(data, x) 
{
  // for tempurature sensors - analog reading is directly proportional to temperature in deg-C
  uint16_t temperature = data*x; //where x = proportionality constant, 
  return temperature;
};

uint16_t brakePressure(data)
{
    //for brake pressure sensor (MIPAN2XX500PSAAX) 
    uint16_t pressure = 625*(data-0.5); // in PSI, V_supply @ 25 deg-C = 5V (ratiometric) 
    return pressure;

};

uint16_t tirePressure(data)
{
    //for coolant pressure sensor (116CP31-M04S2-50)
    uint16_t pressure = 20*(data-0.5);
    return pressure;
};