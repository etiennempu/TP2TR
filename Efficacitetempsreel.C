#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_SENSORS 3 //nombre de capteurs

float calculateEfficiency(float sensorReadings[], float lastSensorReadings[]) {
    float num1 = 0;
    float num2 = 0;
    int i;

    for (i = 0; i < NUM_SENSORS; i++) {
        num1 += sensorReadings[i] / lastSensorReadings[i];
        num2 += pow(sensorReadings[i] / lastSensorReadings[i], 2) - sensorReadings[i] / lastSensorReadings[i] + 1;
        lastSensorReadings[i] = sensorReadings[i];
    }

    float Ei = 1.0/3.0 * (num1 + 2 * num2) / NUM_SENSORS;

    return Ei;
}

int main() {
    float sensorReadings[NUM_SENSORS];
    float lastSensorReadings[NUM_SENSORS] = {0.0, 0.0, 0.0};
    float efficiency;
    int i;

    //simulation des lectures de capteurs
    for (i = 0; i < 10; i++) {
        sensorReadings[0] = 50.0 + i * 0.1;
        sensorReadings[1] = 50.0 + i * 0.2;
        sensorReadings[2] = 50.0 + i * 0.3;

        efficiency = calculateEfficiency(sensorReadings, lastSensorReadings);

        printf("Efficacite : %f\n", efficiency);
    }

    return 0;
}

//Ei = 1/3 * (num1 + 2 * num2) / NUM_SENSORS
