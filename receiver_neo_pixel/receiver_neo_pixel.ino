// Récepteur étincelles aquatiques pour gérer une bande néopixel
// Le récepteur est configurable avec une adresse unique


//********************INCLUDES************//
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
//#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
/*********************************************/


//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 //the same on all nodes that talk to each other
#define NODEID 4

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


#define LED_bas 7 // cette LED montre l'état du décor : si la LEd est allumé le décor doit aussi être allumé 
#define LED_haut 6 // cette LED montre l'état du décor : si la LEd est allumé le décor doit aussi être allumé 
#define reception 8 // le led clignote dès que le récepteur reçoit un message 
#define bas 3 //  pin commande du décor 1 
#define haut 4 // pin commande du décor 2 

#define BANDE1 2 // pin de la bande LEd a définir
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      16
int delayval = 500; // delay for half a second 


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, BANDE1, NEO_GRB + NEO_KHZ800);
RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

//________________SETUP______________________

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Feather RFM69HCW Receiver");

  // Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);

  // Initialize radio
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower(); // Only for RFM69HCW & HW!
  }
  radio.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm)
  radio.encrypt(ENCRYPTKEY);

  // Configuration des entrées/sorties
  pinMode(LED_bas, OUTPUT);
  pinMode(LED_haut, OUTPUT);
  pinMode(reception, OUTPUT);
  pinMode(bas, OUTPUT) ;
  pinMode(haut, OUTPUT ) ;

  Serial.print("\nListening at ");
  Serial.print(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");
  Serial.print("Node: ") ;
  Serial.println(NODEID) ;

  /************* NEOPIXEL SETTINGS ********/
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
  /*************************************/
}

void loop() {
  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    digitalWrite(reception, HIGH) ;
    int state_bas = (int)radio.DATA[0] ;
    int state_haut = (int)radio.DATA[1] ;
    //print message received to serial
    //Serial.print('['); Serial.print(radio.SENDERID); Serial.print("] ");
    //Serial.println(state_bas);
    //Serial.println(state_haut);
    Serial.println() ;
    Serial.print("[RX_RSSI:"); Serial.print(radio.RSSI); Serial.println("]");
    Serial.println(radio.DATA[2]) ;
    //check if received message contains

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.println(" - ACK sent");
    }

    if (state_bas == 49) {
      Serial.println("  state_bas on") ;
      pixels.setBrightness(0) ;
      digitalWrite(LED_bas, HIGH) ;
    }
    if (state_bas == 48) {
      Serial.println("  state_bas off") ;
      for (int i = 0; i < NUMPIXELS; i++) {

        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(0, 150, 0)); // Moderately bright green color.
        pixels.setBrightness(255) ; 
        pixels.show(); // This sends the updated pixel color to the hardware.
        delay(delayval); // Delay for a period of time (in milliseconds).

      }
      digitalWrite(LED_bas, LOW) ;
    }
  }
  digitalWrite(reception, LOW) ;
  radio.receiveDone(); //put radio in RX mode
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
}

