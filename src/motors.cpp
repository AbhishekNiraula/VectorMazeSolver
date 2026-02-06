#include <Arduino.h>
#include <SparkFun_TB6612.h>
#include <EEPROM.h>
#include "sensors.h"
#include "pid.h"
#include "motors.h"

// Pins of the N20 motors (!Encoder)
const int motorAIn1 = 13;
const int motorAIn2 = 12;
const int motorAPwm = 11;
const int motorBIn1 = 3;
const int motorBIn2 = 4;
const int motorBPwm = 10;
const int standbyPin = 8;

// TB6612 Motor Classes for left and right motors.
// Motor(In1, In2, PWM, offset, STBYpin)
Motor right_motor = Motor(motorAIn1, motorAIn2, motorAPwm, 1, standbyPin);
Motor left_motor = Motor(motorBIn1, motorBIn2, motorBPwm, 1, standbyPin);

int MAX_SPEED = 200;
int TURN_SPEED = 320;
int UTURN_SPEED = 320;

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

void makeLeftTurn()
{
	EEPROM.write(eepromAddress++, 'L');

	// Phase 1: Rotate until leftmost sensor hits black
	left(left_motor, right_motor, 400);
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

void makeRightTurn()
{
	EEPROM.write(eepromAddress++, 'R');

	// Phase 1: Rotate until rightmost sensor hits black
	right(left_motor, right_motor, 400);
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

void makeUTurn()
{
	EEPROM.write(eepromAddress++, 'B');

	// Full stop to eliminate inertia
	brake(left_motor, right_motor);
	delay(100);

	// Small forward movement to clear the line
	forward(left_motor, right_motor, 150);
	delay(80);
	brake(left_motor, right_motor);
	delay(100);

	// Phase 1: Rotate right until sensor 7 hits black (slower speed than regular turn)
	right(left_motor, right_motor, 320);
	while (readSensor(7) == 0)
	{
	}

	// Phase 2: Continue rotation for alignment
	right(left_motor, right_motor, 240);
	while (readSensor(7) == 1)
	{
	}

	brake(left_motor, right_motor);
	delay(100);

	// Phase 3: Forward PID alignment
	turn_pid();
	brake(left_motor, right_motor);
	delay(100);

	// Phase 4: Backward PID for fine alignment
	backward_alignment_pid();
	brake(left_motor, right_motor);
	delay(100);

	// Phase 5: Final forward positioning
	forward(left_motor, right_motor, 80);
	delay(50);
	brake(left_motor, right_motor);
}