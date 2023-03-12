// Récepteur étincelles aquatiques pour gérer une bande néopixel avec le protocole DMX via une liaison RFM69
/********* DESCRIPTION **********
   Ce programme permet de configurer un récepteur RF pour contrôler des bandes numériques à LED.
   Un émetteur se charge de lire la trame DMX et d'envoyer une suite de différents canaux pré-sélectionnés décomposés en plusieurs paquets.
   La taille des paquets est définie par l'émetteur et ne peux pas dépasser 60 canaux (limitation du RFM69).
   Tous les récepteurs ont la même adresse et reçoivent l'ensemble des canaux DMX.
   En fonction de l'adresse DMX de départ choisi et du nombre de pixels sur la bande, le programme sélectionne les paquets qu'il doit interpréter.
   Les paramètres à régler sont l'addresse DMX du récepteur (adresse de départ), le nombre de pixels sur la bande utilisée,
   la taille des paquets envoyés par l'émetteur (60 par défaut) et le nombre de pixels controlés par canal (Le programme permet pour l'instant de contrôler 1 pixel par canal).
   A partir des paramètres choisi, le programme calcul l'adresse DMX de fin du récepteur (selon le nombre de pixels).
   Il détermine également les id du premier et du dernier paquets à prendre en compte par le récepteur ainsi que les indices de début et de fin dans ces paquets.
   Une fois ces paramètres déterminés le récepteur peut commencer à écouter en permanence les messages qui lui sont envoyés par l'émetteur.
   L'émetteur émet en mode broadcast donc le récepteur va recevoir tous les paquets, même ceux qui ne le concerne pas.
   Si le paquet reçu le concerne, le récepteur prépare les couleurs des pixels avant des les envoyer sur la bande.
   Il attend d'avoir reçu l'ensemble des canaux de la bande avant de lui envoyer les ordres pour la mettre à jour. Sinon on pourrai voir apparaître les sections des différents paquets.

 ************************************************************/

/************** FONCTIONNEMENT DE LA MACHINE D'ETAT *************
    Une fois le programme initialisée le machine d'état va attendre le premier packet dont il a besoin.
    Une fois reçu, il prépare la couleur des pixels puis il attend le packet suivant jusqu'a recevoir le dernier dont il a besoin.
    Tous les packets ne corespondant pas à celui attendu à un moment donnée sont ignorés. (piste d'amélioration)
    Une fois tous les paquets nécessaires reçu, les nouvelles couleurs sont envoyées à la bande numérique puis le programme va attendre de nouveau le premier paquet.
 *****************************************************************/

/************** *************   INCLUDES LIBRARY ********************************************/
#include "parameters.h"
#include "hardware.h"
#include "wirelessDMX.h"
/*********************************************************************************************/

int mode = 1 ; // 1=receiver ; 2= transmitter; 3 =receiverStruct
//#define DEBUG
//  #define DEBUG_CONFIG
unsigned long previousMillis = 0;  // will store last time LED was updated
// Variables will change:
int ledState = LOW;  // ledState used to set the LED
byte ackCount=0;


#define SERIAL_BAUD 115200

//________________SETUP______________________
void setup() {
  initialisation() ; // initialise the hardware
  find_index() ; // détermine start_index, start_index, start_packet et stop_packet  ;
  state = start_packet ; // initializes the state machine
//#ifdef DEBUG_CONFIG
//  print_config() ; // AFFICHE LES INFOS DU MODULES
//#endif
#ifdef DEBUG
  Serial.print("Wait for packet ") ; Serial.print(state) ; Serial.println("...") ;
#endif
}
//___________________________________________

//______________ LOOP _______________________
void loop() { 
  switch (mode){
    case 1: 
      checkCom();
      traitement();
      break;
      //printReception();
    case 2:
      sendRFMPacket();
      break;
    case 3:
      receiveStruct();
      break;
    case 4:
      stripLEDManual();
      break;
   }

}

