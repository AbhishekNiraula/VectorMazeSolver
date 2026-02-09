#include <Arduino.h>

#include <QTRSensors.h>
#include "motors.h"
#include "sensors.h"

const uint8_t Sensor_Count = 8;
const uint8_t sensorPinNumbers[Sensor_Count] = {A7, A6, A5, A4, A3, A2, A1, A0};

// QTR Sensor object from the QTRSensors library
QTRSensors qtr;

// Making the variables to store the sensor values.
uint16_t sensorValues[Sensor_Count];
uint16_t threshold[Sensor_Count];

// Button pins
const int LH_BUTTON = 10;
const int RH_BUTTON = 11;
const int CAL_BUTTON = 12;

// Initialize the QTR sensor.
void qtrInit()
{
	// Setup all button pins
	pinMode(10, INPUT_PULLUP); // LH algorithm button
	pinMode(11, INPUT_PULLUP); // RH algorithm button
	pinMode(12, INPUT_PULLUP); // Calibration/Solve button
	pinMode(LED_BUILTIN, OUTPUT);
	qtr.setTypeAnalog();
	qtr.setSensorPins(sensorPinNumbers, Sensor_Count);
	digitalWrite(8, HIGH);
	digitalWrite(LED_BUILTIN, LOW);
}

// Function to calibrate the QTR Sensor.
// Put the bot in the present lighting condition in such a way that sensor can read both black and white lines.
void qtrCalibrate()
{
	digitalWrite(LED_BUILTIN, HIGH);

	// THIS NEEDS FIXING JUST FOR TEST PURPOSE CALIBRATION TIME IS DECRESED.
	for (uint16_t i = 0; i < 200; i++)
	{
		if (i < 50 || i >= 150)
		{
			left(left_motor, right_motor, MAX_SPEED * 2);
		}
		else
		{
			right(left_motor, right_motor, MAX_SPEED * 2);
		}
		qtr.calibrate();
		delay(10);
	}

	// Stop motors after calibration
	left_motor.standby();
	right_motor.standby();

	// print the calibration minimum values mesured.
	Serial.print("Minimum: ");
	for (uint8_t i = 0; i < Sensor_Count; i++)
	{
		Serial.print(qtr.calibrationOn.minimum[i]);
		Serial.print(" ");
	}
	Serial.println();

	// print the calibration maximum values measured when emitters were on
	Serial.print("Maximum: ");
	for (uint8_t i = 0; i < Sensor_Count; i++)
	{
		Serial.print(qtr.calibrationOn.maximum[i]);
		Serial.print(" ");
	}
	Serial.println();

	// Calculate and print threshold values
	Serial.print("Threshold: ");
	for (uint8_t i = 0; i < Sensor_Count; i++)
	{
		if (i == 0 || i == 7)
		{
			threshold[i] = (qtr.calibrationOn.minimum[i] + qtr.calibrationOn.maximum[i]) / 2;
		}
		else
		{
			threshold[i] = (qtr.calibrationOn.minimum[i] + qtr.calibrationOn.maximum[i]) / 2;
		}
		Serial.print(threshold[i]);
		Serial.print(" ");
	}
	Serial.println();
}
int readSensor(int n)
{
	qtr.readLineBlack(sensorValues);
	return sensorValues[n] > threshold[n] ? 1 : 0;
}

uint16_t readSensors()
{
	return qtr.readLineBlack(sensorValues);
}

// Fast intersection detection with debouncing to prevent false triggers
bool found_intersection()
{
	// Count active sensors
	bool extremeOnLine = (sensorValues[0] > threshold[0] || sensorValues[7] > threshold[7]);
	bool centerOnLine = (sensorValues[3] > threshold[3] || sensorValues[4] > threshold[4]);
	if (extremeOnLine && centerOnLine)
	{
		return true;
	}

	// 2. CHECK ALL WHITE - all sensors off line (possible u-turn point)
	bool allWhite = true;
	for (int i = 0; i < Sensor_Count; i++)
	{
		if (sensorValues[i] > threshold[i])
		{
			allWhite = false;
			break;
		}
	}

	if (allWhite)
		return true;

	return false;
}

// Check if a specific sensor sees a line based on current buffer (does not read hardware)
bool isLine(int n)
{
	return sensorValues[n] > threshold[n];
}