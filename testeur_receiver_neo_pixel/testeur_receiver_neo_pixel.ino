// Testeur de Strip LEDs : Execute une sequence de couleurs sur la bande LEDs connectée
// Test la réception du signal sans fils 

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

int mode = 1 ; // 1=receiver ; 2= transmitter; 3 =receiverStruct ; 4=stripLed 
#define AUTOMODE 1;
#define REMOTEMANUALMODE 2;
#define LOCALMANUALMODE 3;
//#define DEBUG
//  #define DEBUG_CONFIG
unsigned long previousMillis = 0;  // will store last time LED was updated
// Variables will change:
int ledState = LOW;  // ledState used to set the LED
byte ackCount=0;

#if (STRIP_CONFIG == STRIP_QUAD)
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_STRIP1 , NEO_BRG + NEO_KHZ800); // on configure la première bande avec toutes les LEDs pour assurer la retro-compatibilité
  //Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(STRIP2_LEDS_NBR, PIN_STRIP2, NEO_BRG + NEO_KHZ800); 
  Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(STRIP3_LEDS_NBR + STRIP4_LEDS_NBR , PIN_STRIP3, NEO_BRG + NEO_KHZ800); // on configure la première bande avec toutes les LEDs pour assurer la retro-compatibilité
  Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(STRIP4_LEDS_NBR, PIN_STRIP4, NEO_BRG + NEO_KHZ800); 
#else
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_STRIP1 , NEO_BRG + NEO_KHZ800); // on configure la première bande avec toutes les LEDs pour assurer la retro-compatibilité
  Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(STRIP2_LEDS_NBR, PIN_STRIP2, NEO_BRG + NEO_KHZ800); 
#endif

#define STRIP_ON_OFF_INDIC 

#define SERIAL_BAUD 115200

//________________SETUP______________________
void setup() {
  IOinit();
  Serial.begin(SERIAL_BAUD);
  stripLed_init();
  wireless_init();
  state = start_packet ; // initializes the state machine
//#ifdef DEBUG_CONFIG
//  print_config() ; // AFFICHE LES INFOS DU MODULES
//#endif
}
//___________________________________________


void IOinit(void){
  //**************** CONFIG DES ENTREES/SORTIES *************
  pinMode(LED_ONOFF, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED_RECEPTION, OUTPUT);
  //  pinMode(T1, OUTPUT) ;
  //pinMode(T2, OUTPUT ) ;
  //******************************************************************
  digitalWrite(LED_ONOFF,HIGH);
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
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  
#if (STRIP_CONFIG == STRIP_QUAD)
  pixels.begin(); 
  //strip2.begin();
  strip3.begin();
  strip4.begin();
#else
  pixels.begin(); 
  strip2.begin();

#endif
}


//______________ LOOP _______________________
void loop() { 
  listenRadio();
  testseq() ; 
  switch (mode){
    case 1: 
      //printPacket(1,0,10);
      buildDmxFrame();
      //printDMX(1,65);
      //printReception();
      updateDevices();
      break;
    case 4:
      stripLEDManual();
      break;
   }
}

//****************WIRELESS FUNCTIONS********************************/
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

bool _noDataSince() {
  package_rcv_delta_t = millis() - last_reception; 
  debit = 1000/float(package_rcv_delta_t); 
  if (package_rcv_delta_t > NO_DATA_SINCE) {
    digitalWrite(LED_RECEPTION, LOW) ;
    trameCntOk = 0;
    broadcast_RSSI = 0;
    return false ;
  }
  else
    digitalWrite(LED_RECEPTION, HIGH) ;
  return true ;
}

