#include <Arduino.h>
#include "motors.h"
#include "sensors.h"
#include "pid.h"

// Test sequence matching main.cpp logic
void test_uturn_sequence()
{
	Serial.println("\n=== Starting U-turn Test Sequence ===");
	Serial.println("Following line until dead-end detected...\n");

	while (1)
	{
		// Follow segment until intersection/dead-end detected (matches main.cpp)
		follow_segment();
		left_motor.standby();
		right_motor.standby();
		delay(100);

		// LED indicates intersection handling
		digitalWrite(LED_BUILTIN, HIGH);
		Serial.println("--- Intersection Detected ---");

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

		Serial.print("Initial sensor check - Left: ");
		Serial.print(found_left ? "YES" : "NO");
		Serial.print(" | Right: ");
		Serial.println(found_right ? "YES" : "NO");

		// Drive forward to align wheels with intersection
		Serial.println("Moving forward to check straight path...");
		forward(left_motor, right_motor, 180);
		delay(250);
		brake(left_motor, right_motor);
		delay(100);

		// Check for straight path
		readSensors();
		if (isLine(2) || isLine(3) || isLine(4) || isLine(5))
		{
			found_straight = true;
		}

		Serial.print("Straight path: ");
		Serial.println(found_straight ? "YES" : "NO");

		// Read sensors again for final check
		readSensors();

		// Print all sensor readings
		Serial.print("All sensors: ");
		for (int i = 0; i < 8; i++)
		{
			Serial.print(isLine(i) ? "1" : "0");
		}
		Serial.println();

		// Check for end marker (all black)
		if (isLine(0) && isLine(1) && isLine(2) && isLine(3) && isLine(4) && isLine(5) && isLine(6) && isLine(7))
		{
			Serial.println("\n*** END MARKER DETECTED (all black) - Test Complete! ***");
			digitalWrite(LED_BUILTIN, LOW);
			return;
		}

		// Dead-end detection: if ALL sensors read white (no line detected anywhere), trigger u-turn
		bool allWhite = !isLine(0) && !isLine(1) && !isLine(2) && !isLine(3) && !isLine(4) && !isLine(5) && !isLine(6) && !isLine(7);

		if (allWhite)
		{
			Serial.println("\n*** DEAD-END DETECTED (all white) ***");
			Serial.println("Initiating U-turn sequence...\n");

			// Execute U-turn using the same function as main.cpp
			makeTurn('B');

			Serial.println("\n*** U-TURN COMPLETE ***");
			Serial.print("Final position: ");
			Serial.println(readSensors());
			Serial.println("Continuing to follow line...\n");
		}
		else
		{
			Serial.print("\nDecision - Left: ");
			Serial.print(found_left ? "YES" : "NO");
			Serial.print(" | Straight: ");
			Serial.print(found_straight ? "YES" : "NO");
			Serial.print(" | Right: ");
			Serial.println(found_right ? "YES" : "NO");
			Serial.println("Not a dead-end, but test only handles U-turns.");
			Serial.println("Please place bot at a dead-end for proper testing.\n");
		}

		// Turn off LED after handling intersection
		digitalWrite(LED_BUILTIN, LOW);
	}
}

void setup()
{
	qtrInit();
	Serial.begin(9600);
}

void loop()
{
	Serial.println("\n================================");
	Serial.println("    U-TURN TEST PROGRAM");
	Serial.println("================================");
	Serial.println("\nThis test matches main.cpp logic:");
	Serial.println("1. Follows line segments");
	Serial.println("2. Detects dead-ends (all white)");
	Serial.println("3. Executes U-turn with proper PID alignment\n");

	// Wait until button is pressed to start
	Serial.println("Press button once to skip calibration");
	Serial.println("Double-click button to calibrate sensors\n");

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
		Serial.println("Calibration complete!");
	}
	else
	{
		Serial.println("Single Click Detected: Skipping Calibration...");
	}

	delay(1000);
	Serial.println("\nPress button to start U-turn test...");

	while (digitalRead(buttonPin) == 1)
	{
	}
	delay(50);
	while (digitalRead(buttonPin) == 0)
	{
		delay(10);
	}
	delay(1000); // Prepare to place robot

	Serial.println("\n*** TEST STARTING ***\n");

	// Run the test sequence once
	test_uturn_sequence();

	// Stop after test completes
	brake(left_motor, right_motor);
	Serial.println("\n================================");
	Serial.println("    TEST FINISHED");
	Serial.println("================================");
	Serial.println("Reset Arduino to run again.\n");

	while (1)
	{
		// Infinite loop - reset required
	}
}
