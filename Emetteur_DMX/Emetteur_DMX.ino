// Programme permettant la lecture de la tram DMX et envoit des donnÃƒÂ©es par liaison RF a tous les dÃƒÂ©cors

#define DMX

#ifdef DMX
#include <DMXSerial.h>                   // Appel de la librairie SerialDMX
int mode = 1 ; // 1==DMX transmitter ; 2 == Receiver
#else 
int mode = 4 ; // 1==DMX transmitter ; 2 == Receiver ; 3 == direct transmitter ; 4 : Manual DMX Transmitter
#endif

//#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69


#include "DmxEffects.h"
#include "WirelessShow.h"

#define NO_DATA_SINCE 3000
#define SERIAL_BAUD 115200

#define DEBUG


//*********************   DEFINE OUT/IN PINS **************************
#define LED 8 // onboard blinky
#define LEDMX  7
#define DEBUGPIN 3

/**********************************************************************/

#define DMXSERIAL_MAX 512 // max. number of supported DMX data channels
uint8_t  *dmxData;
uint8_t  manData[DMXSERIAL_MAX];
  




uint8_t last_dmx_channels[513] ;
//int packet_id_list[PACKET_AVAILABLE] = {1, 61, 121, 181, 241, 301, 361, 421, 481} ; // liste des premiers canaux de chaque paquet
int indice_packet = 1 ;
unsigned long last_reception = 0 ;
unsigned long package_rcv_delta_t = 0 ; // delta t entre deux rÃƒÂ©ceptions de packet 
unsigned long iterations = 0; 
unsigned long decimation = 100000;
long lastPeriod = -1;




void setup () { // Configuration au dÃƒÂ©marrage
  #ifndef DMX
      Serial.begin(SERIAL_BAUD);
    #endif
  IOinit();
  wireless_init();
  
  switch (mode){
    case 1: 
    #ifdef DMX
      DMX_init();
    #endif
      break;
    default:
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
//      checkRFMReception();
    #endif
      break;
    case 3:
    #ifndef DMX
      //sendFromMonitor();
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
  static int bRVBSequence = false;
  static bool bWipe = false;  
  if (Serial.available() > 0)
  {
    String input = Serial.readString();
    input.trim();

    if (input == "RVB"){   
      Serial.println(input);
      bRVBSequence = true;
    } 
     if (input == "wipe"){   
      Serial.println(input);
      bWipe = true;
    } 
    else {
      bRVBSequence = false;
      bWipe = false;
      if (input == "off"){
        fulloff(manData);
        Serial.println(input);
      }
      else if (input == "on"){
        fullOn(manData);
        Serial.println(input);
      }
      else if (input == "rouge"){
        fullRed(manData);
        //stripRed(manData,1,3);
        Serial.println(input);
      }
      else if (input == "vert"){
        Serial.println(input);
        fullGreen(manData);
      }
      else if (input == "bleu"){
        fullBlue(manData);   
        Serial.println(input);
      }
      else if (input == "address"){
        sendAddress(manData);   
        Serial.println(input);
      }      
      else{
        Serial.println("erreur commande");
      }
        sendPackets(manData);
    }
  }

  if (bRVBSequence) {
    RVBSequence(manData, 500);
    sendPackets(manData);
  }
  if (bWipe) {
    colorWipeRGB(manData, 1, 46, 20);
    //colorWipe(manData, 0, 0, 255, 151, 31, 20);
    sendPackets(manData);
  }
    
}
#endif






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
/*void sendFromMonitor(void){
    //fill in the struct with new values
    theData.packetId = 3;
    theData.packet[0] = 255;
    
    Serial.print("Sending struct (");
    Serial.print(sizeof(theData));
    Serial.print(" bytes) ... ");*/
    /*if (radio.sendWithRetry(NODERECEIVE, (const void*)(&theData), sizeof(theData)))
      Serial.print(" ok!");
    else Serial.print(" nothing...");*/
    /*
    radio.send(NODERECEIVE, (const void*)(&theData), sizeof(theData));
    Serial.println();

 
}*/
#endif


void IOinit(void){
  pinMode(LED, OUTPUT);
  pinMode(LEDMX, OUTPUT);
  pinMode(DEBUGPIN, OUTPUT) ;
}




#ifndef DMX
/*
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
}*/
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





