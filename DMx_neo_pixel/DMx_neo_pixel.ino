#include <DMXSerial.h>                   // Appel de la librairie SerialDMX


//*********************   DEFINE OUT/IN PINS **************************
#define LED 8 // onboard blinky
#define BP 7 // pin du bouton poussoir 
#define LedDMX  5
#define BANDE1 3 // pin de la bande LEd a définir
#define NUMPIXELS      30

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, BANDE1, NEO_GRB + NEO_KHZ800);

void setup() {
  DMXSerial.init(DMXReceiver);          // Initialise la carte comme un récepteur DMX
  pinMode(LedDMX, OUTPUT);
  pinMode(LED, OUTPUT);

  /************* NEOPIXEL SETTINGS ********/
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
  /*************************************/
}

void loop() {
 
   if (DMXSerial.noDataSince() > 10)      // LED Reception du signal
     digitalWrite(LedDMX, LOW);            // Si le signal n'a pas été reçu depuis + de 100 ms, la LED s'éteint
    else {
     digitalWrite(LedDMX, HIGH);
    }
 
   for (int i = 0; i < NUMPIXELS ; i++) { // envoit des messages pour chaque récepteur. i est l'adresse du récepteur
       pixels.setPixelColor(i, pixels.Color( DMXSerial.read(3 * i + 3), DMXSerial.read(3 * i + 1), DMXSerial.read(3 * i + 2)));
    }
    pixels.show();
    delay(50) ;
}




