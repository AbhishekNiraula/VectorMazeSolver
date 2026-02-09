#include <Arduino.h>
#include <SparkFun_TB6612.h>
#include <EEPROM.h>
#include "sensors.h"
#include "pid.h"
#include "motors.h"

// Pins of the N20 motors (!Encoder)
// Motor A (Right Motor)
const int motorAIn1 = 4;
const int motorAIn2 = 5;
const int motorAPwm = 3;

// Motor B (Left Motor)
const int motorBIn1 = 6;
const int motorBIn2 = 7;
const int motorBPwm = 9;

// Standby Pin
const int standbyPin = 2;

Motor right_motor = Motor(motorAIn1, motorAIn2, motorAPwm, 1, standbyPin);
Motor left_motor = Motor(motorBIn1, motorBIn2, motorBPwm, 1, standbyPin);

int MAX_SPEED = 150;
int TURN_SPEED = 250;
int UTURN_SPEED = 230;

int eepromAddress = 0;

// Deciding the turns based on the character send on the code.
void makeTurn(char c)
{
	switch (c)
	{
	case 'L':
		makeLeftTurn();
		break;
	case 'R':
		makeRightTurn();
		break;
	case 'B':
		makeUTurn();
		break;
	}
}

// Execute turn without writing to EEPROM (for maze solve)
void executeTurn(char c)
{
	switch (c)
	{
	case 'L':
		executeLeftTurn();
		break;
	case 'R':
		executeRightTurn();
		break;
	case 'B':
		executeUTurn();
		break;
	}
}

void executeLeftTurn()
{
	// Phase 1: Rotate until leftmost sensor hits black
	left(left_motor, right_motor, 380);
	while (readSensor(0) == 0)
	{
	}

	// Phase 2: Continue until sensor leaves black for alignment
	left(left_motor, right_motor, 300);
	while (readSensor(0) == 1)
	{
	}

	brake(left_motor, right_motor);
	delay(50);

	turn_pid();
	brake(left_motor, right_motor);
}

void makeLeftTurn()
{
	EEPROM.write(eepromAddress++, 'L');
	executeLeftTurn();
}

void executeRightTurn()
{
	// Phase 1: Rotate until rightmost sensor hits black
	right(left_motor, right_motor, 380);
	while (readSensor(7) == 0)
	{
	}

	// Phase 2: Continue until sensor leaves black for alignment
	right(left_motor, right_motor, 300);
	while (readSensor(7) == 1)
	{
	}

	brake(left_motor, right_motor);
	delay(50);

	// Phase 3: PID alignment to center on line
	turn_pid();
	brake(left_motor, right_motor);
}

void makeRightTurn()
{
	EEPROM.write(eepromAddress++, 'R');
	executeRightTurn();
}

void executeUTurn()
{
	// Full stop to eliminate inertia
	brake(left_motor, right_motor);
	delay(50);

	// Forward movement to clear the line and position for u-turn
	forward(left_motor, right_motor, 100);
	delay(150);
	brake(left_motor, right_motor);
	delay(50);

	// Check if sensor 7 is already on line (bot is inclined)
	readSensors();
	bool sensor7_already_on_line = (readSensor(7) == 1);

	// Phase 1: Rotate right until sensor 7 hits black (slower speed than regular turn)
	right(left_motor, right_motor, 340);

	if (sensor7_already_on_line)
	{
		// If already on line, rotate until it leaves the line first
		while (readSensor(7) == 1)
		{
		}
		// Now wait for it to hit black again
		while (readSensor(7) == 0)
		{
		}
	}
	else
	{
		// Normal case: wait until sensor 7 hits black
		while (readSensor(7) == 0)
		{
		}
	}

	// Phase 2: Continue rotation for alignment
	right(left_motor, right_motor, 240);
	while (readSensor(7) == 1)
	{
	}

	brake(left_motor, right_motor);
	delay(50);

	// Phase 3: Forward PID alignment
	turn_pid();
	brake(left_motor, right_motor);
	delay(50);

	// Phase 4: Backward PID for fine alignment
	backward_alignment_pid();
	brake(left_motor, right_motor);
	delay(50);

	// Phase 5: Final forward positioning
	forward(left_motor, right_motor, 100);
	delay(80);
	brake(left_motor, right_motor);
}

void makeUTurn()
{
	EEPROM.write(eepromAddress++, 'B');
	executeUTurn();
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