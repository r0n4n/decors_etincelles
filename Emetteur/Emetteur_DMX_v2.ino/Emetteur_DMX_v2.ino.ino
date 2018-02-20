// Programme émetteur pour faire l'essai de portée en envoyant on et OFF simultanément


#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 // The same on all nodes that talk to each other
#define NODEID 2 // The unique identifier of this node
#define RECEIVER 1 // The recipient of packets

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


#define LedDMX  5 

#define BP 7 // pin du bouton poussoir 

#define num 0

const int CANAL[6] = {1, 2, 3, 4, 5, 6}; // Définit un tableau pour les canaux d'adressage de sortie


RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

void setup() {
  DMXSerial.init(DMXReceiver);          // Initialise la carte comme un récepteur DMX (la librairie permettant aussi d'émmetre)emmetre


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
  pinMode(BP, INPUT);
  //Serial.print("\nTransmitting at ");
  Serial.print(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");
}

void loop() {
  char radiopacket[5] ; 
  radiopacket[0] = '0' ; 
  
  if (digitalRead(BP) == 1) {
    Serial.print("on") ; 
    radiopacket[num] = '0';
  }

  else if (digitalRead(BP) == 0) {
    radiopacket[num] = '1';
  }

 /* char radiopacket[5] ;   //"hello"; 
  radiopacket[0] = '0' ; // 48 en décimal 
  radiopacket[1] = '1'  ; // 49 en décimal
  radiopacket[2] = '2' ; // 50 en décimal */
  //Serial.println(radiopacket[num]) ;   
  //Serial.print("Sending "); Serial.println((int)radiopacket[num]);
  //if (radio.sendWithRetry(RECEIVER, radiopacket, strlen(radiopacket))) { //target node Id, message as string or byte array, message length
  radio.send(RECEIVER, radiopacket, strlen(radiopacket), false) ;
 //Serial.println("OK");

 // }

  radio.receiveDone(); //put radio in RX mode
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU

}

void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i = 0; i < loops; i++)
  {
    digitalWrite(PIN, HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN, LOW);
    delay(DELAY_MS);
  }
}
