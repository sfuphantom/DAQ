#include <stdio.h>
#include <bcm2835.h>
#include <math.h>
#include <time.h>

#define CONVERSION_REG 0X0
#define CONFIG_REG     0x1
#define LO_THRESH_REG  0x2
#define HI_THRESH_REG  0x3
#define ADC_ADDR      0x48

// Write to a register
void writeReg16(uint8_t reg, uint16_t value) {
    bcm2835_i2c_setSlaveAddress(ADC_ADDR);
    char buf[] = { reg, value >> 8, value & 0xFF };
    bcm2835_i2c_write(buf, 3);
}

// Read from a register
uint16_t readReg16(uint8_t reg) {
    char buf[1];
    bcm2835_i2c_setSlaveAddress(ADC_ADDR);
    bcm2835_i2c_write(&reg, 1);
    bcm2835_i2c_read(buf, 2);
    return buf[0] << 8 | buf[1];
}

// // Write to a register
// void writeReg(uint8_t reg, uint8_t value) {
//     bcm2835_i2c_setSlaveAddress(ADC_ADDR);
//     char buf[] = {reg, value};
//     bcm2835_i2c_write(buf, 2);
// }

// Read from a register
// uint8_t readReg(uint8_t reg) {
//     char value;
//     bcm2835_i2c_setSlaveAddress(ADC_ADDR);
//     bcm2835_i2c_write(&reg, 1);
//     bcm2835_i2c_read(&value, 1);
//     return value;
// }

int main() {
    printf("main 1\n");
    // Initialize the BCM2835 library
    // if (!bcm2835_init()) {
    //     printf("first if statement (true)\n");
    // } else {
    //     printf("first if statement (false)\n");
    //     printf("after\n");
    //     return 0;
    // }

    if (!bcm2835_init()) {
        printf("failed to initialize\n");
        return 0;
    }

    if (!bcm2835_i2c_begin()) {
        printf("I2C failed to initialize\n");
        return 0;
    }

    // // Open I2C
    // if (!bcm2835_i2c_begin()) {
    //     printf("second if statement (true)\n");
    // } else {
    //     printf("second if statement (false)\n");
    //     return 0;
    // }


    // CODE :)

    // the 16 bit for writing to chose: 1 000 010 0 100 0 0 0 11
    // 1 Operational status or single-shot conversion start: PICK THIS ONE IDK WHAT IT MEANS
    // 000 Input multiplexer configuration: 000: AINp = AIN0 and AINn = AIN1 (default)
    // 001 Programmable gain amplifier configuration FSR = +-4.096V
    // 100 Data rate: 128 SPS (default)
    // 0 Comparator mode: traditional comparator
    // 0 comparator polarity: active LOW
    // 0 non latching comparator: The ALERT/RDY pin does not latch when asserted (default)
    // 11 Comparator queue and disable: disable comparator and set ALERT/RDY pin to high impedance (default)

    uint16_t ay0 = 0b1000010010000011;
    writeReg16(CONFIG_REG, ay0);
    uint16_t config = readReg16(CONFIG_REG);

    uint16_t lower_threshold = readReg16(LO_THRESH_REG);
    uint16_t upper_threshold = readReg16(HI_THRESH_REG);

    // ads1115.setGain(GAIN_TWO);


    for (int i = 0; i < 100; i++) {
        uint16_t adc_reg = readReg16(CONVERSION_REG);
        printf("CONFIG_REG 0x%X\n", config);
        // printf("LO_THRESH_REG %d\n", lower_threshold, (int16_t)lower_threshold);
        // printf("HI_THRESH_REG %d\n", upper_threshold, (int16_t)upper_threshold);
        printf(" %d\n", (int16_t)adc_reg);

        double a = 5.0 / 65536;
        double mvolts = (adc_reg * a * 1000 - 400) / 101;
        printf(" mV: %lf\n", mvolts);

    }

    // Close I2C
    bcm2835_i2c_end();
    // Close the BCM2835 library
    bcm2835_close();
    return 1;
}