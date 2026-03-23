#ifndef PID_H
#define PID_H

#include <Arduino.h>

// PID to make Right/Left Turns
void turn_pid();

// PID to make a proper U-Turn
void uturn_pid();

// PID to align the bot after U-Turn
void backward_alignment_pid();

// PID to follow straight line
void follow_segment();

#endif