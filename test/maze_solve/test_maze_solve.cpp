#include <Arduino.h>
#include <EEPROM.h>
#include "sensors.h"
#include "motors.h"
#include "pid.h"

// Left hand algorithm for fallback exploration
char lHRAlgorithm(bool found_left, bool found_straight, bool found_right, bool found_uturn)
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

void maze_solve_test()
{
	Serial.println("Starting maze solve test with fixed path...");

	// Fixed test path - later will read from EEPROM
	const char *testPath = "RSSLLSRRRRLLRSL";
	int pathLength = strlen(testPath);

	Serial.print("Path length: ");
	Serial.println(pathLength);
	Serial.print("Path: ");
	Serial.println(testPath);
	Serial.println();

	int pathIndex = 0;
	bool followingPath = true; // Track if following simplified path or exploring

	while (true)
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
		forward(left_motor, right_motor, 180);
		delay(20);
		brake(left_motor, right_motor);
		delay(100);

		readSensors();
		if (isLine(2) || isLine(3) || isLine(4) || isLine(5))
		{
			found_straight = true;
		}

		// Check for finish line (all sensors on line)
		readSensors();
		if (isLine(0) && isLine(1) && isLine(2) && isLine(3) && isLine(4) && isLine(5) && isLine(6) && isLine(7))
		{
			Serial.println("Finish line detected! Maze solved successfully!");
			digitalWrite(LED_BUILTIN, LOW);
			brake(left_motor, right_motor);

			// Celebrate with LED blinks
			for (int i = 0; i < 10; i++)
			{
				digitalWrite(LED_BUILTIN, HIGH);
				delay(100);
				digitalWrite(LED_BUILTIN, LOW);
				delay(100);
			}
			return;
		}

		char direction;

		// If following path and haven't completed it yet
		if (followingPath && pathIndex < pathLength)
		{
			char expectedTurn = testPath[pathIndex];

			Serial.print("Step ");
			Serial.print(pathIndex + 1);
			Serial.print("/");
			Serial.print(pathLength);
			Serial.print(" - Expected: ");
			Serial.print(expectedTurn);
			Serial.print(" | Available: L=");
			Serial.print(found_left);
			Serial.print(" S=");
			Serial.print(found_straight);
			Serial.print(" R=");
			Serial.println(found_right);

			// Verify the expected turn is available
			bool turnAvailable = false;

			if (expectedTurn == 'L' && found_left)
				turnAvailable = true;
			else if (expectedTurn == 'S' && found_straight)
				turnAvailable = true;
			else if (expectedTurn == 'R' && found_right)
				turnAvailable = true;
			else if (expectedTurn == 'B')
				turnAvailable = true;

			if (!turnAvailable)
			{
				// Path mismatch - switch to exploration mode
				Serial.println("WARNING: Path mismatch! Switching to exploration mode (LH algorithm)");
				followingPath = false;

				// Use left hand algorithm for this intersection
				bool found_uturn = !found_left && !found_straight && !found_right;
				direction = lHRAlgorithm(found_left, found_straight, found_right, found_uturn);
				Serial.print("Exploration turn: ");
				Serial.println(direction);
			}
			else
			{
				direction = expectedTurn;
				pathIndex++;
			}
		}
		else
		{
			// In exploration mode or completed path
			if (pathIndex >= pathLength)
			{
				Serial.println("Path complete, continuing exploration...");
			}

			bool found_uturn = !found_left && !found_straight && !found_right;
			direction = lHRAlgorithm(found_left, found_straight, found_right, found_uturn);

			Serial.print("Exploration mode - Turn: ");
			Serial.print(direction);
			Serial.print(" | L=");
			Serial.print(found_left);
			Serial.print(" S=");
			Serial.print(found_straight);
			Serial.print(" R=");
			Serial.println(found_right);
		}

		// Execute the turn
		if (direction != 'S' && direction != 'N')
		{
			makeTurn(direction);
		}
		else if (direction == 'N')
		{
			Serial.println("No valid direction!");
		}

		// Turn off LED after handling intersection
		digitalWrite(LED_BUILTIN, LOW);
	}
}

void setup()
{
	qtrInit();
	Serial.begin(9600);

	Serial.println("Maze Solve Test");
	Serial.println("Press button 12 to calibrate...");

	// Wait for calibration button
	while (digitalRead(CAL_BUTTON) == HIGH)
	{
	}
	digitalWrite(LED_BUILTIN, HIGH);
	delay(500);

	Serial.println("Calibrating sensors...");
	qtrCalibrate();
	digitalWrite(LED_BUILTIN, LOW);

	Serial.println("Calibration complete!");
	Serial.println("Press button 12 to start maze solve test...");

	// Wait for start button
	while (digitalRead(CAL_BUTTON) == HIGH)
	{
	}
	delay(50);
	while (digitalRead(CAL_BUTTON) == LOW)
	{
		delay(10);
	}

	delay(500);
	Serial.println("Starting maze solve test...");
	maze_solve_test();
}

void loop()
{
	// Nothing to do
}
