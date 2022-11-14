// Récepteur étincelles aquatiques pour gérer une bande néopixel
// Le récepteur est configurable avec une adresse unique


//********************INCLUDES************//
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
/*********************************************/


//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 //the same on all nodes that talk to each other
#define NODEID 2

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY RF69_433MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD 115200

#define RFM69_CS 10
#define RFM69_IRQ 2
#define RFM69_IRQN 0 // Pin 2 is IRQ 0!
#define RFM69_RST 9

//*********************   DEFINE OUT/IN PINS **************************
#define LED_bas 7 // cette LED montre l'état du décor : si la LEd est allumé le décor doit aussi être allumé 
#define LED_haut 6 // cette LED montre l'état du décor : si la LEd est allumé le décor doit aussi être allumé 
#define reception 8 // le led clignote dès que le récepteur reçoit un message 
#define bas 3 //  pin commande du décor 1 
#define haut 4 // pin commande du décor 2 

#define BANDE1 A3 // pin de la bande LEd a définir
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      50

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, BANDE1, NEO_GRB + NEO_KHZ800);
RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);


//________________SETUP______________________
void setup() {
  //************** RADIO INIT *********************
   // Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);
  
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower(); // Only for RFM69HCW & HW!
  }
  radio.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm)
  radio.encrypt(ENCRYPTKEY);
  //*************************************************

  //**************** CONFIG DES ENTREES/SORTIES *************
  pinMode(LED_bas, OUTPUT);
  pinMode(LED_haut, OUTPUT);
  pinMode(reception, OUTPUT);
  pinMode(bas, OUTPUT) ;
  pinMode(haut, OUTPUT ) ;
  //******************************************************************

    /************* NEOPIXEL SETTINGS ********/
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin(); // This initializes the NeoPixel library.
  //*************************************

  //********* AFFICHE LES INFOS DU MODULES *******
 Serial.begin(SERIAL_BAUD);
  Serial.println("Feather RFM69HCW Receiver");
  Serial.print("\nListening at ");
  Serial.print(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");
  Serial.print("Node: ") ; Serial.println(NODEID) ;
//*************************************************

}

void loop() {

  /* // Some example procedures showing how to display to the pixels:
  colorWipe(pixels.Color(255, 0, 0), 50); // Red
  colorWipe(pixels.Color(0, 255, 0), 50); // Green
  colorWipe(pixels.Color(0, 0, 255), 50); // Blue
//colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // Send a theater pixel chase in...
  theaterChase(pixels.Color(127, 127, 127), 50); // White
  theaterChase(pixels.Color(127, 0, 0), 50); // Red
  theaterChase(pixels.Color(0, 0, 127), 50); // Blue
  //check if something was received (could be an interrupt from the radio)*/
  if (radio.receiveDone())
  {
    digitalWrite(reception, HIGH) ;
   /*Serial.print("[RX_RSSI:"); Serial.print(radio.RSSI); Serial.println("]");
   
    //int start_pixel = (radio.DATA[0]-1)/3  ;  // the first byte give the number of the first channel send 
    
    for (int i= 0; i <=19  ; i++) {
      pixels.setPixelColor(i+start_pixel , pixels.Color(radio.DATA[3*i+1],radio.DATA[3*i+2], radio.DATA[3*i+3])); // Moderately bright green color.
      Serial.print("Pixel:") ; Serial.print(i+start_pixel) ; Serial.print(": ") ;
      Serial.print(radio.DATA[3*i+1]) ;
      Serial.print(" ") ;
      Serial.print(radio.DATA[3*i + 2]) ;
      Serial.print(" ") ;
      Serial.print(radio.DATA[3*i + 3]) ;
      Serial.println(" ") ;
    }
    pixels.show();*/
  }
  colorWipe(pixels.Color(255, 0, 0), 50); // Red
  delay(1000);
  colorWipe(pixels.Color(0, 255, 0), 50); // Green
  delay(1000);
  colorWipe(pixels.Color(0, 0, 255), 50); // Blue
  delay(1000);
  /*if (radio.DATA[1]>128) 
      //colorWipe(pixels.Color(255, 0, 0), 50); // Red
      //rainbow(20);
      theaterChaseRainbow(50);
    else 
      //colorWipe(pixels.Color(0, 255, 0), 50); // Green
      rainbowCycle(20);*/
     
  digitalWrite(reception, LOW) ;
  //radio.receiveDone(); //put radio in RX mode
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i+j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, c);    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
