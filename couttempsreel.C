#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#define CHANNEL 0
#define CLOCK_SPEED 1000000

#define SENSOR_PIN_1 0
#define SENSOR_PIN_2 1
#define SENSOR_PIN_3 2
#define LED_PIN_1 3
#define LED_PIN_2 4
#define LED_PIN_3 5

#define NO_ALARM ""
#define LOW_ALARM "L"
#define MED_ALARM "M"
#define HIGH_ALARM "H"

#define LOW_ALARM_THRESHOLD_1 6
#define MED_ALARM_THRESHOLD_1 21
#define HIGH_ALARM_THRESHOLD_1 50
#define LOW_ALARM_THRESHOLD_2 2
#define MED_ALARM_THRESHOLD_2 8
#define HIGH_ALARM_THRESHOLD_2 12
#define LOW_ALARM_THRESHOLD_3 50
#define MED_ALARM_THRESHOLD_3 100
#define HIGH_ALARM_THRESHOLD_3 150

typedef enum {
    NO_ALARM_STATE,
    LOW_ALARM_STATE,
    MED_ALARM_STATE,
    HIGH_ALARM_STATE
} alarm_state;

void initialize() {
    wiringPiSPISetup(CHANNEL, CLOCK_SPEED);
    pinMode(SENSOR_PIN_1, INPUT);
    pinMode(SENSOR_PIN_2, INPUT);
    pinMode(SENSOR_PIN_3, INPUT);
    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
    pinMode(LED_PIN_3, OUTPUT);
}

uint16_t read_adc(int sensor_pin) {
    uint8_t buffer[2];
    buffer[0] = 0b00000001; // Start bit
    buffer[1] = 0b10000000 | (sensor_pin << 4); // Single-ended, specified channel
    wiringPiSPIDataRW(CHANNEL, buffer, 2);
    uint16_t result = ((buffer[0] & 0b00000011) << 8) | buffer[1];
    return result;
}

double convert_to_ppm(uint16_t value) {
    double voltage = value * (3.3 / 1023.0);
    double ppm = voltage * 1000;
    return ppm;
}

alarm_state get_alarm_state(double ppm, int gas_type) {
    if (gas_type == 1) {
        if (ppm >= HIGH_ALARM_THRESHOLD_1) {
            return HIGH_ALARM_STATE;
        } else if (ppm >= MED_ALARM_THRESHOLD_1) {
            return MED_ALARM_STATE;
        } else if (ppm >= LOW_ALARM_THRESHOLD_1) {
            return LOW_ALARM_STATE;
        } else {
            return NO_ALARM_STATE;
        }
    } else if (gas_type == 2) {
        if (ppm >= HIGH_ALARM_THRESHOLD_2) {
            return HIGH_ALARM_STATE;
        } else if (ppm >= MED_ALARM_THRESHOLD_2) {
            return MED_ALARM_STATE;
        } else if (ppm >= LOW_ALARM_THRESHOLD_2) {
            return LOW_ALARM_STATE;
        } else {
            return NO_ALARM_STATE;
        }
    } else if (gas_type == 3) {
        if (ppm >= HIGH_ALARM_THRESHOLD_3) {
            return HIGH_ALARM_STATE;
        } else if (ppm >= MED_ALARM_THRESHOLD_3) {
            return MED_ALARM_STATE;
        } else if (ppm >= LOW_ALARM_THRESHOLD_3) {
            return LOW_ALARM_STATE;
        } else {
            return NO_ALARM_STATE;
        }
    } else {
        return NO_ALARM_STATE;
    }
}

void *read_gas_values
