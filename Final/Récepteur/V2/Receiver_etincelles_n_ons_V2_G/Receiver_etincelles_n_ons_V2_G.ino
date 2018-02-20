


// Récepteur étincelles aquatiques
// Le récepteur est configurable avec une adresse unique
// version avec mise en veille au bout d'un certain temps 
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <avr/sleep.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 //the same on all nodes that talk to each other
#define NODEID 2

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY RF69_433MHZ
//#define FREQUENCY RF69_868MHZ
//#define FREQUENCY RF69_915MHZ
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

unsigned long time;

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

//________________SETUP______________________

void setup() {
  Serial.begin(SERIAL_BAUD);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
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
}

void loop() {
  //check if something was received (could be an interrupt from the radio)
  time = millis();
  
  //prints time since program started
  Serial.println(time);
  if (time == 600000 ) {
    //radio.setMode(RF69_MODE_SLEEP) ; 
    radio.setMode(RF69_MODE_SLEEP) ; 
    sleep_mode();
  }
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
      digitalWrite(bas, HIGH);
      digitalWrite(LED_bas, HIGH) ;
    }
    if (state_bas == 48) {
      Serial.println("  state_bas off") ;
      digitalWrite(bas, LOW);
      digitalWrite(LED_bas, LOW) ;
    }

    if (state_haut == 49) {
      Serial.println("  state_haut on") ;
      digitalWrite(haut, HIGH);
      digitalWrite(LED_haut, HIGH ) ;
    }
    if (state_haut == 48) {
      Serial.println("  state_haut off") ;
      digitalWrite(haut, LOW);
      digitalWrite(LED_haut, LOW) ;
    }
  }
  digitalWrite(reception, LOW) ;
  radio.receiveDone(); //put radio in RX mode
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
}

