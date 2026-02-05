// /*
// #include <Arduino.h>
// #include "sensors.h"

// void setup()
// {
// 	Serial.begin(9600);
// 	qtrInit();
// }

// void loop()
// {
// 	Serial.print("Sensor Reading: ");
// 	Serial.println(readSensor());

// 	if (Serial.available())
// 	{
// 		if (Serial.read() == 'c')
// 		{
// 			qtrCalibrate();
// 		}
// 	}
// }
// */

// #include <Arduino.h>
// #include "sensors.h"

// void setup()
// {
// 	Serial.begin(9600);
// 	initSensors();
// }

// void loop()
// {
// 	Serial.print("Sensor Reading: ");
// 	Serial.println(readSensor());
// 	delay(100);
// }

// #include <Arduino.h>
// #include <QTRSensors.h>

// QTRSensors qtr;

// const uint8_t SensorCount = 8;
// uint16_t sensorValues[SensorCount];

// void setup()
// {
// 	// configure the sensors
// 	qtr.setTypeRC();
// 	qtr.setSensorPins((const uint8_t[]){3, 4, 5, 6, 7, 8, 9, 10}, SensorCount);
// 	qtr.setEmitterPin(2);

// 	delay(500);
// 	pinMode(LED_BUILTIN, OUTPUT);
// 	digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode

// 	// 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
// 	// = ~25 ms per calibrate() call.
// 	// Call calibrate() 400 times to make calibration take about 10 seconds.
// 	for (uint16_t i = 0; i < 400; i++)
// 	{
// 		qtr.calibrate();
// 	}
// 	digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indicate we are through with calibration

// 	// print the calibration minimum values measured when emitters were on
// 	Serial.begin(9600);
// 	for (uint8_t i = 0; i < SensorCount; i++)
// 	{
// 		Serial.print(qtr.calibrationOn.minimum[i]);
// 		Serial.print(' ');
// 	}
// 	Serial.println();

// 	// print the calibration maximum values measured when emitters were on
// 	for (uint8_t i = 0; i < SensorCount; i++)
// 	{
// 		Serial.print(qtr.calibrationOn.maximum[i]);
// 		Serial.print(' ');
// 	}
// 	Serial.println();
// 	Serial.println();
// 	delay(1000);
// }

// void loop()
// {
// 	// read calibrated sensor values and obtain a measure of the line position
// 	// from 0 to 5000 (for a white line, use readLineWhite() instead)
// 	uint16_t position = qtr.readLineBlack(sensorValues);

// 	// print the sensor values as numbers from 0 to 1000, where 0 means maximum
// 	// reflectance and 1000 means minimum reflectance, followed by the line
// 	// position
// 	for (uint8_t i = 0; i < SensorCount; i++)
// 	{
// 		Serial.print(sensorValues[i]);
// 		Serial.print('\t');
// 	}
// 	Serial.println(position);

// 	delay(250);
// }

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