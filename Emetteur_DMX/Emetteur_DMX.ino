// Programme permettant la lecture de la tram DMX et envoit des donnÃƒÂ©es par liaison RF a tous les dÃƒÂ©cors

#define DMX

#ifdef DMX
#include <DMXSerial.h>                   // Appel de la librairie SerialDMX
int mode = 1 ; // 1==DMX transmitter ; 2 == Receiver
#else 
int mode = 4 ; // 1==DMX transmitter ; 2 == Receiver ; 3 == direct transmitter ; 4 : Manual DMX Transmitter
#endif

#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69

#include "parameters.h"

#define DEBUG


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
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module

#define RFM69_CS 10 // Chip select
#define RFM69_IRQ 2 // interruption 
#define RFM69_IRQN 0 // Pin 2 is IRQ 0! 
#define RFM69_RST 9 // Reset 

//*********************   DEFINE OUT/IN PINS **************************
#define LED 8 // onboard blinky
#define LEDMX  7
#define DEBUGPIN 3

/**********************************************************************/

#define DMXSERIAL_MAX 512 // max. number of supported DMX data channels
uint8_t  *dmxData;
uint8_t  manData[DMXSERIAL_MAX];
  

/****************** DEFINE LIST ***************************/
#define PACKET_SIZE 60 // number of channels to send per packet
#define PACKET_SIZE_PLUS_ID  PACKET_SIZE+1 // total size of a packet with the ID  
#define PACKET_NBR 8 // nombre paquet que l'on souhaite envoyer 
#define PACKET_AVAILABLE 8 // nombre de paquets qui dÃƒÂ©composent l'ensemble des canaux DMX 
#define NO_DATA_SINCE 3000
#define SERIAL_BAUD 115200
/*************************************************************/


uint8_t last_dmx_channels[513] ;
//int packet_id_list[PACKET_AVAILABLE] = {1, 61, 121, 181, 241, 301, 361, 421, 481} ; // liste des premiers canaux de chaque paquet
int indice_packet = 1 ;
unsigned long last_reception = 0 ;
unsigned long package_rcv_delta_t = 0 ; // delta t entre deux rÃƒÂ©ceptions de packet 
unsigned long iterations = 0; 
unsigned long decimation = 100000;
long lastPeriod = -1;

typedef struct {
  uint8_t           packetId; //store this nodeId
  uint8_t packet[PACKET_SIZE];
} Payload;
Payload theData;


RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

void setup () { // Configuration au dÃƒÂ©marrage
  IOinit();
  wireless_init();
  
  switch (mode){
    case 1: 
    #ifdef DMX
      DMX_init();
    #endif
      break;
    default:
    #ifndef DMX
      Serial.begin(SERIAL_BAUD);
    #endif
      break;
  }
}


void loop () { // Boucle du programme principal
  //_DMX_RFM69_send() ;
  //debug_channels_change() ;
  switch (mode){
    case 1: 
    #ifdef DMX
      checkDMXCom(); 
    #endif
      break;
    case 2:
    #ifndef DMX
      checkRFMReception();
    #endif
      break;
    case 3:
    #ifndef DMX
      sendFromMonitor();
    #endif
      break;
    case 4:
    #ifndef DMX
      sendManualTram();
    #endif
      break;
  }
  //radio.send(NODERECEIVE, (const void*)radiopacket, strlen(radiopacket), false) ; // envoi du paquet de donnÃƒÂ©es
}

#ifndef DMX
void sendManualTram(){

  if (Serial.available() > 0)
  {
    String input = Serial.readString();
    input.trim();
    if (input == "off"){
      fulloff();
      Serial.println(input);
    }
    else if (input == "rouge"){
      //fullRed();
      Serial.println(input);
    }
    else if (input == "vert"){
      //fullGreen();
      Serial.println(input);
    }
    else if (input == "bleu"){
      //fullBlue();   
      Serial.println(input);
    }
    else
      Serial.println("erreur commande");
  }
  sendPackets(manData);
}
#endif

void fulloff(){
  for (int i = 0; i<DMXSERIAL_MAX;i++){
    manData[i] = 0;
  }
}

void fullOn(){
  for (int i = 0; i<DMXSERIAL_MAX;i++){
    manData[i] = 255;
  }
}

void fullRed(){
  manData[0] = 255;
    manData[1] = 0;
    manData[2] = 0;
  
  /*for (int i = 0; i<12;i+3){
    manData[i] = 255;
    manData[i+1] = 0;
    manData[i+2] = 0;
  }*/
}

void fullGreen(){
  for (int i = 0; i<DMXSERIAL_MAX;i+3){
    manData[i] = 0;
    manData[i+1] = 255;
    manData[i+2] = 0;
  }
}

void fullBlue(){
  for (int i = 0; i<DMXSERIAL_MAX;i+3){
    manData[i] = 0;
    manData[i+1] = 0;
    manData[i+2] = 255;
  }
}

void sendPackets(uint8_t  *trame){
  uint8_t radiopacket[PACKET_SIZE_PLUS_ID] ;
  
  for (int i = 1 ; i <=PACKET_NBR ; i++){
      build_packet(i, trame, radiopacket); // construction et envoi du paquet i   
      radio.send(NODERECEIVE, (const void*)radiopacket, strlen(radiopacket), false) ; // envoi du paquet de donnÃƒÂ©es
    }
}

/*
 * Construit un paquet de plusieurs canaux DMX selon le packet_id donnÃƒÂ©es en paramÃƒÂ¨tre. 
 * @param int packet_id : l'identifiant du paquet ÃƒÂ  envoyer  
 * @return None. 
 */
