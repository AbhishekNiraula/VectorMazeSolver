#include <Arduino.h>
#include <EEPROM.h>
#include "sensors.h"
#include "motors.h"
#include "pid.h"

void dry_run();
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

	while (digitalRead(buttonPin) == 1)
	{
	}
	// simplify_path();
	// digitalWrite(LED_BUILTIN, HIGH);
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

		// Check for finish line: all sensors detect black
		if (isLine(0) && isLine(1) && isLine(2) && isLine(3) && isLine(4) && isLine(5) && isLine(6) && isLine(7))
		{
			// Move forward to confirm it's the finish line
			forward(left_motor, right_motor, 180);
			delay(125);
			brake(left_motor, right_motor);
			delay(50);
			readSensors();

			// Confirm finish: check if center and edge sensors still detect line
			if (isLine(3) && isLine(4) && (isLine(0) || isLine(7)))
			{
				// Maze solved
				digitalWrite(LED_BUILTIN, LOW);
				return;
			}
		}

		// Drive forward to align wheels with intersection
		forward(left_motor, right_motor, 185);
		for (int i = 0; i < 10; i++)
		{
			delay(12);
			readSensors();
			if (isLine(0))
				found_left = true;
			if (isLine(7))
				found_right = true;
		}
		brake(left_motor, right_motor);
		delay(100);

		readSensors();
		// If any of the central sensors detect a line, a straight path exists
		if (isLine(2) || isLine(3) || isLine(4) || isLine(5))
		{
			found_straight = true;
		}

		// Check for dead end (no line detected)
		bool found_uturn = true;
		for (int i = 0; i < 8; i++)
		{
			if (isLine(i))
			{
				found_uturn = false;
				break;
			}
		}

		// Determine direction using left hand algorithm
		char direction = rHAlgorithm(found_right, found_straight, found_left, found_uturn);
		if (direction != 'N')
		{
			Serial.print("Direction: ");
			Serial.println(direction);

			// Record straight decisions to EEPROM (turns are recorded in their respective functions)
			if (direction == 'S')
			{
				extern int eepromAddress;
				EEPROM.write(eepromAddress++, 'S');
			}
			else
			{
				// Move forward to align motors properly for the turn
				// Use tested approach: brake to eliminate inertia, then controlled forward movement
				brake(left_motor, right_motor);
				delay(200);
				forward(left_motor, right_motor, 120);
				delay(150);
				brake(left_motor, right_motor);
				delay(100);

				makeTurn(direction);
			}
		}

		// Clear LED after intersection handling cycle
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

void simplify_path()
{
}