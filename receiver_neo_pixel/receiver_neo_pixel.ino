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
    Une fois tous les paquets nécessaires reçu, les nouvelles couleurs sont envoyés à la bande numérique puis le programme va attendre de nouveau le premier paquet.
 *****************************************************************/

/************** *************   INCLUDES LIBRARY ********************************************/
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <Adafruit_NeoPixel.h>

#include "parameters.h"

#ifdef __AVR__
#include <avr/power.h>
#endif
/*********************************************************************************************/

//#define DEBUG
//#define DEBUG_CONFIG

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
//*********************   DEFINE OUT/IN PINS **************************
// RFM69 PINS
#define RFM69_CS 10 // CHIP SELECT PIN
#define RFM69_IRQ 2 // pin IRQ
#define RFM69_IRQN 0 // Pin 2 is IRQ 0!
#define RFM69_RST 9 // RST PIN 

// HARDWARE OUTPUTS
#define LED1 6
#define LED2 7
#define LED3 8
#define T1 3 // Transistor 1 
#define T2 4 // Transistor 2 
#define BANDE1 5 // pin pour contrôler la bande Led

#define RECEPTION LED3 // la led clignote dès que le récepteur reçoit un message 
//********************************************************************************************

// ***************** NETWORKS PARAMETERS BETWEEN RFM69 *********************
//**************************************************************************
#define NETWORKID 100 //the same on all nodes that talk to each other
#define NODEID 2 // L'adresse réseau du récepteur 
#define FREQUENCY RF69_433MHZ //Match frequency to the hardware version of the radio on your Feather
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module
#define NO_DATA_SINCE 60 

//************************************************************************************
#define SERIAL_BAUD 115200


// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      50.0
#define CHANNELS_PER_PIXEL 3 // RVB
#define PIX_PER_GROUP 1 // number of pixels together 
#define PACKET_SIZE 60.0 // size of a packet received
#define DECOR_DMX_ADRESS  1 // adresse DMX du récepteur 
#define PACKET_ID_MAX 9 // nombre de paquets maximal que peut envoyer l'émetteur (dépend de la taille des paquets)  

#define CHANNELS_NBR (NUMPIXELS*CHANNELS_PER_PIXEL/PIX_PER_GROUP) // on détermine le nombre de canaux nécessaires
#define LAST_DMX_ADRESS (DECOR_DMX_ADRESS+CHANNELS_NBR-1)

/*************** VARIABLES ********************/
int state ; // L'id du paquet qui est attendu
int packet_id ; // L'id du paquet qui vient d'être reçu
int start_index = 0  ; // premier indice dans le premier paquet que le récepteur doit interpréter
int stop_index = 0 ; // dernier indice dans le dernier paquet que le récepteur doit interpréter
int start_packet = 0  ; // id du premier paquet que le récepteur doit interpréter
int stop_packet = 0 ; // id du dernier paquet que le récepteur doit interpréter
unsigned long last_reception = 0 ; 
/**********************************************/

/************** OBJECTS ***********************/
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, BANDE1, NEO_GRB + NEO_KHZ800); // Création de l'objet pixel qui gère la bande numérique du pin BANDE1
RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN); // Création de l'objet radio qui gère le module RFM69
/***********************************************/

//________________SETUP______________________
void setup() {
  SYSTEM_INIT() ; // initialise the hardware
  black_strip() ; 
  find_index() ; // détermine start_index, start_index, start_packet et stop_packet  ;
  state = start_packet ; // initializes the state machine
#ifdef DEBUG_CONFIG
  print_config() ; // AFFICHE LES INFOS DU MODULES
#endif
#ifdef DEBUG
  Serial.print("Wait for packet ") ; Serial.print(state) ; Serial.println("...") ;
#endif
}
//___________________________________________

//______________ LOOP _______________________
void loop() {
  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    last_reception = millis() ; 
    packet_id = radio.DATA[0] ; // the first byte give the packet ID sent
#ifdef DEBUG
  //Serial.print("[RX_RSSI:"); Serial.print(radio.RSSI); Serial.println("]");
    Serial.print("packet_id received: ") ; Serial.println(packet_id) ;
#endif
    
    radio.receiveDone(); //put radio in RX mode // voir si nécessaire
  }
   _noDataSince() ; 
 


  if (packet_id == state) { // check if the packet received is the one we are waiting for
#ifdef DEBUG
    Serial.print("packet ") ; Serial.print(packet_id) ; Serial.println(" received") ;
#endif
    // start_pixel = (packet_id - 1) * PIXELS_PER_PACKET ;

    if (state == stop_packet) { // si le paquet reçu est le dernier paquet exigé
       digitalWrite(LED1, LOW) ;
      prepare_pixel_color1(1, stop_index, packet_id) ;
      pixels.show(); // on met à jour les pîxels de la bande
      state = start_packet ; // on retourne à l'état initial : attendre le premier paquet
#ifdef DEBUG
      Serial.print("Wait for packet ") ; Serial.print(state) ; Serial.println("...") ;
#endif
#ifdef DEBUG
      Serial.println("Strip updated!") ;
#endif
    }
    else if (state == start_packet) { // si le paquet reçu est le premier paquet exigé
       digitalWrite(LED1, HIGH) ;
      prepare_pixel_color1(start_index, PACKET_SIZE, packet_id) ;
      state++ ; // on attend le paquet suivant
#ifdef DEBUG
      Serial.print("Wait for packet ") ; Serial.print(state) ; Serial.println("...") ;
#endif
    }
    else {
      prepare_pixel_color1(1, PACKET_SIZE, packet_id) ;
      state++ ; // on attend le paquet suivant
#ifdef DEBUG
      Serial.print("Wait for packet ") ; Serial.print(state) ; Serial.println("...") ;
#endif
    }
  }

  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU // voir si nécessaire
}
// _________________________________________________________________________

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

