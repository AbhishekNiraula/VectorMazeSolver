#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <QTRSensors.h>

// QTR Sensor
extern QTRSensors qtr;
void qtrInit();
int readSensor(int n);
uint16_t readSensors();
void qtrCalibrate();
bool found_intersection();
bool isLine(int n);
extern uint16_t sensorValues[];
extern uint16_t threshold[];
extern const uint8_t Sensor_Count;
extern int buttonPin;

// Button pins
extern const int LH_BUTTON;
extern const int RH_BUTTON;
extern const int CAL_BUTTON;

#endif
