#include <Arduino.h>
#include "motors.h"
#include "sensors.h"
#include "simplify.h"
#include "pid.h"

// Debug version of follow_segment with detailed output
void debug_follow_segment()
{
	// Balanced PID for smooth, stable line following
	float Kp = 0.042;
	float Kd = 0.5;
	int MAX_PID_SPEED = 110;
	int baseSpeed = 75;

	// Initialize lastError to 0 to prevent derivative spike on first iteration
	int lastError = 0;

	unsigned long loopCount = 0;
	unsigned long startTime = millis();

	Serial.println("\n========== FOLLOW_SEGMENT DEBUG START ==========");
	Serial.print("Kp=");
	Serial.print(Kp, 3);
	Serial.print(" | Kd=");
	Serial.print(Kd, 2);
	Serial.print(" | baseSpeed=");
	Serial.print(baseSpeed);
	Serial.print(" | MAX_PID_SPEED=");
	Serial.println(MAX_PID_SPEED);
	Serial.println("------------------------------------------------");
	Serial.println("Loop | Time(ms) | Pos | Error | P | D | MotSpd | L | R | Type");
	Serial.println("------------------------------------------------");

	while (true)
	{
		uint16_t position = readSensors();
		int error = position - 3500;

		// P and D terms
		float proportional = Kp * error;
		float derivative = Kd * (error - lastError);

		// Reasonable derivative limits
		if (derivative > 50)
			derivative = 50;
		if (derivative < -50)
			derivative = -50;

		float motorSpeed = proportional + derivative;

		// Fixed motor assignment - swap left and right
		int rightMotorSpeed = baseSpeed + motorSpeed;
		int leftMotorSpeed = baseSpeed - motorSpeed;

		// Clamping
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

		// Detailed intersection detection
		bool extremeOnLine = (sensorValues[0] > threshold[0] || sensorValues[7] > threshold[7]);
		bool centerOnLine = (sensorValues[3] > threshold[3] || sensorValues[4] > threshold[4]);

		bool allWhite = true;
		for (int i = 0; i < 8; i++)
		{
			if (sensorValues[i] > threshold[i])
			{
				allWhite = false;
				break;
			}
		}

		bool intersection = false;
		String intersectionType = "none";

		if (extremeOnLine && centerOnLine)
		{
			intersection = true;
			intersectionType = "L/R/T";
		}
		else if (allWhite)
		{
			intersection = true;
			intersectionType = "U-TURN";
		}

		// Print debug info every 10 loops or when intersection found
		if (loopCount % 10 == 0 || intersection)
		{
			Serial.print(loopCount);
			Serial.print(" | ");
			Serial.print(millis() - startTime);
			Serial.print(" | ");
			Serial.print(position);
			Serial.print(" | ");
			Serial.print(error);
			Serial.print(" | ");
			Serial.print(proportional, 1);
			Serial.print(" | ");
			Serial.print(derivative, 1);
			Serial.print(" | ");
			Serial.print(motorSpeed, 1);
			Serial.print(" | ");
			Serial.print(leftMotorSpeed);
			Serial.print(" | ");
			Serial.print(rightMotorSpeed);
			Serial.print(" | ");
			Serial.println(intersectionType);
		}

		if (intersection)
		{
			brake(left_motor, right_motor);
			Serial.println("------------------------------------------------");
			Serial.print("INTERSECTION DETECTED - Type: ");
			Serial.println(intersectionType);
			Serial.print("Total loops: ");
			Serial.println(loopCount);
			Serial.print("Duration: ");
			Serial.print(millis() - startTime);
			Serial.println("ms");

			// Print sensor state at intersection
			Serial.print("Sensor values: ");
			for (int i = 0; i < 8; i++)
			{
				Serial.print(sensorValues[i]);
				Serial.print(isLine(i) ? "*" : " ");
				Serial.print(" ");
			}
			Serial.println();
			Serial.println("========== FOLLOW_SEGMENT DEBUG END ==========\n");
			return;
		}

		lastError = error;
		loopCount++;
	}
}

void setup()
{
	Serial.begin(9600);
	qtrInit();

	Serial.println("Setup Complete. Waiting 2 seconds before starting loop...");
	delay(2000);
}

void loop()
{
	// Wait until button is pressed to start
	while (digitalRead(buttonPin) == 1)
	{
	}
	// Debounce the initial press (ignore contact bounce)
	delay(50);

	// Wait for release (Button must go HIGH/1)
	while (digitalRead(buttonPin) == 0)
	{
		delay(10);
	}

	// Debounce the release (ignore contact bounce on release)
	// This prevents the "tail" of the first click from registering as a second click
	delay(100);

	// Check for a second press within a short window (e.g., 500ms)
	// We are looking for the button to go LOW (0) again.
	unsigned long startTime = millis();
	bool doubleClick = false;
	while (millis() - startTime < 500)
	{
		if (digitalRead(buttonPin) == 0)
		{
			doubleClick = true;
			break;
		}
	}

	if (doubleClick)
	{
		Serial.println("Double Click Detected: Calibrating...");
		// Debounce the second press
		delay(50);
		// Wait for button release of the second click
		while (digitalRead(buttonPin) == 0)
		{
			delay(10);
		}
		delay(1000); // Time to move hand away
		qtrCalibrate();
	}
	else
	{
		Serial.println("Single Click Detected: Skipping Calibration...");
	}

	while (true)
	{
		Serial.println("\nPress button to start debug follow_segment...");
		while (digitalRead(buttonPin) == 1)
		{
		}
		delay(50);
		while (digitalRead(buttonPin) == 0)
		{
			delay(10);
		}
		delay(1000); // Prepare to place robot

		debug_follow_segment();

		delay(2000); // Wait before next run
	}
}