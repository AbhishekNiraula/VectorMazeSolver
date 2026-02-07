#include <Arduino.h>
#include <EEPROM.h>
#include "sensors.h"
#include "simplify.h"
#include "motors.h"
#include "pid.h"

void dry_run();
void maze_solve();
char lHAlgorithm(bool found_left, bool found_straight, bool found_right, bool found_uturn);
char rHAlgorithm(bool found_right, bool found_straight, bool found_left, bool found_uturn);

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

	// while (digitalRead(buttonPin) == 1)
	// {
	// }
	// simplify_path();

	// delay(1000);
	// Serial.println("Press button to start maze solve...");

	// while (digitalRead(buttonPin) == 1)
	// {
	// }
	// maze_solve();
}
void dry_run()
{
	// Clear EEPROM at the start of each run for new path recording
	extern int eepromAddress;
	for (int i = 0; i < 1024; i++)
	{
		EEPROM.write(i, 0xFF);
	}
	// Reset address counter
	eepromAddress = 0;

	while (1)
	{
		follow_segment();
		left_motor.standby();
		right_motor.standby();
		delay(100);

		// LED indicates intersection handling
		digitalWrite(LED_BUILTIN, HIGH);
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
		// Drive forward to align wheels with intersection
		forward(left_motor, right_motor, 100);
		delay(200);
		brake(left_motor, right_motor);
		delay(100);
		readSensors();
		if (isLine(2) || isLine(3) || isLine(4) || isLine(5))
		{
			found_straight = true;
		}
		readSensors();
		if (isLine(0) && isLine(1) && isLine(2) && isLine(3) && isLine(4) && isLine(5) && isLine(6) && isLine(7))
		{
			return;
		}
		// Determine direction using right hand algorithm
		// Dead-end detection: if ALL sensors read white (no line detected anywhere), trigger u-turn
		bool found_uturn = !isLine(0) && !isLine(1) && !isLine(2) && !isLine(3) && !isLine(4) && !isLine(5) && !isLine(6) && !isLine(7);
		char direction = rHAlgorithm(found_right, found_straight, found_left, found_uturn);
		// Record the turn in EEPROM
		if (direction != 'N')
		{
			if (direction == 'S')
			{
				EEPROM.write(eepromAddress++, direction);
			}
			else
			{
				makeTurn(direction);
			}
		}
		// Turn off LED after handling intersection
		digitalWrite(LED_BUILTIN, LOW);
	}
}

char lHAlgorithm(bool found_left, bool found_straight, bool found_right, bool found_uturn)
{
	if (found_left)
		return 'L';
	else if (found_straight)
		return 'S';
	else if (found_right)
		return 'R';
	else if (found_uturn)
		return 'B';
	else
		return 'N';
}

char rHAlgorithm(bool found_right, bool found_straight, bool found_left, bool found_uturn)
{
	if (found_right)
		return 'R';
	else if (found_straight)
		return 'S';
	else if (found_left)
		return 'L';
	else if (found_uturn)
		return 'B';
	else
		return 'N';
}