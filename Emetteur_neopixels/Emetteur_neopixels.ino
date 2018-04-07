// Programme envoit des données par liaison RF pour controler des leds néopixel

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
#define CHANNELSNUMBER  120
#define PIXELNBR (CHANNELSNUMBER/3) 

uint8_t radiopacket[61] ;
uint8_t DMX_channels[CHANNELSNUMBER] ; // tableau simulant la valeur des canaux DMX reçus
uint8_t red_channels[CHANNELSNUMBER] ;
uint8_t green_channels[CHANNELSNUMBER] ;
uint8_t blue_channels[CHANNELSNUMBER] ;

 int mode = 1 ; 

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

void setup () { // Configuration au démarrage

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

  rouge() ; // init the DMX channel values to red pixels 
  vert() ; 
  bleu() ; 
  Serial.begin(115200) ; 
 
}


void loop () { // Boucle du programme principal

  switch (mode) {
    case 1:
      for (int i=0;i<90;i++) {
        DMX_channels[i] = green_channels[i] ;   
      }
      Serial.print("case: ") ; Serial.println(mode) ; 
      break ; 
    case 2:
      for (int i=0;i<90;i++) {
        DMX_channels[i] = red_channels[i] ;   
      }
      Serial.print("case: ") ; Serial.println(mode) ; 
      break ; 
    
    case 3:
      for (int i=0;i<90;i++) {
        DMX_channels[i] = red_channels[i] ;   
      }
      Serial.print("case: ") ; Serial.println(mode) ; 
      break ;
    default: 
      mode = 0 ; 
      break ;
  } 
   mode= mode +1  ; 
  
  
  digitalWrite(LedDMX, HIGH);
  
  // envoit les 60 premiers canaux DMX
  radiopacket[0] = 1 ;
  for (int i = 1; i < 61 ; i++) { // envoit des messages pour chaque récepteur. i est l'adresse du récepteur
    
    radiopacket[i] = DMX_channels[i-1] ;
    Serial.print(i) ; Serial.print(": ") ;  Serial.println(radiopacket[i]) ;
    /*if (radiopacket[i] == 0 )
      radiopacket[i] = 1 ;*/
  }
  radio.send(NODERECEIVE, (const void*)radiopacket, strlen(radiopacket), false) ;

  //envoit les canaux DMX de 61 à 120
  radiopacket[0] = 61 ;
  for (int i = 61; i < 121 ; i++) { // envoit des messages pour chaque récepteur. i est l'adresse du récepteur
    radiopacket[i - 60] = DMX_channels[i-1] ;
    if (radiopacket[i - 60] == 0 )
      radiopacket[i - 60] = 1 ;
  }
  radio.send(NODERECEIVE, (const void*)radiopacket, strlen(radiopacket), false) ;
  
  digitalWrite(LedDMX, LOW);
  delay(500) ; 
}

void bleu() {
  for (int i=0;i<PIXELNBR;i++) {
    blue_channels[i*3] = 200 ; // set blue value 
    blue_channels[i*3+1] = 1 ; // set red value 
    blue_channels[i*3+2] = 1 ; //set green value
  }
}

void rouge() {
  for (int i=0;i<PIXELNBR;i++) {
    red_channels[i*3] = 1 ; // set blue value 
    red_channels[i*3+1] = 200 ; // set red value 
    red_channels[i*3+2] = 1 ; //set green value
  }
}

void vert() {
  for (int i=0;i<PIXELNBR;i++) {
    green_channels[i*3] = 1 ; // set blue value 
    green_channels[i*3+1] = 1 ; // set red value 
    green_channels[i*3+2] = 200 ; //set green value
  }
}
