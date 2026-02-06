#include <Arduino.h>
#include "motors.h"
#include "sensors.h"
#include "pid.h"

// Turn alignment PID - runs after completing a turn to center the bot on the line
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

// Make U-turn with controlled speeds and backward alignment
void make_uturn()
{
	Serial.println("Making U-turn...");

	// Full stop to eliminate inertia
	brake(left_motor, right_motor);
	delay(300);

	// Small forward movement to clear the line
	forward(left_motor, right_motor, 150);
	delay(80);
	brake(left_motor, right_motor);
	delay(100);

	// Phase 1: Rotate right until sensor 7 hits black (slower speed than regular turn)
	// Speed 320 = ±160 per motor (reduced from 400 for better control)
	right(left_motor, right_motor, 320);
	while (readSensor(7) == 0)
	{
	}

	// Phase 2: Continue rotation for alignment
	// Speed 240 = ±120 per motor (controlled speed)
	right(left_motor, right_motor, 240);
	while (readSensor(7) == 1)
	{
	}

	brake(left_motor, right_motor);
	delay(100);

	// Phase 3: Forward PID alignment
	Serial.println("Forward alignment...");
	turn_alignment_pid();
	brake(left_motor, right_motor);
	delay(100);

	// Phase 4: Backward PID for fine alignment
	Serial.println("Backward alignment...");
	backward_alignment_pid();
	brake(left_motor, right_motor);
	delay(100);

	// Phase 5: Final forward positioning
	forward(left_motor, right_motor, 80);
	delay(50);
	brake(left_motor, right_motor);

	Serial.print("U-turn complete! Position: ");
	Serial.println(readSensors());
}

// Test sequence for U-turn (dead end detection)
void test_sequence()
{
	Serial.println("Following line... Place bot at a dead end");

	while (1)
	{
		// Follow line until it exits (intersection or dead end)
		follow_segment();
		brake(left_motor, right_motor);
		delay(100);

		digitalWrite(LED_BUILTIN, HIGH);

		// Read sensors to check what happened
		readSensors();
		Serial.print("Stopped. Sensor readings: ");
		for (int i = 0; i < 8; i++)
		{
			Serial.print(isLine(i) ? "1" : "0");
		}
		Serial.println();

		// Check if all sensors are white (dead end)
		bool all_white = true;
		for (int i = 0; i < 8; i++)
		{
			if (isLine(i))
			{
				all_white = false;
				break;
			}
		}

		if (all_white)
		{
			Serial.println("All sensors white - moving forward to confirm...");

			// Move forward to confirm dead end
			forward(left_motor, right_motor, 120);
			delay(150);
			brake(left_motor, right_motor);
			delay(100);

			// Read sensors again
			readSensors();
			Serial.print("After forward. Sensor readings: ");
			for (int i = 0; i < 8; i++)
			{
				Serial.print(isLine(i) ? "1" : "0");
			}
			Serial.println();

			bool still_no_line = true;
			for (int i = 0; i < 8; i++)
			{
				if (isLine(i))
				{
					Serial.println("Found line - not a dead end");
					still_no_line = false;
					break;
				}
			}

			if (still_no_line)
			{
				Serial.println("Confirmed dead end! Making U-turn...");
				make_uturn();

				// Exit after single U-turn
				Serial.println("U-turn test complete!");
				digitalWrite(LED_BUILTIN, LOW);
				break;
			}
		}
		else
		{
			Serial.println("Not a dead end - continuing");
		}

		digitalWrite(LED_BUILTIN, LOW);
	}
}

void setup()
{
	Serial.begin(9600);
	qtrInit();

	Serial.println("=== U-TURN TEST ===");
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
	Serial.println("Press button to start U-turn test...");

	while (digitalRead(buttonPin) == 1)
	{
	}
	delay(100);
	while (digitalRead(buttonPin) == 0)
	{
	}
	delay(500);

	// Run the test sequence (follows until dead end, then U-turn)
	test_sequence();

	brake(left_motor, right_motor);
	Serial.println("Test finished. Reset to run again.");
	while (1)
	{
	}
}
