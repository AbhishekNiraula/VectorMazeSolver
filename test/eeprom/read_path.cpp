#include <Arduino.h>
#include <EEPROM.h>

/*
 * EEPROM Path Reader Test
 *
 * This test program reads all the navigation decisions stored in EEPROM
 * from the previous micromouse run and prints them to Serial Monitor.
 *
 * EEPROM data persists across uploads and power cycles, so you can upload
 * this test program after a maze run to see what path the bot took.
 *
 * Instructions:
 * 1. Upload this test to your Arduino Nano
 * 2. Open Serial Monitor (9600 baud)
 * 3. Press the button (pin 5) to read and display the path
 *
 * To clear EEPROM: Uncomment the clearEEPROM() function call in loop()
 */

const int buttonPin = 5;
const int MAX_PATH_LENGTH = 1024; // Arduino Nano has 1KB EEPROM

void readStoredPath();

void setup()
{
	Serial.begin(9600);
	pinMode(buttonPin, INPUT_PULLUP);
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.println("=================================");
	Serial.println("EEPROM Path Reader - Ready");
	Serial.println("=================================");
	Serial.println("Press button to read stored path");
	Serial.println();
}

void loop()
{
	// Wait for button press
	if (digitalRead(buttonPin) == LOW)
	{
		delay(50); // Debounce

		// Wait for button release
		while (digitalRead(buttonPin) == LOW)
		{
			delay(10);
		}

		readStoredPath();

		// Optional: Uncomment the line below to clear EEPROM after reading
		// clearEEPROM();

		delay(1000); // Prevent multiple reads
	}
}

void readStoredPath()
{
	digitalWrite(LED_BUILTIN, HIGH);

	Serial.println("\n=== Reading Stored Path from EEPROM ===\n");

	int pathLength = 0;
	char direction;

	Serial.print("Path: ");

	// Read EEPROM until we hit an empty byte (0xFF) or invalid character
	for (int address = 0; address < MAX_PATH_LENGTH; address++)
	{
		direction = EEPROM.read(address);

		// Check if we've reached unwritten EEPROM (default value is 0xFF)
		// or if the character is not a valid direction
		if (direction == 0xFF || direction == 0x00)
		{
			break;
		}

		// Only print valid direction characters
		if (direction == 'L' || direction == 'R' || direction == 'S' || direction == 'B')
		{
			Serial.print(direction);
			pathLength++;

			// Add space every 10 characters for readability
			if (pathLength % 10 == 0)
			{
				Serial.print(" ");
			}
		}
		else
		{
			// Invalid character found, likely end of path
			break;
		}
	}

	Serial.println();
	Serial.print("\nTotal decisions recorded: ");
	Serial.println(pathLength);

	// Decode the path
	Serial.println("\nPath Legend:");
	Serial.println("  L = Left turn");
	Serial.println("  R = Right turn");
	Serial.println("  S = Straight");
	Serial.println("  B = U-turn (Back)");

	Serial.println("\n=== End of Path ===\n");

	digitalWrite(LED_BUILTIN, LOW);
}

void clearEEPROM()
{
	Serial.println("\n=== Clearing EEPROM ===");
	digitalWrite(LED_BUILTIN, HIGH);

	for (int address = 0; address < MAX_PATH_LENGTH; address++)
	{
		EEPROM.write(address, 0xFF);

		// Show progress every 100 bytes
		if (address % 100 == 0)
		{
			Serial.print(".");
		}
	}

	Serial.println("\nEEPROM cleared!");
	digitalWrite(LED_BUILTIN, LOW);
}
