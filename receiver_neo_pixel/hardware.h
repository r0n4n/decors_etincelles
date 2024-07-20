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

// Strips LEDs config
#define STRIP_CONFIG STRIP_QUAD // STRIP_SINGLE OR STRIP_DOUBLE OR STRIP_QUAD
#define STRIP_SINGLE 1 // 
#define STRIP_DOUBLE 2 //
#define STRIP_QUAD 3 //

// PIN actionneurs
#define LED_ONOFF 8
#define LED_RECEPTION 7
#define LED1 6
//#define T1 3 // Transistor 1 
//#define T2 4 // Transistor 2 
//#define PIN_RELAY 5

#if (STRIP_CONFIG == STRIP_QUAD)
  #define PIN_STRIP1 A3 // haut 1 
  #define PIN_STRIP2 3
  #define PIN_STRIP3 5 // bas 1
  #define PIN_STRIP4 4 // bas 2
#else
  #define PIN_STRIP1 A3 // pin pour contrôler la bande Led
  #define PIN_STRIP2 5 
#endif 

#define NUMPIXELS      50.0
//********************************************************************************



#if (STRIP_CONFIG == STRIP_QUAD)
  #define STRIP1_LEDS_NBR 15 // haut 1 
  #define STRIP2_LEDS_NBR 12 // haut 2
  #define STRIP3_LEDS_NBR 8 // bas 1
  #define STRIP4_LEDS_NBR 11 // bas 2
  #define STRIP1_ADDRESS DECOR_DMX_ADRESS
  #define STRIP2_ADDRESS STRIP1_ADDRESS + 3*STRIP1_LEDS_NBR 
  #define STRIP3_ADDRESS STRIP2_ADDRESS + 3*STRIP2_LEDS_NBR
  #define STRIP4_ADDRESS STRIP3_ADDRESS + 3*STRIP3_LEDS_NBR
#else 
  #define STRIP1_LEDS_NBR 27
  #define STRIP2_LEDS_NBR 19
  #define STRIP1_ADDRESS DECOR_DMX_ADRESS
  #define STRIP2_ADDRESS DECOR_DMX_ADRESS + 3*STRIP1_LEDS_NBR
#endif 

#define STRIP_ONOFF_LED 7
#define ONOFF_LED_BRIGHTNESS 1

#define RBG
#define RGB

// ***************** Configuration du module RFM69 *********************
#define FREQUENCY RF69_433MHZ //Match frequency to the hardware version of the radio on your Feather
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module
//**************************************************************************

#endif
