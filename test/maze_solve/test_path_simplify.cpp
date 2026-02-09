#include <Arduino.h>
#include <EEPROM.h>

void simplify_path();

void setup()
{
	Serial.begin(9600);
	while (!Serial)
	{
		delay(10);
	}

	delay(1000);
	Serial.println("\n=== Path Simplification Test ===\n");

	Serial.println("Reading current EEPROM path...");
	Serial.print("Current path: ");
	int count = 0;
	for (int i = 0; i < 100; i++)
	{
		char turn = EEPROM.read(i);
		if (turn == 0xFF)
			break;
		Serial.print(turn);
		count++;
	}
	Serial.println();
	Serial.print("Path length: ");
	Serial.println(count);
	Serial.println();

	if (count == 0)
	{
		Serial.println("EEPROM is empty! Run a dry_run first to record a path.");
	}
	else
	{
		Serial.println("Simplifying path...");
		simplify_path();

		Serial.println("\n=== Final Simplified Path ===");
		Serial.print("Final path: ");
		for (int i = 0; i < 100; i++)
		{
			char turn = EEPROM.read(i);
			if (turn == 0xFF)
				break;
			Serial.print(turn);
		}
		Serial.println();
		Serial.println("\nThis is the path that will be used in maze_solve()");
	}
}

void loop()
{
	// Nothing to do
}
