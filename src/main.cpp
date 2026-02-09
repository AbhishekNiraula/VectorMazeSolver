#include <Arduino.h>
#include <EEPROM.h>
#include "sensors.h"
#include "simplify.h"
#include "motors.h"
#include "pid.h"
#include "maze_solve.h"

bool useLH = false;
bool isCalibrated = false;

void dry_run();

void setup()
{
	// Initialize the QTR and other Sensors interfaced.
	qtrInit();
	Serial.begin(9600);

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

	if (digitalRead(LH_BUTTON) == LOW)
	{
		useLH = true;
		delay(50);
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
	while (digitalRead(CAL_BUTTON) == HIGH)
	{
	}
	delay(500);

	extern int eepromAddress;
	for (int i = 0; i < 1024; i++)
	{
		EEPROM.write(i, 0xFF);
	}
	eepromAddress = 0;
}

void loop()
{
	dry_run();

	while (digitalRead(RH_BUTTON) == HIGH && digitalRead(CAL_BUTTON) == HIGH)
	{
	}

	if (digitalRead(RH_BUTTON) == LOW)
	{
		// Redo dry run
		delay(50);
		while (digitalRead(RH_BUTTON) == LOW)
		{
			delay(10);
		}
		delay(500);
		return; // Restart loop to redo dry_run
	}

	// Dry run completed, simplify path
	simplify_path();

	// Wait for button 12 to start maze solve
	while (digitalRead(CAL_BUTTON) == HIGH)
	{
	}
	delay(50);
	while (digitalRead(CAL_BUTTON) == LOW)
	{
		delay(10);
	}

	delay(500);
	maze_solve(useLH);

	// Maze solve complete - stop
	while (1)
	{
		delay(1000);
	}
}
void dry_run()
{
	extern int eepromAddress;

	while (1)
	{
		// Press digital pin 11 for restarting dry run
		if (digitalRead(RH_BUTTON) == LOW)
		{
			delay(50);
			while (digitalRead(RH_BUTTON) == LOW)
			{
				delay(10);
			}
			left_motor.standby();
			right_motor.standby();
			delay(100);
			continue; // Restart dry run
		}

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
		// Determine direction using selected algorithm
		bool found_uturn = !isLine(0) && !isLine(1) && !isLine(2) && !isLine(3) && !isLine(4) && !isLine(5) && !isLine(6) && !isLine(7);

		char direction;
		if (useLH)
		{
			direction = lHAlgorithm(found_left, found_straight, found_right, found_uturn);
		}
		else
		{
			direction = rHAlgorithm(found_right, found_straight, found_left, found_uturn);
		}

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