#include <Arduino.h>
#include <EEPROM.h>
#include "sensors.h"
#include "motors.h"
#include "pid.h"
#include "simplify.h"

// maze_solve is already defined in maze_solve.cpp
extern void maze_solve();

// Test case structure
struct TestCase
{
	const char *name;
	const char *inputPath;
	const char *expectedOutput;
};

// Test cases for path simplification
TestCase simplificationTests[] = {
	{"LBR -> B", "LBR", "B"},
	{"LBS -> R", "LBS", "R"},
	{"RBL -> B", "RBL", "B"},
	{"SBL -> R", "SBL", "R"},
	{"SBS -> B", "SBS", "B"},
	{"LBL -> S", "LBL", "S"},
	{"Complex: LBRSBL", "LBRSBL", "BR"},  // LBR->B, then SBL->R
	{"Multiple: LBLSBS", "LBLSBS", "SB"}, // LBL->S, then SBS->B
	{"No simplification", "LRS", "LRS"},
	{"Long path: LBRLBSSBL", "LBRLBSSBL", "BRR"}, // LBR->B, LBS->R, SBL->R
};

int numTests = sizeof(simplificationTests) / sizeof(TestCase);

// Helper function to write test path to EEPROM
void writeTestPath(const char *path)
{
	int i;
	for (i = 0; path[i] != '\0'; i++)
	{
		EEPROM.write(i, path[i]);
	}
	// Mark end of path
	EEPROM.write(i, 0xFF);
	// Clear rest
	for (int j = i + 1; j < 100; j++)
	{
		EEPROM.write(j, 0xFF);
	}
}

// Helper function to read path from EEPROM
void readTestPath(char *buffer, int maxLen)
{
	for (int i = 0; i < maxLen; i++)
	{
		char turn = EEPROM.read(i);
		if (turn == 0xFF)
		{
			buffer[i] = '\0';
			break;
		}
		buffer[i] = turn;
	}
}

// Helper function to compare paths
bool comparePaths(const char *path1, const char *path2)
{
	int i = 0;
	while (path1[i] != '\0' && path2[i] != '\0')
	{
		if (path1[i] != path2[i])
			return false;
		i++;
	}
	return path1[i] == path2[i]; // Both should be '\0'
}

// Test path simplification
void testPathSimplification()
{
	Serial.println("\n=== PATH SIMPLIFICATION TESTS ===\n");

	int passed = 0;
	int failed = 0;

	for (int i = 0; i < numTests; i++)
	{
		Serial.print("Test ");
		Serial.print(i + 1);
		Serial.print(": ");
		Serial.print(simplificationTests[i].name);
		Serial.print(" ... ");

		// Write test input to EEPROM
		writeTestPath(simplificationTests[i].inputPath);

		// Run simplification
		simplify_path();

		// Read result
		char result[100];
		readTestPath(result, 100);

		// Compare
		if (comparePaths(result, simplificationTests[i].expectedOutput))
		{
			Serial.println("PASS");
			passed++;
		}
		else
		{
			Serial.print("FAIL (Expected: ");
			Serial.print(simplificationTests[i].expectedOutput);
			Serial.print(", Got: ");
			Serial.print(result);
			Serial.println(")");
			failed++;
		}

		delay(100);
	}

	Serial.println("\n--- Test Summary ---");
	Serial.print("Passed: ");
	Serial.println(passed);
	Serial.print("Failed: ");
	Serial.println(failed);
	Serial.print("Total: ");
	Serial.println(numTests);
}

// Test maze solve with manual path
void testMazeSolveWithPath(const char *testPath)
{
	Serial.println("\n=== MAZE SOLVE TEST ===\n");
	Serial.print("Testing with path: ");
	Serial.println(testPath);

	// Write test path to EEPROM
	writeTestPath(testPath);

	Serial.println("Path written to EEPROM");
	Serial.println("NOTE: This test requires physical maze setup");
	Serial.println("\nCalibrating sensors...");
	Serial.println("Press button when ready to calibrate...");

	// Wait for button press
	while (digitalRead(buttonPin) == 1)
	{
	}
	delay(200);
	while (digitalRead(buttonPin) == 0)
	{
		delay(10);
	}
	delay(1000); // Time to move hand away

	// Calibrate sensors
	qtrCalibrate();

	Serial.println("Calibration complete!");
	Serial.println("Press button to start maze_solve...");

	// Wait for button press to start maze solve
	while (digitalRead(buttonPin) == 1)
	{
	}
	delay(200);
	while (digitalRead(buttonPin) == 0)
	{
		delay(10);
	}
	delay(500);

	// Run maze solve
	maze_solve();

	Serial.println("Maze solve test complete");
}

// Display current path in EEPROM
void displayCurrentPath()
{
	Serial.println("\n=== CURRENT PATH IN EEPROM ===\n");

	char path[100];
	readTestPath(path, 100);

	Serial.print("Path: ");
	Serial.println(path);

	Serial.print("Length: ");
	Serial.println(strlen(path));

	Serial.println("\nDetailed breakdown:");
	for (int i = 0; path[i] != '\0'; i++)
	{
		Serial.print("Step ");
		Serial.print(i + 1);
		Serial.print(": ");
		switch (path[i])
		{
		case 'L':
			Serial.println("Left");
			break;
		case 'R':
			Serial.println("Right");
			break;
		case 'S':
			Serial.println("Straight");
			break;
		case 'B':
			Serial.println("Back (U-turn)");
			break;
		default:
			Serial.print("Unknown (");
			Serial.print(path[i]);
			Serial.println(")");
		}
	}
}

void setup()
{
	Serial.begin(9600);
	qtrInit();

	delay(2000);

	Serial.println("\n\n=================================");
	Serial.println("PATH SOLVER TEST SUITE");
	Serial.println("=================================\n");

	Serial.println("Test Sequence:");
	Serial.println("1. Path simplification tests (automated)");
	Serial.println("2. Display current EEPROM path");
	Serial.println("3. Maze solve test with calibration");
	Serial.println("\nPress button to start simplification tests...");
}

void loop()
{
	// Wait for button press
	while (digitalRead(buttonPin) == 1)
	{
	}
	delay(200);
	while (digitalRead(buttonPin) == 0)
	{
		delay(10);
	}
	delay(500);

	// Run automated tests first
	testPathSimplification();

	delay(2000);

	// Display current path
	displayCurrentPath();

	Serial.println("\n\nPress button to test maze_solve with path 'LRSR'...");

	while (digitalRead(buttonPin) == 1)
	{
	}
	delay(200);
	while (digitalRead(buttonPin) == 0)
	{
		delay(10);
	}
	delay(500);

	// Test maze solve
	testMazeSolveWithPath("LRSR");

	Serial.println("\n\nAll tests complete! Press reset to run again.");

	// Stop
	while (1)
	{
		delay(1000);
	}
}
