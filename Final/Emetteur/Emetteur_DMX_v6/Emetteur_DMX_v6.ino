// Programme permettant la lecture de la tram DMX et envoit des données par liaison RF a tous les décors

#include <DMXSerial.h>                   // Appel de la librairie SerialDMX
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69


//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 // The same on all nodes that talk to each other
#define NODEID 1 // The unique identifier of this node
#define NODERECEIVE 2

//******************* LIST OF RECEIVERS *****************

//*******************************************************

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY RF69_433MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module


//*********************************************************************************************
#define RFM69_CS 10 // Chip select
#define RFM69_IRQ 2 // interruption 
#define RFM69_IRQN 0 // Pin 2 is IRQ 0! 
#define RFM69_RST 9 // Reset 

//*********************   DEFINE OUT/IN PINS **************************
#define LED 8 // onboard blinky
#define BP 7 // pin du bouton poussoir 
#define LedDMX  5

uint8_t radiopacket[61] ;
uint8_t radiopacket1[61] ;
//uint8_t *channels ;


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
  //channels = DMXSerial.getBuffer() ; 
}


void loop () { // Boucle du programme principal
  // vérifie si des données ont été reçues via la liaison DMX
  if (DMXSerial.noDataSince() > 1)      // LED Reception du signal
    digitalWrite(LedDMX, LOW);            // Si le signal n'a pas été reçu depuis + de 10 ms, la LED s'éteint
  else { 
    digitalWrite(LedDMX, HIGH);
    //channels = DMXSerial.getBuffer() ; 
    //radio.send(NODERECEIVE, (const void*)channels, 61, false) ;
    
    radiopacket[0] = 1 ;
    for (int i = 1; i < 61 ; i++) { // envoit des messages pour chaque récepteur. i est l'adresse du récepteur
      radiopacket[i] = DMXSerial.read(i) ;
      if (radiopacket[i] ==0 )
        radiopacket[i] = 1 ; 
    }

    radiopacket1[0] = 61 ;
    for (int i = 61; i < 121; i++) { // envoit des messages pour chaque récepteur. i est l'adresse du récepteur
      radiopacket1[i-60] = DMXSerial.read(i) ;
      if (radiopacket1[i-60] ==0 )
        radiopacket1[i-60] = 1 ;
    }
  }

  // envoit les 60 premiers canaux DMX 
  radio.send(NODERECEIVE, (const void*)radiopacket, strlen(radiopacket), false) ;
  
 //envoit les canaux DMX de 61 à 120 
 //radio.send(NODERECEIVE, (const void*)radiopacket1, strlen(radiopacket1), false) ;


 //delay (50) ; 
  
}
