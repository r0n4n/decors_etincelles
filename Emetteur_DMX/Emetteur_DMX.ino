// Programme permettant la lecture de la tram DMX et envoit des donnÃƒÂ©es par liaison RF a tous les dÃƒÂ©cors
#include "DmxEffects.h"
#include "WirelessShow.h"

//#define DMX

#ifdef DMX
#include <DMXSerial.h>                   // Appel de la librairie SerialDMX
int mode = 1 ; // 1==DMX transmitter ; 2 == Receiver
#else 
int mode = 4 ; // 1==DMX transmitter ; 2 == Receiver ; 3 == direct transmitter ; 4 : Manual DMX Transmitter
#endif

#define SERIAL_BAUD 115200

#define DEBUG

//*********************   DEFINE OUT/IN PINS **************************/
#define LED 8 // onboard blinky
#define LEDMX  7
#define DEBUGPIN 3
/**********************************************************************/

//*********************   DMX Variables *************************************
#define DMXSERIAL_MAX 512 // max. number of supported DMX data channels
uint8_t  *dmxData;
uint8_t  manData[DMXSERIAL_MAX];
uint8_t last_dmx_channels[513] ;
/**********************************************************************/


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
}

#ifndef DMX
void sendManualTram(){
  static int bRVBSequence = false; 
  if (Serial.available() > 0)
  {
    String input = Serial.readString();
    input.trim();

    if (input == "RVB"){   
      Serial.println(input);
      bRVBSequence = true;
    } 
    else {
      bRVBSequence = false;
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
     
      else
        Serial.println("erreur commande");
        sendPackets(manData);
    }
  }

  if (bRVBSequence) {
    RVBSequence(manData, 1000);
    sendPackets(manData);
  }
    
}
#endif


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


void IOinit(void){
  pinMode(LED, OUTPUT);
  pinMode(LEDMX, OUTPUT);
  pinMode(DEBUGPIN, OUTPUT) ;
}


void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
}

