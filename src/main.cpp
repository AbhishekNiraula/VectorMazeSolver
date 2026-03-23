#include <Arduino.h>
#include <EEPROM.h>
#include "sensors.h"
#include "simplify.h"
#include "motors.h"
#include "pid.h"
#include "maze_solve.h"

// By default use Right Hand Algorithm
bool useLH = false;

void dry_run();

void setup()
{
	// Initialize the QTR and other Sensors interfaced.
	qtrInit();
	Serial.begin(9600);

	// Press the calibration button to begin Calibration
	while (digitalRead(CAL_BUTTON) == HIGH)
	{
	}
	digitalWrite(LED_BUILTIN, HIGH);
	delay(500);
	qtrCalibrate();

	// Wait until the algorithm selection buttons are pressed
	while (digitalRead(LH_BUTTON) == HIGH && digitalRead(RH_BUTTON) == HIGH)
	{
	}

	// Select Left/Right Hand Algorithm
	if (digitalRead(LH_BUTTON) == LOW)
	{
		useLH = true;
		delay(50);
		// Waiting for debounce
		while (digitalRead(LH_BUTTON) == LOW)
		{
			delay(10);
		}
	}
	else if (digitalRead(RH_BUTTON) == LOW)
	{
		useLH = false;
		delay(50);
		while (digitalRead(RH_BUTTON) == LOW)
		{
			delay(10);
		}
	}
	// Flash the EEPROM
	extern int eepromAddress;
	for (int i = 0; i < 1024; i++)
	{
		EEPROM.write(i, 0xFF);
	}
	eepromAddress = 0;
	// Press the Calibration button to begin the loop function
	while (digitalRead(CAL_BUTTON) == HIGH)
	{
	}
	delay(500);
}

void loop()
{
	// Begin the Dry Run
	dry_run();

	// After Dry Run Ends wait if Right Hand or Calibration Button is pressed.
	while (digitalRead(RH_BUTTON) == HIGH && digitalRead(CAL_BUTTON) == HIGH)
	{
	}

	// If Right Hand Button is pressed then again do the dry run -- if anything wrong is noticed during dry run.
	if (digitalRead(RH_BUTTON) == LOW)
	{
		delay(50);
		while (digitalRead(RH_BUTTON) == LOW)
		{
			delay(10);
		}
		delay(500);
		return;
	}

	// Simplify the path registered in EEPROM
	simplify_path();

	// Wait for Calibration button to be pressed to begin the final run.
	while (digitalRead(CAL_BUTTON) == HIGH)
	{
	}
	delay(500);
	maze_solve(useLH);

	// Stop after solving maze.
	while (1)
	{
	}
}
void dry_run()
{
	extern int eepromAddress;

	while (1)
	{
		follow_segment();
		left_motor.standby();
		right_motor.standby();

		// LED indicates intersection handling
		digitalWrite(LED_BUILTIN, HIGH);
		// Move forward very slightly to eliminate false readings.
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

		// Straight line following the left/right turn - T-Intersection or Cross Intersection.
		readSensors();
		if (isLine(2) || isLine(3) || isLine(4) || isLine(5))
		{
			found_straight = true;
		}

		// Read the end of the maze.
		readSensors();
		if (isLine(0) && isLine(1) && isLine(2) && isLine(3) && isLine(4) && isLine(5) && isLine(6) && isLine(7))
		{
			for (int i = 0; i < 10; i++)
			{
				digitalWrite(LED_BUILTIN, HIGH);
				delay(200);
				digitalWrite(LED_BUILTIN, LOW);
				delay(200);
			}
			return;
		}

		//  If no line read by any sensor then it's a U-Turn
		bool found_uturn = !isLine(0) && !isLine(1) && !isLine(2) && !isLine(3) && !isLine(4) && !isLine(5) && !isLine(6) && !isLine(7);

		// Determine direction using selected algorithm
		char direction;
		if (useLH)
		{
			direction = lHAlgorithm(found_left, found_straight, found_right, found_uturn);
		}
		else
		{
			direction = rHAlgorithm(found_right, found_straight, found_left, found_uturn);
		}

		// If it's a Straight movement then Register it in EEPROM.
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