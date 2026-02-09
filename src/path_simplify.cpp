#include <Arduino.h>
#include <EEPROM.h>

void simplify_path()
{
	char path[100];
	int pathLength = 0;

	// Read path from EEPROM
	for (int i = 0; i < 100; i++)
	{
		char turn = EEPROM.read(i);
		if (turn == 0xFF)
			break;
		path[pathLength++] = turn;
	}

	if (pathLength < 3)
	{
		return;
	}

	// Patterns: LBR=B, LBS=R, RBL=B, SBL=R, SBS=B, LBL=S
	bool simplified = true;
	while (simplified)
	{
		simplified = false;
		for (int i = 1; i < pathLength - 1; i++)
		{
			if (path[i] == 'B')
			{
				char before = path[i - 1];
				char after = path[i + 1];
				char newTurn = 0;

				// Pattern matching
				if (before == 'L' && after == 'R')
					newTurn = 'B'; // LBR = B
				else if (before == 'L' && after == 'S')
					newTurn = 'R'; // LBS = R
				else if (before == 'R' && after == 'L')
					newTurn = 'B'; // RBL = B
				else if (before == 'S' && after == 'L')
					newTurn = 'R'; // SBL = R
				else if (before == 'S' && after == 'S')
					newTurn = 'B'; // SBS = B
				else if (before == 'L' && after == 'L')
					newTurn = 'S'; // LBL = S
				else if (before == 'R' && after == 'R')
					newTurn = 'S'; // RBR = S
				else if (before == 'R' && after == 'S')
					newTurn = 'L'; // RBS = L
				else if (before == 'S' && after == 'R')
					newTurn = 'L'; // SBR = L

				if (newTurn != 0)
				{
					// Replace the 3 turns with 1 simplified turn
					path[i - 1] = newTurn;

					// Shift remaining path elements left by 2
					for (int j = i; j < pathLength - 2; j++)
					{
						path[j] = path[j + 2];
					}
					pathLength -= 2;
					simplified = true;
					break;
				}
			}
		}
	}

	// Write simplified path back to EEPROM
	for (int i = 0; i < pathLength; i++)
	{
		EEPROM.write(i, path[i]);
	}
	// Mark end of path
	EEPROM.write(pathLength, 0xFF);

	// Clear remaining EEPROM
	for (int i = pathLength + 1; i < 100; i++)
	{
		EEPROM.write(i, 0xFF);
	}
}
