#ifndef Morse_h
#define Morse_h

#include "Arduino.h"
#define DMXSERIAL_MAX 512

void fulloff(uint8_t  *manData);
void fullOn(uint8_t  *data);
void fullRed(uint8_t  *data);
void fullGreen(uint8_t  *data);
void fullBlue(uint8_t  *data);
void RVBSequence(uint8_t  *data, int _delay);

#endif
