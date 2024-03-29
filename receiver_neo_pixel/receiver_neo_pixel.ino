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
#define AUTOMODE 1;
#define REMOTEMANUALMODE 2;
#define LOCALMANUALMODE 3;
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
  listenRadio();
  
  switch (mode){
    case 1: 
      //printReception();
      traitement();
      break;
    case 2:
      remoteManual();
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
static unsigned int  counter = 0;
static unsigned int print_decimation = 100000;
if (counter>=print_decimation){
    Serial.print("Broadcast RSSI: ");Serial.println(broadcast_RSSI);
    Serial.print("Qualité comm :") ; Serial.print(trameCntOk) ; Serial.println("/10") ;
    //Serial.print("Période :") ; Serial.println(package_rcv_delta_t) ;
    //Serial.print("Débit :") ; Serial.println(debit) ;

    /*Serial.print("Buff packet ID : ") ; 
    for (int idx=0;idx<PACKET_NBR ;idx++){
      Serial.print(packetIdBuff[idx]);
    }
    Serial.println("") ;*/
    


    counter = 0;
  }
  else 
    counter++;
}

void traitement() {  
  digitalWrite(6, 1);
  //check if something was received 
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
  /*if (packet_id == 6 ) {
    
    //pixels.show(); // on met à jour les pîxels de la bande
    state = start_packet ; // on retourne à l'état initial : attendre le premier paquet

  }*/
  // Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU // voir si nécessaire
  if (first_iter == true) 
    first_iter = false;
}


void listenRadio(void){
  //check if something was received
  bPacketRcv = radio.receiveDone();
  _noDataSince() ; // display the com status with the LED
  
  if (bPacketRcv)
  { 
    last_reception = millis() ;
    if (radio.DATALEN == sizeof(diagBuff) && radio.SENDERID == TESTEURID && radio.TARGETID == NODEID)
    {    
      //Serial.println("Message testeur reçu");
      diagBuff = *(DiagBuff*)radio.DATA;
      //Serial.println(diagBuff.diagCode);
      if (diagBuff.diagCode == DIAGCODE){ //
        //Serial.println("Diag demandé");
        
        if (radio.ACKRequested())
        {
          radio.sendACK();
        }
        //delay(500);
        diagStatus.broadcast_RSSI = broadcast_RSSI; 
        diagStatus.trameCntOk = trameCntOk;
        if (radio.sendWithRetry(TESTEURID, (const void*)(&diagStatus), sizeof(DiagStatus),5,100)) {
          Serial.println("status de diag envoyé !") ;
        }
        else {
          Serial.println("Impossible d'envoyer le status de diag") ;
        } 
      }
      else if (diagBuff.mode == REMOTEMANUAL){
        if (radio.ACKRequested())
        {
          radio.sendACK();
        }
        mode = REMOTEMANUALMODE ;
        Serial.println("REMOTEMANUALMODE") ;
      }
      else if (diagBuff.mode == AUTO){
        if (radio.ACKRequested())
        {
          radio.sendACK();
        }
        mode = AUTOMODE ;
        Serial.println("Auto mode") ;
      }
    }
    else if (radio.DATALEN == sizeof(Payload) && radio.TARGETID == BROADCASTID)
    {
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      packet_id = theData.packetId;
      broadcast_RSSI = radio.readRSSI();
      checkCom();
    }
    radio.receiveDone(); //put back the radio in RX mode
  } 
    
    
  



 
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

  
  if (radio.receiveDone())
  {
    //Blink(LED1,500);
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");
  
/*
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
    }*/
    
   
    
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
  radio.promiscuous(true);
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
  stripLed_init();
  wireless_init();
  
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

void remoteManual(void){
 if (bPacketRcv){
   if (diagBuff.diagCode == FULLOFF){
     black_strip();
     //digitalWrite(PIN_RELAY, LOW);
   }
   else if (diagBuff.diagCode == FULLRED){
     fullRed();
     //digitalWrite(PIN_RELAY, HIGH);
   }
   else if (diagBuff.diagCode == FULLGREEN){
     fullGreen();
   }
   else if (diagBuff.diagCode == FULLBLUE){
     fullBlue();
   }
 }
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

void controlRelay(){
  if (packet_id == 1){
    if (radio.DATA[0]>150)
        digitalWrite(PIN_RELAY, HIGH);
    else
        digitalWrite(PIN_RELAY, LOW);    
  }
}
