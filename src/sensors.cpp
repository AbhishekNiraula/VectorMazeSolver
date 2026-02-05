#include <Arduino.h>

#include <QTRSensors.h>
#include "motors.h"
#include "sensors.h"

extern const uint8_t Sensor_Count = 8;
const uint8_t sensorPinNumbers[Sensor_Count] = {A0, A1, A2, A3, A4, A5, A6, A7};

// QTR Sensor object from the QTRSensors library
QTRSensors qtr;

// Making the variables to store the sensor values.
uint16_t sensorValues[Sensor_Count];
uint16_t threshold[Sensor_Count] = {683, 715, 751, 774, 771, 709, 615, 557};

extern int buttonPin = 5;

// Initialize the QTR sensor.
void qtrInit()
{
	pinMode(buttonPin, INPUT_PULLUP);
	pinMode(LED_BUILTIN, OUTPUT);
	qtr.setTypeAnalog();
	qtr.setSensorPins(sensorPinNumbers, Sensor_Count);
	digitalWrite(2, HIGH);
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
	left_motor.drive(0);
	right_motor.drive(0);

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
		threshold[i] = (qtr.calibrationOn.minimum[i] + qtr.calibrationOn.maximum[i]) / 2;
		Serial.print(threshold[i]);
		Serial.print(" ");
	}
	Serial.println();
}
int readSensor(int n)
{
	qtr.readLineBlack(sensorValues);
	if (n >= 0 && n < Sensor_Count)
	{
		if (sensorValues[n] > threshold[n])
		{
			return 1;
		}
	}
	return 0;
}

uint16_t readSensors()
{
	return qtr.readLineBlack(sensorValues);
}

// Fast intersection detection using already-read sensor values (no re-reading!)
bool found_intersection()
{
	// 1. CHECK EXTREME SENSORS - if extreme sensors (0 or 7) see line, sharp turn needed
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
	{
		return true;
	}

	return false;
}

// Check if a specific sensor sees a line based on current buffer (does not read hardware)
bool isLine(int n)
{
	if (n >= 0 && n < Sensor_Count)
	{
		return sensorValues[n] > threshold[n];
	}
	return false;
}