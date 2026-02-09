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
	float Kp = 0.3;
	float Kd = 15.0; // Reduced from 80.0 to prevent vibration
	static float lastError = 0;
	int baseSpeed = 60; // Reduced from 80 for smoother movement

	for (int i = 0; i < 60; i++)
	{
		uint16_t position = readSensors();
		int error = position - 3500;

		float motorSpeed = Kp * error + Kd * (error - lastError);
		lastError = error;

		// Reversed for backward movement
		int rightMotorSpeed = baseSpeed + motorSpeed;
		int leftMotorSpeed = baseSpeed - motorSpeed;

		if (rightMotorSpeed > 100)
			rightMotorSpeed = 100;
		if (leftMotorSpeed > 100)
			leftMotorSpeed = 100;
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
	// Tuned PID values from motor testing - smooth, stable line following
	float Kp = 0.042;
	float Kd = 0.5;
	int MAX_PID_SPEED = 150;
	int baseSpeed = 120;

	// Initialize lastError to 0 on first call
	static bool firstCall = true;
	if (firstCall)
	{
		lastError = 0;
		firstCall = false;
	}

	unsigned long segmentStartTime = millis(); // Track elapsed time to reset inertia

	while (true)
	{
		if (millis() - segmentStartTime >= 1300)
		{
			left_motor.standby();
			right_motor.standby();
			delay(80);
			segmentStartTime = millis();
		}

		uint16_t position = readSensors();
		int error = position - 3500;

		float proportional = Kp * error;
		float derivative = Kd * (error - lastError);

		// Reasonable derivative limits
		if (derivative > 50)
			derivative = 50;
		if (derivative < -50)
			derivative = -50;

		float motorSpeed = proportional + derivative;
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

			firstCall = true;
			return;
		}
	}
}