void printReception() {
  digitalWrite(RECEPTION, HIGH) ;
  //Serial.flush();
  //Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    
  if (radio.receiveDone())
  {
    //Serial.flush();
    
    //Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    //Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");
    
    delay(100);
    digitalWrite(RECEPTION, LOW) ;
    delay(100);
   // Blink(RECEPTION,100);
   packet_id = radio.DATA[0] ; // the first byte give the packet ID sent
   
   radio.receiveDone(); //put radio in RX mode // voir si nécessaire
   //Serial.println("Hello");
   }
  else {
    Serial.println("Waiting for data...");
    Serial.println(radio._mode);
  
  }
}

void traitement() {  
  digitalWrite(6, 1);
  //check if something was received (could be an interrupt from the radio)
  if (bPacketRcv)
  {
    if (packet_id == PACKET_NBR ) {
      //digitalWrite(LED2, HIGH) ;
    }
    else {
      //digitalWrite(LED2, LOW) ;
    }
  }

  if ((packet_id == state) ) { // check if the packet received is the one we are waiting for

    //Serial.print("packet ") ; Serial.print(packet_id) ; Serial.println(" received") ;
    // start_pixel = (packet_id - 1) * PIXELS_PER_PACKET ;
    

    if ((state == stop_packet) ) { // si le paquet reçu est le dernier paquet exigé
      digitalWrite(LED1, LOW) ;
      prepare_pixel_color1(1, stop_index, packet_id) ;
      pixels.show(); // on met à jour les pîxels de la bande
      state = start_packet ; // on retourne à l'état initial : attendre le premier paquet
      //Serial.print("Wait for packet ") ; Serial.print(state) ; Serial.println("...") ;
      //Serial.println("Strip updated!") ;

    }

    else if (state == start_packet) { // si le paquet reçu est le premier paquet exigé
      digitalWrite(LED1, HIGH) ;
      prepare_pixel_color1(start_index, PACKET_SIZE, packet_id) ;
      state++ ; // on attend le paquet suivant
      //Serial.print("Wait for packet ") ; Serial.print(state) ; Serial.println("...") ;

    }
 

    else {
      prepare_pixel_color1(1, PACKET_SIZE, packet_id) ;
      state++ ; // on attend le paquet suivant
      //Serial.print("Wait for packet ") ; Serial.print(state) ; Serial.println("...") ;
    }
  }
  if (packet_id == 6 ) {
    
    //pixels.show(); // on met à jour les pîxels de la bande
    state = start_packet ; // on retourne à l'état initial : attendre le premier paquet

  }
  // Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU // voir si nécessaire
  if (first_iter == true) 
    first_iter = false;
}


