#include <Arduino.h>
#include "motors.h"
#include "sensors.h"
#include "pid.h"

// Turn alignment PID - runs after completing a turn to center the bot on the line
// Based on follow_segment1() from wrc.cpp
void turn_alignment_pid()
{
	float Kp = 1.0;
	float Kd = 40.0;
	static float lastError = 0;
	int baseSpeed = 150;

	for (int i = 0; i < 30; i++)
	{
		uint16_t position = readSensors();
		int error = position - 3500;

		float motorSpeed = Kp * error + Kd * (error - lastError);
		lastError = error;

		int rightMotorSpeed = baseSpeed - motorSpeed;
		int leftMotorSpeed = baseSpeed + motorSpeed;

		// Limit motor speeds
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

// Make left turn with alignment - based on wrc.cpp turn() function
void make_left_turn()
{
	Serial.println("Making left turn...");

	// Phase 1: Rotate until leftmost sensor hits black
	// Speed 400 = ±200 per motor (fast but sensors can still read)
	left(left_motor, right_motor, 400);
	while (readSensor(0) == 0)
	{
	}

	// Phase 2: Continue until sensor leaves black for alignment
	// Speed 300 = ±150 per motor (controlled alignment with sensor feedback)
	left(left_motor, right_motor, 300);
	while (readSensor(0) == 1)
	{
	}

	brake(left_motor, right_motor);
	delay(50);

	// Phase 3: PID alignment to center on line
	turn_alignment_pid();
	brake(left_motor, right_motor);

	Serial.print("Turn complete! Position: ");
	Serial.println(readSensors());
}

// Test sequence following dry_run pattern from main.cpp
void test_sequence()
{
	while (1)
	{
		// Follow line segment until intersection detected
		follow_segment();
		brake(left_motor, right_motor);
		delay(100);

		digitalWrite(LED_BUILTIN, HIGH);

		// Check for left turn
		bool found_left = false;

		readSensors();
		Serial.print("Initial check - Sensor 0: ");
		Serial.print(readSensor(0));
		Serial.print(", Sensor 7: ");
		Serial.println(readSensor(7));

		if (isLine(0))
			found_left = true;

		// Drive forward to align with intersection
		forward(left_motor, right_motor, 120);
		for (int i = 0; i < 12; i++)
		{
			delay(12);
			readSensors();
			if (isLine(0))
				found_left = true;
		}
		brake(left_motor, right_motor);
		delay(200);

		readSensors();
		Serial.print("After forward - Sensor 0: ");
		Serial.print(readSensor(0));
		Serial.print(", found_left: ");
		Serial.println(found_left);

		// Check for end condition (all sensors black)
		readSensors();
		if (isLine(0) && isLine(1) && isLine(2) && isLine(3) &&
			isLine(4) && isLine(5) && isLine(6) && isLine(7))
		{
			Serial.println("End detected!");
			break;
		}

		// If left turn detected, make the turn and exit test
		if (found_left)
		{
			Serial.println("Left turn detected!");

			// Move forward 5-6cm to align motors properly for the turn
			forward(left_motor, right_motor, 150);
			delay(400); // ~5-6cm at speed 150
			brake(left_motor, right_motor);
			delay(100);

			make_left_turn();

			// Exit after single left turn
			Serial.println("Single left turn complete. Test finished!");
			break;
		}
		else
		{
			Serial.println("No left turn, continuing straight");
		}

		digitalWrite(LED_BUILTIN, LOW);
	}
}

void setup()
{
	Serial.begin(9600);
	qtrInit();

	Serial.println("=== LEFT TURN TEST ===");
	Serial.println("Press button to calibrate...");

	while (digitalRead(buttonPin) == 1)
	{
	}
	delay(100);
	while (digitalRead(buttonPin) == 0)
	{
	}

	Serial.println("Calibrating...");
	delay(1000);

	// Sweeping calibration
	qtrCalibrate();

	brake(left_motor, right_motor);
	Serial.println("Calibration complete!");
	Serial.println("Press button to start test...");

	while (digitalRead(buttonPin) == 1)
	{
	}
	delay(100);
	while (digitalRead(buttonPin) == 0)
	{
	}
	delay(500);
}

void loop()
{
	Serial.println("Press button to start test sequence...");

	while (digitalRead(buttonPin) == 1)
	{
	}
	delay(100);
	while (digitalRead(buttonPin) == 0)
	{
	}
	delay(500);

	// Run the test sequence (follows segment and detects/makes left turns)
	test_sequence();

	// Halt after test complete
	brake(left_motor, right_motor);
	Serial.println("Test finished. Reset to run again.");
	while (1)
	{
	}
}
