// Récepteur étincelles aquatiques pour le décor à LEDS
// Le récepteur est configurable avec une adresse unique
// VERIFIER QUE L'ADRESSE N'EST PAS DEJA UTILISEE

#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 //the same on all nodes that talk to each other
#define NODEID 5

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

#define decor1 4 // le décor1 est commandé sur la pin 4 
#define LED 7 // cette LED montre l'état du décor : si la LEd est allumé le décor doit aussi être allumé 
#define reception 8 // le led clignote dès que le récepteur reçoit un message 

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

void setup() {
  //while (!Serial); // wait until serial console is open, remove if not tethered to computer
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
  pinMode(LED, OUTPUT);
  pinMode(decor1, OUTPUT) ;
  pinMode(reception, OUTPUT ) ;
  digitalWrite(decor1, LOW);
  digitalWrite(LED, HIGH);

  Serial.print("\nListening at ");
  Serial.print(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");
}

void loop() {
  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    digitalWrite(reception, HIGH) ;
    int message = (int)radio.DATA[0] ;
    //print message received to serial
    Serial.print('['); Serial.print(radio.SENDERID); Serial.print("] ");
    Serial.println(message);
    Serial.print(" [RX_RSSI:"); Serial.print(radio.RSSI); Serial.println("]");
    //check if received message contains Hello World

    //check if sender wanted an ACK
    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.println(" - ACK sent");
    }

    if (message == 49) {
      Serial.println("  state_bas on") ;
      digitalWrite(LED, HIGH) ;
      digitalWrite(decor1, HIGH) ;
    }
    else if (message == 48) {
      Serial.println("  state_bas off") ;
      digitalWrite(LED, LOW) ;
      digitalWrite(decor1, LOW) ;
    }
  }
  digitalWrite(reception, LOW) ;
  radio.receiveDone(); //put radio in RX mode
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
}