//void prepare_pixel_color(int start_pixel, int pixel_number) {
//  for (int i = 0; i < pixel_number  ; i++) {
//    pixels.setPixelColor(i + start_pixel, pixels.Color(radio.DATA[3 * i + 1], radio.DATA[3 * i + 2], radio.DATA[3 * i + 3])); // change the color
//#ifdef DEBUG
//    //          Serial.print("Pixel:") ; Serial.print(i + start_pixel) ; Serial.print(": ") ;
//    //          Serial.print(radio.DATA[3 * i + 1]) ;
//    //          Serial.print(" ") ;
//    //          Serial.print(radio.DATA[3 * i + 2]) ;
//    //          Serial.print(" ") ;
//    //          Serial.print(radio.DATA[3 * i + 3]) ;
//    //          Serial.println(" ") ;
//#endif
//  }
//}

//void prepare_pixel_color3(int start_pixel, int pixel_number) {
//  for (int i = 0; i < pixel_number  ; i++) {
//    pixels.setPixelColor(i*3+start_pixel, pixels.Color(radio.DATA[3*i+1], radio.DATA[3*i+2], radio.DATA[3*i+3])); // change the color
//    pixels.setPixelColor(i*3+1+start_pixel, pixels.Color(radio.DATA[3*i+1], radio.DATA[3*i+2], radio.DATA[3*i+3])); // change the color
//    pixels.setPixelColor(i*3+2+start_pixel, pixels.Color(radio.DATA[3*i+1], radio.DATA[3*i+2], radio.DATA[3*i+3])); // change the color
//#ifdef DEBUG
//    //          Serial.print("Pixel:") ; Serial.print(i + start_pixel) ; Serial.print(": ") ;
//    //          Serial.print(radio.DATA[3 * i + 1]) ;
//    //          Serial.print(" ") ;
//    //          Serial.print(radio.DATA[3 * i + 2]) ;
//    //          Serial.print(" ") ;
//    //          Serial.print(radio.DATA[3 * i + 3]) ;
//    //          Serial.println(" ") ;
//#endif
//  }
//}

void find_index() {
  int packet_index = 1 ;

  while ( packet_index < PACKET_ID_MAX) {
    int channel_inf = (packet_index - 1) * PACKET_SIZE + 1 ;
    int channel_sup = packet_index * PACKET_SIZE ;
    if ( (channel_inf <= DECOR_DMX_ADRESS)   & (DECOR_DMX_ADRESS <= channel_sup)  ) {
      start_index = DECOR_DMX_ADRESS - ((packet_index - 1) * 60) ;
      start_packet = packet_index ;
    }
    if (  (channel_inf <= LAST_DMX_ADRESS)   & (LAST_DMX_ADRESS <= channel_sup)  ) {
      stop_index = LAST_DMX_ADRESS - ((packet_index - 1) * 60) ;
      stop_packet = packet_index ;
      break ;
    }
    packet_index++ ;
  }
}


void SYSTEM_INIT(void) {

  //**************** CONFIG DES ENTREES/SORTIES *************
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(RECEPTION, OUTPUT);
  pinMode(T1, OUTPUT) ;
  pinMode(T2, OUTPUT ) ;
  //******************************************************************
  digitalWrite(LED1, HIGH) ; // set led high to show that the setup has started

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

  //************* NEOPIXEL SETTINGS *********************************
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin(); // This initializes the NeoPixel library.
  //*****************************************************************

  // init serial port for debugging
#ifdef DEBUG || DEBUG_CONFIG
  Serial.begin(SERIAL_BAUD);
#endif
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

void black_strip(void){
  for (int i= 0; i<NUMPIXELS; i++){
    pixels.setPixelColor(i, 0, 0, 0);
    pixels.show(); // on met à jour les pîxels de la bande
  }
}

bool _noDataSince() {
  if (millis()-last_reception > NO_DATA_SINCE){
    digitalWrite(RECEPTION, LOW) ;
    return false ;
  }
  else 
    digitalWrite(RECEPTION, HIGH) ;
    return true ;
}

