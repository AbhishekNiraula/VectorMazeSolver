#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include <SparkFun_TB6612.h>
#include "sensors.h"

extern int MAX_SPEED;

extern Motor left_motor;
extern Motor right_motor;

void makeTurn(char c);
void makeLeftTurn();
void makeRightTurn();
void makeUTurn();
#endif