void printReception() { 
  static unsigned int  counter = 0;
  static unsigned int print_decimation = 100000;
  if (counter>=print_decimation){
      Serial.print("Broadcast RSSI: ");Serial.println(broadcast_RSSI);
      Serial.print("Qualité comm :") ; Serial.print(trameCntOk) ; Serial.println("/10") ;
      //printDMX(40,50);
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

void printPacket(int packetNbr, int start, int stop){
  if (packet_id == packetNbr){
    for (int i = start;i<stop;i++){
      Serial.print(theData.packet[i]); Serial.print(" ");
    }
    Serial.println();
  }
}
/*******************************************************************************/


/******* DMX FRAME FUNCTIONS **********/
void checkCom(){
  static int packetCounter = 0;
  static int trameCnt = 0;
  static int trameCntOkTmp = 0;
  static bool paquet_perdu = false; // True si un paquet n'a pas été reçu, false sinon
  #define TRAMESNBRMAX 10
  bDMXFrameRcv = false;

  if (trameCnt>=TRAMESNBRMAX){
    trameCntOk = trameCntOkTmp;
    trameCntOkTmp = 0;
    trameCnt = 0;
    
  }

  //if (first_iter == false){ 
    if (packet_id <last_packet_id){ // une nouvelle trame arrive
      paquet_perdu = false;
      trameCnt++;
      packetCounter = 0;
    }
      
    if (paquet_perdu ==false && packet_id>1 && packet_id<=PACKET_NBR && last_packet_id!=(packet_id-1)){ // si un paquet est perdu
      paquet_perdu = true;
      //Serial.println("paquet perdu");
    }

    if (packet_id==PACKET_NBR){ // si à la fin de la trame aucun paquet n'a été perdu 
      if (!paquet_perdu){
        trameCntOkTmp++;
        bDMXFrameRcv = true;
      } 
    }

  //}
  last_packet_id = packet_id;

  if (packetCounter<10){
      packetIdBuff[packetCounter] = packet_id;
      packetCounter++;
  }
  else 
      packetCounter = 0;
}

// build DMX frame with packets received
bool buildDmxFrame(){
  static uint8_t nxt_packetId_ = 1 ; // next packet expected to rebuild the DMX frame
  if (bPacketRcv) {
    int adressOffset = (packet_id-1)*PACKET_SIZE;
    for (uint8_t i = 1; i<=PACKET_SIZE ;i++ ){
      if (radio.DATA[i] == 1 ) { // remove zero values
        dmxData[adressOffset + i] = 0 ;
      }
      else {
        dmxData[adressOffset + i] = radio.DATA[i] ;
      }
    }
  }
  return true;
}

void printDMX(uint8_t startAdr, uint8_t stopAdr){
  Serial.print(startAdr); Serial.print("-"); Serial.print(stopAdr) ;Serial.print(": ");
  for (uint8_t i = startAdr; i<= stopAdr ; i++){
    Serial.print(dmxData[i]);Serial.print(" ");
  }
  Serial.println();
}
/*********************************************************/

/****************** STRIP LEDS CONTROL ************************/
void updateDevices(){
  if (bDMXFrameRcv){
    //Serial.println("New DMX frame");
    #if (STRIP_CONFIG == STRIP_SINGLE)
    //Serial.println("String single");
    // strip 1 update
    for (int i = 0; i < NUMPIXELS  ; i++) { // parcours les éléments du tableau reçu
      #ifdef RBG
      // set color for RBG strip LEDs
      pixels.setPixelColor(i, pixels.Color(dmxData[DECOR_DMX_ADRESS+3*i], dmxData[DECOR_DMX_ADRESS + 3*i + 2], dmxData[DECOR_DMX_ADRESS+ 3*i +1])); // change the color
      #else if RGB 
      // set color for RGB strip LEDs
      pixels.setPixelColor(i, dmxData[DECOR_DMX_ADRESS+3*i], dmxData[DECOR_DMX_ADRESS + 3*i + 1], dmxData[DECOR_DMX_ADRESS+ 3*i +2]); // change the color
      #endif
    }
    

    #elif (STRIP_CONFIG == STRIP_DOUBLE)
    //Serial.println("String double");
    // strip 1 update
    for (int i = 0; i < NUMPIXELS  ; i++) { // parcours les éléments du tableau reçu
      #ifdef RBG
      // set color for RBG strip LEDs
      pixels.setPixelColor(i, dmxData[DECOR_DMX_ADRESS+3*i], dmxData[DECOR_DMX_ADRESS + 3*i + 2], dmxData[DECOR_DMX_ADRESS+ 3*i +1]); // change the color
      #else if RGB 
      // set color for RGB strip LEDs
      pixels.setPixelColor(i, dmxData[DECOR_DMX_ADRESS+3*i], dmxData[DECOR_DMX_ADRESS + 3*i + 1], dmxData[DECOR_DMX_ADRESS+ 3*i +2]); // change the color
      #endif
    }
    
    // strip2 update
    for (int i = 0; i < strip2.numPixels(); i++) { // parcours les éléments du tableau reçu
      #ifdef RBG
      // set color for RBG strip LEDs
      strip2.setPixelColor(i, dmxData[STRIP2_ADDRESS+3*i], dmxData[STRIP2_ADDRESS + 3*i + 2], dmxData[STRIP2_ADDRESS+ 3*i +1]); // change the color
      #else if RGB
      // set color for RGB strip LEDs
      strip2.setPixelColor(i, dmxData[STRIP2_ADDRESS+3*i], dmxData[STRIP2_ADDRESS + 3*i + 1], dmxData[STRIP2_ADDRESS+ 3*i +2]); // change the color
      #endif
    }
    
    #elif (STRIP_CONFIG == STRIP_QUAD)
    //Serial.println("String quad");
    // strip 1 update
    for (int i = 0; i < NUMPIXELS  ; i++) { // parcours les éléments du tableau reçu
      #ifdef RBG
      // set color for RBG strip LEDs
      pixels.setPixelColor(i, dmxData[DECOR_DMX_ADRESS+3*i], dmxData[DECOR_DMX_ADRESS + 3*i + 2], dmxData[DECOR_DMX_ADRESS+ 3*i +1]); // change the color
      #else if RGB 
      // set color for RGB strip LEDs
      pixels.setPixelColor(i, dmxData[DECOR_DMX_ADRESS+3*i], dmxData[DECOR_DMX_ADRESS + 3*i + 1], dmxData[DECOR_DMX_ADRESS+ 3*i +2]); // change the color
      #endif
    }
    /*
    // strip2 update
    for (int i = 0; i < strip2.numPixels(); i++) { // parcours les éléments du tableau reçu
      #ifdef RBG
      // set color for RBG strip LEDs
      strip2.setPixelColor(i, dmxData[STRIP2_ADDRESS+3*i], dmxData[STRIP2_ADDRESS + 3*i + 2], dmxData[STRIP2_ADDRESS+ 3*i +1]); // change the color
      #else if RGB
      // set color for RGB strip LEDs
      strip2.setPixelColor(i, dmxData[STRIP2_ADDRESS+3*i], dmxData[STRIP2_ADDRESS + 3*i + 1], dmxData[STRIP2_ADDRESS+ 3*i +2]); // change the color
      #endif
    }*/

    // strip 3 update
    for (int i = 0; i < strip3.numPixels()  ; i++) { // parcours les éléments du tableau reçu
      #ifdef RBG
      // set color for RBG strip LEDs
      strip3.setPixelColor(i, dmxData[STRIP3_ADDRESS+3*i], dmxData[STRIP3_ADDRESS + 3*i + 2], dmxData[STRIP3_ADDRESS + 3*i +1]); // change the color
      #else if RGB 
      // set color for RGB strip LEDs
      strip3.setPixelColor(i, dmxData[STRIP3_ADDRESS+3*i], dmxData[STRIP3_ADDRESS + 3*i + 1], dmxData[STRIP3_ADDRESS + 3*i +2]); // change the color
      #endif
    }
    
    // strip 4 update
    for (int i = 0; i < strip4.numPixels(); i++) { // parcours les éléments du tableau reçu
      #ifdef RBG
      // set color for RBG strip LEDs
      strip4.setPixelColor(i, dmxData[STRIP4_ADDRESS+3*i], dmxData[STRIP4_ADDRESS + 3*i + 2], dmxData[STRIP4_ADDRESS+ 3*i +1]); // change the color
      #else if RGB
      // set color for RGB strip LEDs
      strip4.setPixelColor(i, dmxData[STRIP4_ADDRESS+3*i], dmxData[STRIP4_ADDRESS + 3*i + 1], dmxData[STRIP4_ADDRESS+ 3*i +2]); // change the color
      #endif
    }
    #endif
  }
  #ifdef STRIP_ON_OFF_INDIC
  // turn on/off the LED on indicator
  if (pixels.getPixelColor(STRIP_ONOFF_LED)==0){
    pixels.setPixelColor(STRIP_ONOFF_LED, ONOFF_LED_BRIGHTNESS, 0, 0); // change the color
  }
  #endif
  
  // update the the strip LED every function calls 
  #if (STRIP_CONFIG == STRIP_QUAD)
    pixels.show();
    //strip2.show();
    strip3.show();
    strip4.show();
  #else
    //Serial.println("show else");
    pixels.show();
    strip2.show();
  #endif
}

void print_config(void) {
  Serial.print("Recepteur strip leds ");
  Serial.println("\nNetwork : ") ;
  Serial.print("Node: ") ; Serial.println(NODEID) ;
  Serial.print("NETWORKID: ") ; Serial.println(NETWORKID) ;
  Serial.print("DECOR_DMX_ADRESS: ") ; Serial.println(DECOR_DMX_ADRESS) ;
  Serial.println("\nDecor parameters : ") ;
  Serial.print("NUMPIXELS: ") ; Serial.println(NUMPIXELS) ;
}

void black_strip(void) {
  for (int i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, 0, 0, 0);
    pixels.show(); // on met à jour les pîxels de la bande
  }
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
    else if (input == "bleu")
      fullBlue();   
  }
}

