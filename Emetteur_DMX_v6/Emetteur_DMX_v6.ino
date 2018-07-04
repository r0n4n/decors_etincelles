// Programme permettant la lecture de la tram DMX et envoit des données par liaison RF a tous les décors

#include <DMXSerial.h>                   // Appel de la librairie SerialDMX
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69

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
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
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

/****************** DEFINE LIST ***************************/
#define PACKET_SIZE 60 // number of channels to send per packet
#define PACKET_SIZE_PLUS_ID  PACKET_SIZE+1 // total size of a packet with the ID  
#define PACKET_NBR 8 // nombre paquet que l'on souhaite envoyer 
#define PACKET_AVAILABLE 8 // nombre de paquets qui décomposent l'ensemble des canaux DMX 
/*************************************************************/

uint8_t radiopacket[PACKET_SIZE_PLUS_ID] ;
uint8_t last_dmx_channels[513] ;
//int packet_id_list[PACKET_AVAILABLE] = {1, 61, 121, 181, 241, 301, 361, 421, 481} ; // liste des premiers canaux de chaque paquet
int indice_packet = 1 ;

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

void setup () { // Configuration au démarrage
  DMXSerial.init(DMXReceiver);          // Initialise la carte comme un récepteur DMX

  pinMode(LED, OUTPUT);
  pinMode(LEDMX, OUTPUT);
  pinMode(DEBUGPIN, OUTPUT) ;

  digitalWrite(LEDMX,HIGH) ;
//  digitalWrite(LED,HIGH) ;  

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
  
  DMXSerial.attachOnUpdate(_DMX_RFM69_send) ; // Run the _DMX_RFM69_send function each time that a new DMX packet is received
  //DMXSerial.attachOnUpdate(debug_channels_change) ; 
  
}


void loop () { // Boucle du programme principal
  // vérifie si des données ont été reçues via la liaison DMX
  if (DMXSerial.noDataSince() > 5)      // LED Reception du signal
    digitalWrite(LEDMX, LOW);            // Si le signal n'a pas été reçu depuis + de 1 ms, la LED s'éteint
  else
    digitalWrite(LEDMX, HIGH);
  //_DMX_RFM69_send() ;
  //debug_channels_change() ; 
}

/********* Envoi les paquets l'un après l'autre *******/
void _DMX_RFM69_send(void) {
   // digitalWrite(LED,HIGH) ; 
   if (dmx_change()) {
    digitalWrite(LED,HIGH) ; 
    for (int i = 1 ; i <=PACKET_NBR ; i++){
      build_packet(i); // construction et envoi du paquet i   
      radio.send(NODERECEIVE, (const void*)radiopacket, strlen(radiopacket), false) ; // envoi du paquet de données
    }
   }
   else
    digitalWrite(LED,LOW) ; 
    
   // digitalWrite(LED,LOW) ;
}

/*
 * Construit un paquet de plusieurs canaux DMX selon le packet_id données en paramètre. 
 * @param int packet_id : l'identifiant du paquet à envoyer  
 * @return None. 
 */
void build_packet(int packet_id) {
  radiopacket[0] = packet_id ; // L'identifiant du paquet est le premier byte du paquet à envoyer
  int channel_offset = (packet_id - 1) * PACKET_SIZE ; // calcul de l'offset du canal DMX selon le packet_id
  for (int i = 1 ; i < (PACKET_SIZE + 1) ; i++) { // construction du paquet à envoyer avec les canaux DMX
    radiopacket[i] = DMXSerial.read(i + channel_offset) ;
    delete_zeros(i) ; 
  }
}

/*
 * delete_zeros regarde si la valeur du canal donnée en paramètre est nulle. Si oui elle la chnage en 1. Cette function est utilisée pour permettre l'envoi des données via    
 * le module RFM69. Le module ne prends pas les valeurs nulles.
 * @param int i : le canal concerné 
 * @return None. 
 */
void delete_zeros(int i){
   if (radiopacket[i] == 0 )
      radiopacket[i] = 1 ; // rejet des zeros sur les canaux DMX pour éviter les erreurs de transmission sur la liason RFM69
}

/*
 * dmx_change compare la trame DMX qui vient d'être reçu avec la précedente pour voir si les valeurs ont changées.  
 * @param none.
 * @return un boolean. True si la trame a changée. False sinon.  
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

/********************************DEBUG FUNCTION ***********************/


/** cette function fait varier la luminosité d'une led en fonction de la valeur du canal canal choisi.
* peut être utile pour vérifier la bonne lecture de la trame DMX **/
void debug_channels_change(void) {  
  analogWrite(LED, DMXSerial.read(1)) ;
}
/************************************************************************/
