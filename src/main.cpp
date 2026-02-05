#include <Arduino.h>
#include <string.h>
#include "sensors.h"
#include "motors.h"
#include "pid.h"

void dry_run();
char lHAlgorithm(int found_left, int found_right, int found_straight);

void setup()
{
	qtrInit();
	Serial.begin(9600);
}

void loop()
{
	// uint16_t position = readSensors();
	// Serial.println(position);
	// Wait until button is pressed to start
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

	while (digitalRead(buttonPin) == 1)
	{
	}
	dry_run();
}
void dry_run()
{
	while (1)
	{
		follow_segment();

		// If intersection found LED is HIGH.
		digitalWrite(LED_BUILTIN, HIGH);

		// Does not seem necessary
		// Move forward a bit (very slowly) to align the sensor to the line exactly.
		// forward(left_motor, right_motor, 50);
		// delay(30);
		// brake(left_motor, right_motor);

		// Variables to store all possible intersections
		bool found_left = false;
		bool found_right = false;
		bool found_straight = false;

		// Read all Sensors to deduce left/right turns
		readSensors();

		if (isLine(0))
			found_left = true;
		if (isLine(7))
			found_right = true;

		// Drive forward to align wheels with intersection (approx 3-5cm)
		// Speed 150 ensures torque, 120ms prevents overshoot
		forward(left_motor, right_motor, 150);
		for (int i = 0; i < 15; i++)
		{
			delay(15);
			readSensors();
			if (isLine(0))
				found_left = true;
			if (isLine(7))
				found_right = true;
		}
		brake(left_motor, right_motor);
		delay(50);
		// Read sensors again after moving forward
		readSensors();
		if (isLine(2) || isLine(3) || isLine(4) || isLine(5))
		{
			found_straight = true;
		}

		// If any of the central sensors detect a line, a straight path exists

		// WAIT a possible failure of algorithm. If there is a intersection immediately after a turn then this might fail.
		if (isLine(0) && isLine(1) && isLine(2) && isLine(3) && isLine(4) && isLine(5) && isLine(6) && isLine(7))
		{
			forward(left_motor, right_motor, 180);
			delay(70);
			brake(left_motor, right_motor);
			readSensors();

			// If it's still all black then maze solved
			if (isLine(0) && isLine(7) && isLine(3))
			{
				return;
			}
		}

		// For now only left hand algorithm written.
		char direction = lHAlgorithm(found_left, found_right, found_straight);
		makeTurn(direction);
		Serial.print("Direction:");
		Serial.println(direction);
	}
}

char lHAlgorithm(int found_left, int found_right, int found_straight)
{
	if (found_left)
		return 'L';
	else if (found_straight)
		return 'S';
	else if (found_right)
		return 'R';
	else
		return 'B';
}