void fullRed() {  
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
        pixels.setPixelColor(i, pixels.Color(255,0,0));
    }
    pixels.show();
}

void fullGreen() {  
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
        pixels.setPixelColor(i, pixels.Color(0,255,0));
    }
    pixels.show();
}

void fullBlue() {  
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
        pixels.setPixelColor(i, pixels.Color(0,0,255));
    }
    pixels.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  static int step = 0; 
  int step_nbr = pixels.numPixels();
  static unsigned long   _last_date = 0;
  unsigned long date = millis();
  unsigned long  _timer = date - _last_date ; 
  
  if (_timer> wait){
    pixels.setPixelColor(step, c);
    pixels.show();
    _last_date = date;
    step++;
    if (step>step_nbr){
      step = 0;
    }
  }
}

void testseq(void){
  static int step = 2; 
  int step_nbr = 5;
  int main_delay = 800; // ms
  int wipe_delay = 50; // ms
  static int _delay = main_delay; // ms
  static unsigned long   _last_date = 0;
  unsigned long date = millis();
  unsigned long  _timer = date - _last_date ; 
  
  /*if (step == 2){ // leave time to run the color wipe 
    _delay = pixels.numPixels()*wipe_delay;
  }
  else {
    _delay = main_delay;
  }*/
  
  if (_timer>_delay){
    _last_date = date;
    step++;
    if (step>step_nbr){
      step = 1;
    }
  }


  switch (step){
    case 1:
      black_strip();
      break;
    case 2:
      //colorWipe(pixels.Color(100, 100, 100), wipe_delay);
      step++;
      break;
    case 3:
      fullRed();
      break;
    case 4:
      fullGreen();
      break;
    case 5:
      fullBlue();
      break;
    default:
      break;
  }

  
 /* delay(_delay);
  fullGreen() ;
  delay(_delay);
  fullBlue() ; 
  delay(_delay);*/
}

/*********************************************************/

/*void controlRelay(){
  if (packet_id == 1){
    if (radio.DATA[0]>150)
        digitalWrite(PIN_RELAY, HIGH);
    else
        digitalWrite(PIN_RELAY, LOW);    
  }
}*/

/******************* UTILITIES *****************/

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
/****************************************/
