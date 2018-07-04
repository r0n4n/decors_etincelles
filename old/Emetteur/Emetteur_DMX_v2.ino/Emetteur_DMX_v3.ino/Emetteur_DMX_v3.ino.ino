// Programme permettant la lecture de la tram DMX et envoit des données par liaison RF

#include <DMXSerial.h>                   // Appel de la librairie SerialDMX
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 // The same on all nodes that talk to each other
#define NODEID 1 // The unique identifier of this node

//******************* LIST OF RECEIVERS *****************
//#define RECEIVER 2 // The recipient of packets
int RECEIVER = 2 ; 



//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY RF69_433MHZ
//#define FREQUENCY RF69_868MHZ
//#define FREQUENCY RF69_915MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module


//*********************************************************************************************
#define SERIAL_BAUD 115200

#define RFM69_CS 10 // Chip select
#define RFM69_IRQ 2 // interruption 
#define RFM69_IRQN 0 // Pin 2 is IRQ 0! 
#define RFM69_RST 9 // Reset 



//*********************   DEFINE OUT/IN PINS **************************
#define LED 8 // onboard blinky
#define BP 7 // pin du bouton poussoir 
#define LedDMX  5


#define nbr_canaux 6
const int CANAL[nbr_canaux] = {1, 2, 3, 4, 5, 6}; // Définit un tableau pour les canaux d'adressage de sortie
int dmx_memory[nbr_canaux] = {0, 0, 0, 0, 0, 0};
char modes_memory[nbr_canaux] = {0, 0, 0, 0, 0, 0};
int ACK_received[nbr_canaux] = {0, 0, 0, 0, 0, 0};


RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

void setup () { // Configuration au démarrage

  DMXSerial.init(DMXReceiver);          // Initialise la carte comme un récepteur DMX

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

  pinMode(LED, OUTPUT);
  pinMode(LedDMX, OUTPUT);
  pinMode(BP, INPUT);

}

void loop () { // Boucle du programme principal

  if (DMXSerial.noDataSince() > 100)      // LED Reception du signal
    digitalWrite(LedDMX, LOW);            // Si le signal n'a pas été reçu depuis + de 100 ms, la LED s'éteint
  else
    digitalWrite(LedDMX, HIGH);


  // LECTURE DE LA TRAM DMX
  char radiopacket[5] ;

  for (int i = 0; i < nbr_canaux; i++) {          // Lecture des canaux et traduction en MLI
    //if (DMXSerial.read(CANAL[i]) != dmx_memory[i]) {
      //dmx_memory[i] = DMXSerial.read(CANAL[i]) ;
      if (DMXSerial.read(CANAL[i]) > 128) {
        //int rang = i -(i/2)*2 ;
        radiopacket[i] = '1';
      }
      else if (DMXSerial.read(CANAL[i]) < 128) {
        int rang = i -(i/2)*2 ;
        radiopacket[i] = '0';
      }
      RECEIVER = (i/2)+2 ; 
      radio.send(RECEIVER, radiopacket, strlen(radiopacket), false) ;
      //ACK_received[i]=0 ; 
      radio.receiveDone(); //put radio in RX mode*/
    //}
    /*else if (ACK_received[i]==0){
      if (DMXSerial.read(CANAL[i]) > 128) {
        int rang = i -(i/2)*2 ;
        radiopacket[rang] = '1';
      }
      else if (DMXSerial.read(CANAL[i]) < 128) {
        int rang = i -(i/2)*2 ;
        radiopacket[rang] = '0';
      }
      RECEIVER = (i/2)+2 ;
      if (radio.sendWithRetry(RECEIVER, radiopacket, strlen(radiopacket))) {  
          ACK_received[i] = 1 ; 
      }
    }*/
  }


  /*
    char radiopacket[5] ;
    if (DMXSerial.read(1) > 128) {
      radiopacket[0] = '1';
      //digitalWrite(LedDMX, HIGH);
    }
    else if (DMXSerial.read(1) < 128) {
      radiopacket[0] = '0';
      //digitalWrite(LedDMX, LOW);
    }
  */

  /*
    char radiopacket[5] ;
    radiopacket[0] = '0' ;

    if (digitalRead(BP) == 1) {
    //Serial.print("on") ;
    radiopacket[num] = '0';
    }

    else if (digitalRead(BP) == 0) {
    radiopacket[num] = '1';
    }*/



}







