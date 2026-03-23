#include <Arduino.h>
#include <EEPROM.h>
#include "sensors.h"
#include "motors.h"
#include "pid.h"

void maze_solve(bool useLeftHand)
{
	// Reading the EEPROM data onto an array.
	char path[100];
	int pathLength = 0;

	for (int i = 0; i < 100; i++)
	{
		char turn = EEPROM.read(i);
		if (turn == 0xFF)
			break;
		path[pathLength++] = turn;
	}

	if (pathLength == 0)
	{
		return;
	}

	int pathIndex = 0;
	bool followingPath = true;

	while (true)
	{
		follow_segment();
		left_motor.standby();
		right_motor.standby();

		// LED indicates intersection handling
		digitalWrite(LED_BUILTIN, HIGH);
		forward(left_motor, right_motor, 50);
		delay(30);
		left_motor.standby();
		right_motor.standby();

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
		delay(250);
		brake(left_motor, right_motor);
		delay(30);

		readSensors();
		if (isLine(2) || isLine(3) || isLine(4) || isLine(5))
		{
			found_straight = true;
		}

		// Check for finish line
		readSensors();
		if (isLine(0) && isLine(1) && isLine(2) && isLine(3) && isLine(4) && isLine(5) && isLine(6) && isLine(7))
		{
			digitalWrite(LED_BUILTIN, LOW);
			brake(left_motor, right_motor);
			return;
		}

		char direction;

		// If following path and haven't completed it yet
		if (followingPath && pathIndex < pathLength)
		{
			char expectedTurn = path[pathIndex];
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
				followingPath = false;
				bool found_uturn = !found_left && !found_straight && !found_right;
				if (useLeftHand)
				{
					direction = lHAlgorithm(found_left, found_straight, found_right, found_uturn);
				}
				else
				{
					direction = rHAlgorithm(found_right, found_straight, found_left, found_uturn);
				}
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
			bool found_uturn = !found_left && !found_straight && !found_right;
			if (useLeftHand)
			{
				direction = lHAlgorithm(found_left, found_straight, found_right, found_uturn);
			}
			else
			{
				direction = rHAlgorithm(found_right, found_straight, found_left, found_uturn);
			}
		}

		// Execute the turn or record straight
		if (direction != 'N')
		{
			if (direction == 'S')
			{
				// Continue straight - no turn needed, already positioned
			}
			else
			{
				executeTurn(direction);
			}
		}

		// Turn off LED after handling intersection
		digitalWrite(LED_BUILTIN, LOW);
	}
}
