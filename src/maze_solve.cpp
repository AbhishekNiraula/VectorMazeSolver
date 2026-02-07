#include <Arduino.h>
#include <EEPROM.h>
#include "sensors.h"
#include "motors.h"
#include "pid.h"

// Forward declaration
void dry_run();

void maze_solve()
{
	Serial.println("Starting maze solve with simplified path...");

	// Read simplified path from EEPROM
	char path[100];
	int pathLength = 0;

	for (int i = 0; i < 100; i++)
	{
		char turn = EEPROM.read(i);
		if (turn == 0xFF) // End of path marker
			break;
		path[pathLength++] = turn;
	}

	if (pathLength == 0)
	{
		Serial.println("No path found in EEPROM! Running dry_run...");
		dry_run();
		return;
	}

	Serial.print("Path length: ");
	Serial.println(pathLength);
	Serial.print("Path: ");
	for (int i = 0; i < pathLength; i++)
	{
		Serial.print(path[i]);
	}
	Serial.println();

	int pathIndex = 0;

	while (pathIndex < pathLength)
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

		// Check for finish line (all sensors on black)
		readSensors();
		if (isLine(0) && isLine(1) && isLine(2) && isLine(3) && isLine(4) && isLine(5) && isLine(6) && isLine(7))
		{
			Serial.println("Finish line detected! Maze solved successfully!");
			digitalWrite(LED_BUILTIN, LOW);
			brake(left_motor, right_motor);
			return;
		}

		// Get expected turn from path
		char expectedTurn = path[pathIndex];

		Serial.print("Expected turn: ");
		Serial.print(expectedTurn);
		Serial.print(" | Available: L=");
		Serial.print(found_left);
		Serial.print(" S=");
		Serial.print(found_straight);
		Serial.print(" R=");
		Serial.println(found_right);

		// Verify the expected turn is available at this intersection
		bool turnAvailable = false;

		if (expectedTurn == 'L' && found_left)
			turnAvailable = true;
		else if (expectedTurn == 'S' && found_straight)
			turnAvailable = true;
		else if (expectedTurn == 'R' && found_right)
			turnAvailable = true;
		else if (expectedTurn == 'B') // U-turn is always available
			turnAvailable = true;

		if (!turnAvailable)
		{
			// Path mismatch detected! The simplified path doesn't match maze reality
			Serial.println("ERROR: Expected turn not available!");
			Serial.println("Path mismatch - reverting to dry_run mode...");
			digitalWrite(LED_BUILTIN, LOW);
			delay(1000);

			// Revert to exploration mode
			dry_run();
			return;
		}

		// Execute the turn
		if (expectedTurn != 'S')
		{
			makeTurn(expectedTurn);
		}

		pathIndex++;

		// Turn off LED after handling intersection
		digitalWrite(LED_BUILTIN, LOW);
	}

	// If we completed all turns in the path, follow one more segment to finish
	Serial.println("All turns completed, following final segment...");
	follow_segment();
	brake(left_motor, right_motor);
	Serial.println("Maze solve complete!");
}
