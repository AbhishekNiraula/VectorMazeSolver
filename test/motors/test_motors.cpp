#include <Arduino.h>
#include "motors.h"
#include "sensors.h"
#include "pid.h"
void setup()
{
	Serial.begin(9600);
	qtrInit();

	Serial.println("Setup Complete. Waiting 2 seconds before starting loop...");
}

void loop()
{
	// // Wait until button is pressed to start
	while (digitalRead(buttonPin) == 1)
	{
	}
	// Debounce the initial press (ignore contact bounce)
	delay(50);

	// Wait for release (Button must go HIGH/1)
	while (digitalRead(buttonPin) == 0)
	{
		delay(10);
	}

	// Debounce the release (ignore contact bounce on release)
	// This prevents the "tail" of the first click from registering as a second click
	delay(100);

	// Check for a second press within a short window (e.g., 500ms)
	// We are looking for the button to go LOW (0) again.
	unsigned long startTime = millis();
	bool doubleClick = false;
	while (millis() - startTime < 500)
	{
		if (digitalRead(buttonPin) == 0)
		{
			doubleClick = true;
			break;
		}
	}

	if (doubleClick)
	{
		Serial.println("Double Click Detected: Calibrating...");
		// Debounce the second press
		delay(50);
		// Wait for button release of the second click
		while (digitalRead(buttonPin) == 0)
		{
			delay(10);
		}
		delay(1000); // Time to move hand away
		qtrCalibrate();
	}
	else
	{
		Serial.println("Single Click Detected: Skipping Calibration...");
	}

	delay(1000);

	while (true)
	{
		while (digitalRead(buttonPin) == 1)
		{
		}
		follow_segment();
		delay(1000);
	}
}