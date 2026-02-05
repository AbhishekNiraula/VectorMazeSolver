#include <Arduino.h>
#include "sensors.h"
#include "motors.h"
float lastError = 0;

// For now it's unnecessary. For the current knowledge. Maybe it will be necessary later.
void turn_pid()
{
	// PID to align robot to line center after turn with minimal linear movement
	// Quick rotational push only, minimal linear drift
	float Kp = 1;
	float Kd = 40;
	int i = 0;
	float prevError = 0;

	// Run for ~50ms - quick alignment push
	while (i < 40)
	{
		uint16_t position = readSensors();
		int error = position - 3500;

		float motorAdjustment = Kp * error + Kd * (error - prevError);
		prevError = error;

		int turnBaseSpeed = 150;

		int rightMotorSpeed = turnBaseSpeed - motorAdjustment;
		int leftMotorSpeed = turnBaseSpeed + motorAdjustment;

		if (rightMotorSpeed > MAX_SPEED)
			rightMotorSpeed = MAX_SPEED;
		if (leftMotorSpeed > MAX_SPEED)
			leftMotorSpeed = MAX_SPEED;
		if (rightMotorSpeed < -MAX_SPEED)
			rightMotorSpeed = -MAX_SPEED;
		if (leftMotorSpeed < -MAX_SPEED)
			leftMotorSpeed = -MAX_SPEED;

		left_motor.drive(leftMotorSpeed);
		right_motor.drive(rightMotorSpeed);

		i++;
		delay(5);
	}
	left_motor.brake();
	right_motor.brake();
}

void uturn_pid()
{
	int Kp = 0.5; // High P for sharp application
	int Kd = 80;
	int i = 0;
	float prevError = 0;

	while (i < 50)
	{
		int position = readSensors();
		int error = 3500 - position;

		float motorAdjustment = Kp * error + Kd * (error - prevError);
		prevError = error;

		// To reduce the linear movement. We just want rotation jerk.
		int turnBaseSpeed = 40;

		int rightMotorSpeed = turnBaseSpeed + motorAdjustment;
		int leftMotorSpeed = turnBaseSpeed - motorAdjustment;

		if (rightMotorSpeed > MAX_SPEED)
			rightMotorSpeed = MAX_SPEED;
		if (leftMotorSpeed > MAX_SPEED)
			leftMotorSpeed = MAX_SPEED;
		if (rightMotorSpeed < 0)
			rightMotorSpeed = 0;
		if (leftMotorSpeed < 0)
			leftMotorSpeed = 0;

		left_motor.drive(-leftMotorSpeed);
		right_motor.drive(-rightMotorSpeed);

		i++;
		delay(1);
	}
	left_motor.brake();
	right_motor.brake();
}

// Follow this for PID Setup: https://www.instructables.com/Line-Follower-Robot-PID-Control-Android-Setup/
void follow_segment()
{
	// THIS PID IS WORKING GREAT Kd = 4.0 Was great.
	float Kp = 0.09;
	float Kd = 3.0;
	int MAX_PID_SPEED = 200;
	int baseSpeed = 170;
	while (true)
	{

		uint16_t position = readSensors();
		int error = position - 3500;

		float motorSpeed = Kp * error + Kd * (error - lastError);
		lastError = error;

		int rightMotorSpeed = baseSpeed - motorSpeed;
		int leftMotorSpeed = baseSpeed + motorSpeed;

		if (rightMotorSpeed > MAX_PID_SPEED)
			rightMotorSpeed = MAX_PID_SPEED;
		if (leftMotorSpeed > MAX_PID_SPEED)
			leftMotorSpeed = MAX_PID_SPEED;
		if (rightMotorSpeed < -MAX_PID_SPEED)
			rightMotorSpeed = -MAX_PID_SPEED;
		if (leftMotorSpeed < -MAX_PID_SPEED)
			leftMotorSpeed = -MAX_PID_SPEED;

		left_motor.drive(leftMotorSpeed);
		right_motor.drive(rightMotorSpeed);

		if (found_intersection())
		{
			brake(left_motor, right_motor);
			return;
		}
	}
}