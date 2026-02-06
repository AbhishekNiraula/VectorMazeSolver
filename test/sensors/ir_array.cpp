#include <Arduino.h>
#include "sensors.h"
void setup()
{
	qtrInit();
	Serial.begin(9600);
}
void loop()
{
	while (digitalRead(buttonPin) == 1)
	{
	}
	Serial.println("Calibration Started");
	qtrCalibrate();
	while (true)
	{
		for (int i = 0; i < Sensor_Count; i++)
		{
			Serial.print(readSensor(i));
			Serial.print(" ");
		}
		Serial.print(readSensors());
		Serial.println();
		delay(100);
	}
}