void build_packet(int packet_id, uint8_t  *trame , uint8_t  *packet ) {
  packet[0] = packet_id ; // L'identifiant du paquet est le premier byte du paquet ÃƒÂ  envoyer
  int channel_offset = (packet_id - 1) * PACKET_SIZE ; // calcul de l'offset du canal DMX selon le packet_id

  for (int i = 1 ; i < (PACKET_SIZE + 1) ; i++) { // construction du paquet ÃƒÂ  envoyer avec les canaux DMX
    packet[i] = trame[i + channel_offset] ;
    delete_zeros(packet, i) ; 
  }
}

/*
 * delete_zeros regarde si la valeur du canal donnÃƒÂ©e en paramÃƒÂ¨tre est nulle. Si oui elle la chnage en 1. Cette function est utilisÃƒÂ©e pour permettre l'envoi des donnÃƒÂ©es via    
 * le module RFM69. Le module ne prends pas les valeurs nulles.
 * @param int i : le canal concernÃƒÂ© 
 * @return None. 
 */
void delete_zeros(uint8_t *packet, int i){
   if (packet[i] == 0 )
      packet[i] = 1 ; // rejet des zeros sur les canaux DMX pour ÃƒÂ©viter les erreurs de transmission sur la liason RFM69
}


/********* Envoi les paquets l'un aprÃƒÂ¨s l'autre *******/
#ifdef DMX
void _DMX_RFM69_send(void) {
   // digitalWrite(LED,HIGH) ; 
   if (dmx_change()) {
    digitalWrite(LED,HIGH) ; 
    
   }
   else
    digitalWrite(LED,LOW) ; 
    sendPackets(dmxData);
   // digitalWrite(LED,LOW) ;
}

/*
 * dmx_change compare la trame DMX qui vient d'ÃƒÂªtre reÃƒÂ§u avec la prÃƒÂ©cedente pour voir si les valeurs ont changÃƒÂ©es.  
 * @param none.
 * @return un boolean. True si la trame a changÃƒÂ©e. False sinon.  
 */
bool dmx_change(){
  bool change = false ; 
  for (int i=1; i<=512; i++) {
    if (last_dmx_channels[i] !=  DMXSerial.read(i))
      change = true ; 
    last_dmx_channels[i] =  DMXSerial.read(i) ; 
  }
  return change ; 
}

void checkDMXCom(void){
  // vÃƒÂ©rifie si des donnÃƒÂ©es ont ÃƒÂ©tÃƒÂ© reÃƒÂ§ues via la liaison DMX
  if (DMXSerial.noDataSince() > 5)      // LED Reception du signal
    digitalWrite(LEDMX, LOW);            // Si le signal n'a pas ÃƒÂ©tÃƒÂ© reÃƒÂ§u depuis + de 1 ms, la LED s'ÃƒÂ©teint
  else
    digitalWrite(LEDMX, HIGH);
}

void DMX_init(void){
  DMXSerial.init(DMXReceiver);          // Initialise la carte comme un rÃƒÂ©cepteur DMX
  DMXSerial.attachOnUpdate(_DMX_RFM69_send) ; // Run the _DMX_RFM69_send function each time that a new DMX packet is received
  dmxData = DMXSerial.getBuffer();
  //DMXSerial.attachOnUpdate(debug_channels_change) ; 
}

/********************************DEBUG FUNCTION ***********************/


/** cette function fait varier la luminositÃƒÂ© d'une led en fonction de la valeur du canal canal choisi.
* peut ÃƒÂªtre utile pour vÃƒÂ©rifier la bonne lecture de la trame DMX **/
void debug_channels_change(void) {  
  analogWrite(LED, DMXSerial.read(1)) ;
}
/************************************************************************/
#endif

#ifndef DMX
void sendFromMonitor(void){
    //fill in the struct with new values
    theData.packetId = 3;
    theData.packet[0] = 255;
    
    Serial.print("Sending struct (");
    Serial.print(sizeof(theData));
    Serial.print(" bytes) ... ");
    /*if (radio.sendWithRetry(NODERECEIVE, (const void*)(&theData), sizeof(theData)))
      Serial.print(" ok!");
    else Serial.print(" nothing...");*/
    
    radio.send(NODERECEIVE, (const void*)(&theData), sizeof(theData));
    Serial.println();

 
}
#endif


void IOinit(void){
  pinMode(LED, OUTPUT);
  pinMode(LEDMX, OUTPUT);
  pinMode(DEBUGPIN, OUTPUT) ;
}

void wireless_init(void){
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
}


#ifndef DMX
void checkRFMReception(){
  if (radio.receiveDone())
  {     
    last_reception = millis() ;
    radio.receiveDone(); //put back the radio in RX mode
  }
  _noDataSince();
  
  if (iterations>=decimation){
    iterations=0;
    //Serial.println("Hello");
    //Serial.print('SENDERID: ');Serial.println(radio.SENDERID, DEC);
    //Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.println("]");
    //Serial.print("PÃƒÂ©riode rÃƒÂ©ception paquet :") ; Serial.print(package_rcv_delta_t) ; Serial.println(" ms") ;
  }
  else {
    //Serial.println(iterations);
    iterations= iterations + 1;
  }  
}
#endif

void _noDataSince() {
  package_rcv_delta_t = millis() - last_reception;  
  if (package_rcv_delta_t > NO_DATA_SINCE) {
    digitalWrite(LEDMX, LOW) ;
  }
  else
    digitalWrite(LEDMX, HIGH) ;
}

void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
}





