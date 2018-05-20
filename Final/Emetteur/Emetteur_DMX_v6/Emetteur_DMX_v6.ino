// Programme permettant la lecture de la tram DMX et envoit des données par liaison RF a tous les décors

#include <DMXSerial.h>                   // Appel de la librairie SerialDMX
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69


//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 // The same on all nodes that talk to each other
#define NODEID 1 // The unique identifier of this node


//******************* LIST OF RECEIVERS *****************
#define NODERECEIVE 2
//*******************************************************

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY RF69_433MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module

#define RFM69_CS 10 // Chip select
#define RFM69_IRQ 2 // interruption 
#define RFM69_IRQN 0 // Pin 2 is IRQ 0! 
#define RFM69_RST 9 // Reset 

//*********************   DEFINE OUT/IN PINS **************************
#define LED 8 // onboard blinky
#define BP 7 // pin du bouton poussoir 
#define LedDMX  5
#define DEBUGPIN 3

/**********************************************************************/

/****************** DEFINE LIST ***************************/
#define PACKET_SIZE 61 // number of channels to send per packet   
#define PACKET_NBR 7
/*************************************************************/
uint8_t radiopacket[61] ;
uint8_t radiopacket1[61] ;
//uint8_t *channels ;
int buff ;
int packet_id_list[PACKET_NBR] = {1, 61, 121, 181, 241, 301, 361} ;


RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

void setup () { // Configuration au démarrage
  DMXSerial.init(DMXReceiver);          // Initialise la carte comme un récepteur DMX


  //DMXSerial.attachOnUpdate(_channels_updated) ;

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
  pinMode(DEBUGPIN, OUTPUT) ;
  //channels = DMXSerial.getBuffer() ;x
  //  buff = DMXSerial.read(3) ;
  //  digitalWrite(DEBUGPIN, HIGH) ;

  DMXSerial.attachOnUpdate(_DMX_RFM69_send) ;

}


void loop () { // Boucle du programme principal
  // vérifie si des données ont été reçues via la liaison DMX
  if (DMXSerial.noDataSince() > 1)      // LED Reception du signal
    digitalWrite(LedDMX, LOW);            // Si le signal n'a pas été reçu depuis + de 10 ms, la LED s'éteint
  else
    digitalWrite(LedDMX, HIGH);
  //channels = DMXSerial.getBuffer() ;
  //radio.send(NODERECEIVE, (const void*)channels, 61, false) ;
  //_trigger_detector(3) ;
  //_DMX_RFM69_send() ;
}

void _DMX_RFM69_send(void) {
   build_packet(0); 
   build_packet(1); 
}

void _trigger_detector(int channel) {
  if (buff != DMXSerial.read(channel)) {
    digitalWrite(DEBUGPIN, HIGH) ;
    buff = DMXSerial.read(channel) ;
    delay(10) ;
  }
  else
    digitalWrite(DEBUGPIN, LOW) ;
}

void _channels_updated(void) {
  digitalWrite(DEBUGPIN, HIGH) ;
  delay(10000) ;
  digitalWrite(DEBUGPIN, LOW) ;

}

void build_packet(int packet_id) {
  radiopacket[0] = packet_id_list[packet_id] ;
  for (int i = 1 ; i < PACKET_SIZE ; i++) { // envoit des messages pour chaque récepteur. i est l'adresse du récepteur
    radiopacket[i] = DMXSerial.read(i + packet_id_list[packet_id] - 1) ;
    if (radiopacket[i] == 0 )
      radiopacket[i] = 1 ;
  }
  radio.send(NODERECEIVE, (const void*)radiopacket, strlen(radiopacket), false) ;
  delay(1) ;
}

