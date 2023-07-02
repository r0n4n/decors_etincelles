#include "wirelessDMX.h"

#define RECEPTION 13
#define SERIAL_BAUD 115200
 

void setup() {
  // put your setup code here, to run once:
  wireless_init();
  pinMode(RECEPTION, OUTPUT);
  Serial.begin(SERIAL_BAUD);
}

void loop() {
  // put your main code here, to run repeatedly:
  listenRadio();
  
  printReception();
}


void wireless_init(void){
  //************************ RFM69 INIT ******************************
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
  radio.encrypt(ENCRYPTKEY); // set the ENCRYPTKEY
  radio.promiscuous(true);
  //*****************************************************************
}

void listenRadio(void){
  //check if something was received
  bPacketRcv = radio.receiveDone();
  _noDataSince() ; // display the com status with the LED
  
  if (bPacketRcv)
  { 
    Blink(RECEPTION, 100);
    last_reception = millis() ;
    if (radio.DATALEN == sizeof(Payload) && radio.TARGETID == BROADCASTID)
    {
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      packet_id = theData.packetId;
      broadcast_RSSI = radio.readRSSI();
      checkCom();
    }
    radio.receiveDone(); //put back the radio in RX mode
  } 
}

bool _noDataSince() {
  package_rcv_delta_t = millis() - last_reception; 
  debit = 1000/float(package_rcv_delta_t); 
  if (package_rcv_delta_t > NO_DATA_SINCE) {
    digitalWrite(RECEPTION, LOW) ;
    trameCntOk = 0;
    broadcast_RSSI = 0;
    return false ;
  }
  else
    digitalWrite(RECEPTION, HIGH) ;
  return true ;
}

void printReception() { 
static unsigned int  counter = 0;
static unsigned int print_decimation = 100000;
if (counter>=print_decimation){
    //Serial.print("Broadcast RSSI: ");Serial.println(broadcast_RSSI);
    Serial.print("Qualité comm :") ; Serial.print(trameCntOk) ; Serial.println("/10") ;
    //Serial.print("Période :") ; Serial.println(package_rcv_delta_t) ;
    //Serial.print("Débit :") ; Serial.println(debit) ;

    Serial.print("Buff packet ID : ") ; 
    for (int idx=0;idx<PACKET_NBR ;idx++){
      Serial.print(packetIdBuff[idx]);
    }
    Serial.println("") ;

    counter = 0;
  }
  else 
    counter++;
}

void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
}

void checkCom(){
  static int packetCounter = 0;
  static int trameCnt = 0;
  static int trameCntOkTmp = 0;
  bool paquet_perdu = false; // True si un paquet n'a pas été reçu, false sinon
  #define TRAMESNBRMAX 10
  
  if (trameCnt>=TRAMESNBRMAX){
    trameCntOk = trameCntOkTmp;
    trameCntOkTmp = 0;
    trameCnt = 0;
    
  }

  if (first_iter == false){ 
    if (packet_id <last_packet_id){ // une nouvelle trame arrive
      paquet_perdu = false;
      trameCnt++;
      packetCounter = 0;
    }
      
    if (paquet_perdu ==false && packet_id>1 && packet_id<=PACKET_NBR && last_packet_id!=(packet_id-1)){ // si un paquet est perdu
      paquet_perdu = true;
    }

    if (packet_id==PACKET_NBR && !paquet_perdu){ // si à la fin de la trame aucun paquet n'a été perdu 
      trameCntOkTmp++;
    }

  }
  last_packet_id = packet_id;

  if (packetCounter<10){
      packetIdBuff[packetCounter] = packet_id;
      packetCounter++;
  }
  else 
      packetCounter = 0;
}