#include <Arduino.h>
#include "sensors.h"
#include "motors.h"
float lastError = 0;

void turn_pid()
{
	// Fast PID after turn to align bot quickly on line
	float Kp = 1.0;
	float Kd = 40.0;
	static float prevError = 0;
	int baseSpeed = 150;

	for (int i = 0; i < 30; i++)
	{
		uint16_t position = readSensors();
		int error = position - 3500;

		float motorSpeed = Kp * error + Kd * (error - prevError);
		prevError = error;

		int rightMotorSpeed = baseSpeed - motorSpeed;
		int leftMotorSpeed = baseSpeed + motorSpeed;

		if (rightMotorSpeed > MAX_SPEED)
			rightMotorSpeed = MAX_SPEED;
		if (leftMotorSpeed > MAX_SPEED)
			leftMotorSpeed = MAX_SPEED;
		if (rightMotorSpeed < 0)
			rightMotorSpeed = 0;
		if (leftMotorSpeed < 0)
			leftMotorSpeed = 0;

		left_motor.drive(leftMotorSpeed);
		right_motor.drive(rightMotorSpeed);

		delay(1);
	}
}
// Backward moving PID for U-turn alignment
void backward_alignment_pid()
{
	float Kp = 0.5;
	float Kd = 80.0;
	static float lastError = 0;
	int baseSpeed = 80; // Slow backward speed

	for (int i = 0; i < 60; i++)
	{
		uint16_t position = readSensors();
		int error = position - 3500;

		float motorSpeed = Kp * error + Kd * (error - lastError);
		lastError = error;

		// Reversed for backward movement
		int rightMotorSpeed = baseSpeed + motorSpeed;
		int leftMotorSpeed = baseSpeed - motorSpeed;

		if (rightMotorSpeed > 120)
			rightMotorSpeed = 120;
		if (leftMotorSpeed > 120)
			leftMotorSpeed = 120;
		if (rightMotorSpeed < 0)
			rightMotorSpeed = 0;
		if (leftMotorSpeed < 0)
			leftMotorSpeed = 0;

		// Negative for backward movement
		left_motor.drive(-leftMotorSpeed);
		right_motor.drive(-rightMotorSpeed);

		delay(1);
	}
}

// Follow this for PID Setup: https://www.instructables.com/Line-Follower-Robot-PID-Control-Android-Setup/
void follow_segment()
{
	// Tuned PID values from testing - provides good tracking with minimal vibration
	// Previously working 0.08 kd = 4.0
	float Kp = 0.18;
	float Kd = 0.5;
	int MAX_PID_SPEED = 180;
	int baseSpeed = 150;

	// Initialize lastError on first call to prevent derivative spike
	static bool firstCall = true;
	if (firstCall)
	{
		uint16_t position = readSensors();
		lastError = position - 3500;
		firstCall = false;
	}

	while (true)
	{
		uint16_t position = readSensors();
		int error = position - 3500;

		float derivative = Kd * (error - lastError);

		// Limit derivative to prevent spikes
		if (derivative > 20)
			derivative = 20;
		if (derivative < -20)
			derivative = -20;

		float motorSpeed = Kp * error + derivative;
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
			delay(100);
			firstCall = true;
			return;
		}
	}
}
