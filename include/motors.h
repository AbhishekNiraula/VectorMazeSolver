#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include <SparkFun_TB6612.h>
#include <EEPROM.h>
#include "sensors.h"

extern int MAX_SPEED;

extern Motor left_motor;
extern Motor right_motor;

void makeTurn(char c);
void makeLeftTurn();
void makeRightTurn();
void makeUTurn();

// Turn without EEPROM Write
void executeTurn(char c);
void executeLeftTurn();
void executeRightTurn();
void executeUTurn();

// Maze solving algorithms
char lHAlgorithm(bool found_left, bool found_straight, bool found_right, bool found_uturn);
char rHAlgorithm(bool found_right, bool found_straight, bool found_left, bool found_uturn);
#endif