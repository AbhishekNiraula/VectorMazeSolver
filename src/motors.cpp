#include <Arduino.h>
#include <SparkFun_TB6612.h>
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

// TB6612 Motor Classes for left and right motors.
Motor right_motor = Motor(motorAIn1, motorAIn2, motorAPwm, 1, 1);
Motor left_motor = Motor(motorBIn1, motorBIn2, motorBPwm, 1, 1);

int MAX_SPEED = 200;
int TURN_SPEED = 320;
int UTURN_SPEED = 280;

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
	// Turn left until the leftMost sensor reads 1.
	left(left_motor, right_motor, TURN_SPEED);
	while (readSensor(0) == 0)
	{
	}

	// Same logic as that of the left. Just the sensors are inverted.
	left(left_motor, right_motor, TURN_SPEED);
	while (readSensor(0) == 1 || readSensor(4) == 0)
	{
	}
	brake(left_motor, right_motor);
}

void makeRightTurn()
{
	right(left_motor, right_motor, TURN_SPEED);
	// Until the rightmost sensor is not on the black line rotate right.
	while (readSensor(7) == 0)
	{
	}
	// Rightmost sensor is on black now it should leave black to align the bot. Also Right after it becomes white the bot stops which we don't want since the bot is still at an angle. So to mitigate that I will read the second to left sensor, if it comes to black then the bot is more or less aligned.
	right(left_motor, right_motor, TURN_SPEED);
	while (readSensor(7) == 1 || readSensor(3) == 0)
	{
	}
	brake(left_motor, right_motor);
}

void makeUTurn()
{
	right(left_motor, right_motor, UTURN_SPEED);
	while (readSensor(7) == 0)
	{
	}
	// Cut the speed of the motors to reduce the drift of the motor
	right(left_motor, right_motor, UTURN_SPEED);
	while (readSensor(7) == 1 || readSensor(3) == 0)
	{
	}
	brake(left_motor, right_motor);
}