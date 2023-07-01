#ifndef hardware_h
#define hardware_h

/************** *************   INCLUDES LIBRARY ********************************************/
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>
#endif
/*********************************************************************************************/


//*********************   Définition des entrées/sorties *************************

// RFM69 PINS
#define RFM69_CS 10 // CHIP SELECT PIN
#define RFM69_IRQ 2 // pin IRQ
#define RFM69_IRQN 0 // Pin 2 is IRQ 0!
#define RFM69_RST 9 // RST PIN 

// HARDWARE OUTPUTS
#define LED1 6
#define LED2 7
#define LED3 8
#define T1 3 // Transistor 1 
#define T2 4 // Transistor 2 
#define BANDE1 A3 // pin pour contrôler la bande Led
//********************************************************************************


//*********************  Renommage des entrées sorties **************************

#define RECEPTION LED3 // la led clignote dès que le récepteur reçoit un message 
//**********************************************************************************






#endif