void checkCom(void){
  //check if something was received
  bPacketRcv = radio.receiveDone();
  _noDataSince() ; // display the com status with the LED
  
  if (bPacketRcv)
  { 
    //packet_id = radio.DATA[0] ; // the first byte give the packet ID sent
    //Serial.print("Période réception paquet :") ; Serial.print(package_rcv_delta_t) ; Serial.println(" ms") ;
    last_reception = millis() ;
    //Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    //Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");
  
    if (radio.DATALEN != sizeof(Payload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else
    {
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      packet_id = theData.packetId;
      //Serial.print(" packetId=");
      //Serial.print(packet_id);
      //Serial.print(" first adress=");
      //Serial.print(theData.packet[0]);
    }
    //Serial.println();
    //printDMX();
    radio.receiveDone(); //put back the radio in RX mode
  } 

  



  #ifdef DEBUG
//    Serial.print("Etat trame : ") ; 
//    if (paquet_perdu)
//      Serial.println("NOK") ;
//    else
//      Serial.println("OK") ;   
//    Serial.print("Nbr de paquets perdu :") ; Serial.println(nbr_paquet_perdu) ;
  #endif
}

void printDMX(){
  if (packet_id == 1){
    for (int i = 0;i<10;i++){
      Serial.print(theData.packet[i]); Serial.print(" ");
    }
    Serial.println();
  }
}

void sendRFMPacket(void){
  uint8_t radiopacket[61] ;
  radio.send(TRANSMITTERID, (const void*)radiopacket, strlen(radiopacket), false) ; // envoi du paquet de données
}

void receiveStruct(void){
  typedef struct {
    int           nodeId; //store this nodeId
    unsigned long uptime; //uptime in ms
    float         temp;   //temperature maybe?
  } Payload;
  Payload theData;
  
  if (radio.receiveDone())
  {
    //Blink(LED1,500);
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");
  

    if (radio.DATALEN != sizeof(Payload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else
    {
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      Serial.print(" nodeId=");
      Serial.print(theData.nodeId);
      Serial.print(" uptime=");
      Serial.print(theData.uptime);
      Serial.print(" temp=");
      Serial.print(theData.temp);
    }
    
   
    
    Serial.println();
  }
  else {
    //Serial.println("Nothing");
  }
  
  //Blink(LED2,500);
}

// _________________________________________________________________________

// Cette fonction reconstitue la trame DMX afin de contrôler la bande LED selon l'adressage
void prepare_pixel_color1(int start_indice, int stop_indice, int packet_ID) {
  // Serial.println("prepare_pixel_color1 launched") ;
  //int pixel_offset = ((packet_ID-1)*PACKET_SIZE-DECOR_DMX_ADRESS)/CHANNELS_PER_PIXEL  ;
  int pixel_offset = ((packet_ID - 1) * PACKET_SIZE + 1 - DECOR_DMX_ADRESS) / 3 - 1  ;
  for (int i = start_indice; i < stop_indice  ; i += 3) { // parcours les éléments du tableau reçu
    // Serial.print("i : ") ;   Serial.println(i) ;
    if (radio.DATA[i] == 1 ) {
      radio.DATA[i] = 0 ;
    }
    if (radio.DATA[i + 1] == 1 ) {
      radio.DATA[i + 1] = 0 ;
    }
    if (radio.DATA[i + 2] == 1 ) {
      radio.DATA[i + 2] = 0 ;
    }
    pixels.setPixelColor((i + 2) / 3 + pixel_offset, pixels.Color(radio.DATA[i], radio.DATA[i + 1], radio.DATA[i + 2])); // change the color
#ifdef DEBUG
    //    Serial.print("Pixel:") ; Serial.print((i + 2) / 3 + pixel_offset) ; Serial.print(": ") ;
    //    Serial.print(radio.DATA[i]) ;
    //    Serial.print(" ") ;
    //    Serial.print(radio.DATA[i + 1]) ;
    //    Serial.print(" ") ;
    //    Serial.print(radio.DATA[i + 2]) ;
    //    Serial.println(" ") ;
#endif
  }
}



void find_index() {
  int packet_index = 1 ;

  while ( packet_index < PACKET_ID_MAX) { // on parcourt les packets pour trouver les indices de départ et d'arriver
    int channel_inf = (packet_index - 1) * PACKET_SIZE + 1 ;
    int channel_sup = packet_index * PACKET_SIZE ;
    if ( (channel_inf <= DECOR_DMX_ADRESS)   & (DECOR_DMX_ADRESS <= channel_sup)  ) {
      start_index = DECOR_DMX_ADRESS - ((packet_index - 1) * PACKET_SIZE) ;
      start_packet = packet_index ;
    }
    if (  (channel_inf <= LAST_DMX_ADRESS)   & (LAST_DMX_ADRESS <= channel_sup)  ) {
      stop_index = LAST_DMX_ADRESS - ((packet_index - 1) * PACKET_SIZE) ;
      stop_packet = packet_index ;
      break ;
    }
    packet_index++ ;
  }
}

void IOinit(void){
  //**************** CONFIG DES ENTREES/SORTIES *************
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(RECEPTION, OUTPUT);
  pinMode(T1, OUTPUT) ;
  pinMode(T2, OUTPUT ) ;
  //******************************************************************
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
  //*****************************************************************
}

void stripLed_init(){
   //************* NEOPIXEL SETTINGS *********************************
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin(); // This initializes the NeoPixel library.
  //*****************************************************************
  black_strip() ;
}

void initialisation(void) {

  IOinit();
  digitalWrite(LED1, HIGH) ; // set led high to show that the setup has started
  wireless_init();
  stripLed_init();
  // init serial port for debugging
  //#ifdef DEBUG || DEBUG_CONFIG
  Serial.begin(SERIAL_BAUD);
  //#endif
  digitalWrite(LED1, LOW) ; // shows that the setup is finished
}

void print_config(void) {
  Serial.print("Recepteur strip led ");

  Serial.println("\nNetwork : ") ;
  Serial.print("Node: ") ; Serial.println(NODEID) ;
  Serial.print("NETWORKID: ") ; Serial.println(NETWORKID) ;
  Serial.print("PACKET_SIZE: ") ; Serial.println(PACKET_SIZE) ;
  Serial.print("DECOR_DMX_ADRESS: ") ; Serial.println(DECOR_DMX_ADRESS) ;
  Serial.print("LAST_DMX_ADRESS: ") ; Serial.println(LAST_DMX_ADRESS) ;


  Serial.println("\nDecor parameters : ") ;
  Serial.print("NUMPIXELS: ") ; Serial.println(NUMPIXELS) ;
  Serial.print("PIX_PER_GROUP: ") ; Serial.println(PIX_PER_GROUP) ;
  Serial.print("CHANNELS_PER_PIXEL: ") ; Serial.print(CHANNELS_PER_PIXEL) ;  Serial.println("(RVB)") ;
  Serial.print("CHANNELS_NBR : ") ; Serial.println(CHANNELS_NBR) ;
  //  Serial.print("PIXELS_PER_PACKET : ") ; Serial.println(PIXELS_PER_PACKET) ;
  //  Serial.print("NUM_PACKET : ") ; Serial.println(NUM_PACKET) ;
  //  Serial.print("CHANNELS_REST : ") ; Serial.println(CHANNELS_REST) ;
  //  Serial.print("PIXELS_IN_LAST_PACKET : ") ; Serial.println(PIXELS_IN_LAST_PACKET) ;
  Serial.print("start_packet : ") ; Serial.println(start_packet) ;
  Serial.print("start_index : ") ; Serial.println(start_index) ;
  Serial.print("stop_packet : ") ; Serial.println(stop_packet) ;
  Serial.print("stop_index : ") ; Serial.println(stop_index) ; Serial.print("\n") ;
}

void black_strip(void) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, 0, 0, 0);
    pixels.show(); // on met à jour les pîxels de la bande
  }
}

bool _noDataSince() {
  package_rcv_delta_t = millis() - last_reception;  
  if (package_rcv_delta_t > NO_DATA_SINCE) {
    digitalWrite(RECEPTION, LOW) ;
    return false ;
  }
  else
    digitalWrite(RECEPTION, HIGH) ;
  return true ;
}

void check_paquet_perdu(){
  if (first_iter == false){ 
    if ((packet_id == 1 & last_packet_id == PACKET_NBR) || (packet_id ==(last_packet_id +1))){
      paquet_perdu = false;
    }
    else 
      paquet_perdu = true;   
      nbr_paquet_perdu = nbr_paquet_perdu + 1;
  }
  last_packet_id = packet_id;
}

void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
}

void BlinkNoDelay(byte ledPin, int DELAY_MS)
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= DELAY_MS) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    //Serial.print("hello");
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}

void stripLEDManual(void){
  if (Serial.available() > 0)
  {
    String input = Serial.readString();
    input.trim();
    if (input == "off")
      black_strip();
    else if (input == "rouge")
      fullRed();
    else if (input == "vert")
      fullGreen();
    else if (input == "blue")
      fullBlue();
    
      
  }
}

void fullRed() {  
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
        pixels.setPixelColor(i, pixels.Color(255,0,0 ) );
    }
    pixels.show();
}

void fullGreen() {  
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
        pixels.setPixelColor(i, pixels.Color(0,0,255 ) );
    }
    pixels.show();
}

void fullBlue() {  
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
        pixels.setPixelColor(i, pixels.Color(0,255,0 ) );
    }
    pixels.show();
}

