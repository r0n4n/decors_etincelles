// Récepteur étincelles aquatiques pour gérer une bande néopixel
// Le récepteur est configurable avec une adresse unique


//********************INCLUDES************//
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>
#endif
/*********************************************/

//#define DEBUG
//#define DEBUG_CONFIG

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 //the same on all nodes that talk to each other
#define NODEID 2

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY RF69_433MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD 115200

#define RFM69_CS 10
#define RFM69_IRQ 2
#define RFM69_IRQN 0 // Pin 2 is IRQ 0!
#define RFM69_RST 9

//*********************   DEFINE OUT/IN PINS **************************
#define LED_bas 7 // cette LED montre l'état du décor : si la LEd est allumé le décor doit aussi être allumé 
#define LED_haut 6 // cette LED montre l'état du décor : si la LEd est allumé le décor doit aussi être allumé 
#define reception 8 // le led clignote dès que le récepteur reçoit un message 
#define bas 3 //  pin commande du décor 1 
#define haut 4 // pin commande du décor 2 
#define BANDE1 5 // pin de la bande LEd a définir

// How many NeoPixels are attached to the Arduino?
#define DECOR_ID 2 
#define NUMPIXELS      50.0
#define CHANNELS_PER_PIXEL 3 // RVB
#define PIX_PER_GROUP 1 // number of pix together 
#define PACKET_SIZE 60.0 // size max of a packet received
#define DECOR_DMX_ADRESS  1
#define PACKET_ID_MAX 9

#define CHANNELS_NBR (NUMPIXELS*CHANNELS_PER_PIXEL/PIX_PER_GROUP) // on détermine le nombre de canaux nécessaires 
#define PIXELS_PER_PACKET  (PACKET_SIZE*PIX_PER_GROUP/CHANNELS_PER_PIXEL) // nombre de pixel par paquet 
#define NUM_PACKET ceil(CHANNELS_NBR/PACKET_SIZE) // nombre de paquets  
#define CHANNELS_REST ((int)CHANNELS_NBR%(int)PACKET_SIZE) // le nombre de channels restant si le nombre de channels total n'est pas multiple de la taille d'un paquet
#define PIXELS_IN_LAST_PACKET (CHANNELS_REST*PIX_PER_GROUP/CHANNELS_PER_PIXEL) // Le nombre de channels concernés par le dernier paquet 
#define NBR_GROUP_PER_PACKET (PIXELS_PER_PACKET/PIX_PER_GROUP)
#define LAST_DMX_ADRESS (DECOR_DMX_ADRESS+CHANNELS_NBR-1)
#define FIRST_PACKET_TO_READ ceil(DECOR_DMX_ADRESS/PACKET_SIZE)




#define INITIAL_STATE 1
#define FINAL_STATE 2

int start_pixel ;
int state ;
int packet_id ;

int start_index =0  ;
int stop_index =0 ; 

int start_packet =0  ; 
int stop_packet =0 ; 

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, BANDE1, NEO_GRB + NEO_KHZ800);
RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);


//________________SETUP______________________
void setup() {
  find_index() ;
  state = start_packet ; // initialize the state machine 
  //**************** CONFIG DES ENTREES/SORTIES *************
  pinMode(LED_bas, OUTPUT);
  pinMode(LED_haut, OUTPUT);
  pinMode(reception, OUTPUT);
  pinMode(bas, OUTPUT) ;
  pinMode(haut, OUTPUT ) ;
  //******************************************************************
  digitalWrite(LED_haut, HIGH) ;

  //************** RADIO INIT *********************
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
  radio.encrypt(ENCRYPTKEY);
  //*************************************************

  /************* NEOPIXEL SETTINGS ********/
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin(); // This initializes the NeoPixel library.
  //*************************************
  Serial.begin(SERIAL_BAUD);
   

  //********* AFFICHE LES INFOS DU MODULES *******
#ifdef DEBUG_CONFIG
  
  Serial.print("Recepteur strip led numero "); Serial.println(DECOR_ID); 
  
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
  Serial.print("PIXELS_PER_PACKET : ") ; Serial.println(PIXELS_PER_PACKET) ;
  Serial.print("NUM_PACKET : ") ; Serial.println(NUM_PACKET) ;
  Serial.print("CHANNELS_REST : ") ; Serial.println(CHANNELS_REST) ;
  Serial.print("PIXELS_IN_LAST_PACKET : ") ; Serial.println(PIXELS_IN_LAST_PACKET) ;
  Serial.print("start_packet : ") ; Serial.println(start_packet) ;
  Serial.print("start_index : ") ; Serial.println(start_index) ;
  Serial.print("stop_packet : ") ; Serial.println(stop_packet) ;
  Serial.print("stop_index : ") ; Serial.println(stop_index) ;
  
  
#endif
digitalWrite(LED_haut, LOW) ;

}

void loop() {
  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    digitalWrite(reception, HIGH) ;
    //Serial.print("[RX_RSSI:"); Serial.print(radio.RSSI); Serial.println("]");
    packet_id = radio.DATA[0] ; // the first byte give the pakcet ID sent
#ifdef DEBUG
//    Serial.print("start_pixel: ") ;
//    Serial.println(packet_id) ;
#endif
    digitalWrite(reception, LOW) ;
    radio.receiveDone(); //put radio in RX mode
  }

#ifdef DEBUG
  //Serial.print("Wait for packet ") ; Serial.print(state) ; Serial.println("...") ;
#endif

  if (packet_id == state) {
#ifdef DEBUG
    Serial.print("packet ") ; Serial.print(packet_id) ; Serial.println(" received") ;
#endif
   // start_pixel = (packet_id - 1) * PIXELS_PER_PACKET ;

    if (state == stop_packet) {
      prepare_pixel_color1(1, stop_index, packet_id) ;
      //prepare_pixel_color(start_pixel, PIXELS_PER_PACKET) ;
      //prepare_pixel_color3(start_pixel, NBR_GROUP_PER_PACKET) ; 
      pixels.show();
      state = start_packet ;
#ifdef DEBUG
      Serial.println("Strip updated!") ;
#endif
    }
    else if (state ==start_packet){
       prepare_pixel_color1(start_index, PACKET_SIZE, packet_id) ;
       state++ ;
    }
    else {
      prepare_pixel_color1(1, PACKET_SIZE, packet_id) ;
      //prepare_pixel_color(start_pixel, PIXELS_PER_PACKET) ;
     // prepare_pixel_color3(start_pixel, NBR_GROUP_PER_PACKET) ;
      state++ ;
    }
  }
  
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
}

void prepare_pixel_color1(int start_indice, int stop_indice, int packet_ID) {
 // Serial.println("prepare_pixel_color1 launched") ; 
  //int pixel_offset = ((packet_ID-1)*PACKET_SIZE-DECOR_DMX_ADRESS)/CHANNELS_PER_PIXEL  ; 
  int pixel_offset = ((packet_ID-1)*PACKET_SIZE + 1 - DECOR_DMX_ADRESS)/3 ; 
  for (int i = start_indice; i < stop_indice  ; i+=3) { // parcours les éléments du tableau reçu 
    // Serial.print("i : ") ;   Serial.println(i) ; 
    pixels.setPixelColor((i+2)/3+pixel_offset, pixels.Color(radio.DATA[i], radio.DATA[i+1], radio.DATA[i+2])); // change the color
#ifdef DEBUG
    //          Serial.print("Pixel:") ; Serial.print(i + start_pixel) ; Serial.print(": ") ;
    //          Serial.print(radio.DATA[3 * i + 1]) ;
    //          Serial.print(" ") ;
    //          Serial.print(radio.DATA[3 * i + 2]) ;
    //          Serial.print(" ") ;
    //          Serial.print(radio.DATA[3 * i + 3]) ;
    //          Serial.println(" ") ;
#endif
  }
}

void prepare_pixel_color(int start_pixel, int pixel_number) {
  for (int i = 0; i < pixel_number  ; i++) {
    pixels.setPixelColor(i+start_pixel, pixels.Color(radio.DATA[3*i+1], radio.DATA[3*i+2], radio.DATA[3*i+3])); // change the color
#ifdef DEBUG
    //          Serial.print("Pixel:") ; Serial.print(i + start_pixel) ; Serial.print(": ") ;
    //          Serial.print(radio.DATA[3 * i + 1]) ;
    //          Serial.print(" ") ;
    //          Serial.print(radio.DATA[3 * i + 2]) ;
    //          Serial.print(" ") ;
    //          Serial.print(radio.DATA[3 * i + 3]) ;
    //          Serial.println(" ") ;
#endif
  }
}

void prepare_pixel_color3(int start_pixel, int pixel_number) {
  for (int i = 0; i < pixel_number  ; i++) {
    pixels.setPixelColor(i*3+start_pixel, pixels.Color(radio.DATA[3*i+1], radio.DATA[3*i+2], radio.DATA[3*i+3])); // change the color
    pixels.setPixelColor(i*3+1+start_pixel, pixels.Color(radio.DATA[3*i+1], radio.DATA[3*i+2], radio.DATA[3*i+3])); // change the color
    pixels.setPixelColor(i*3+2+start_pixel, pixels.Color(radio.DATA[3*i+1], radio.DATA[3*i+2], radio.DATA[3*i+3])); // change the color
#ifdef DEBUG
    //          Serial.print("Pixel:") ; Serial.print(i + start_pixel) ; Serial.print(": ") ;
    //          Serial.print(radio.DATA[3 * i + 1]) ;
    //          Serial.print(" ") ;
    //          Serial.print(radio.DATA[3 * i + 2]) ;
    //          Serial.print(" ") ;
    //          Serial.print(radio.DATA[3 * i + 3]) ;
    //          Serial.println(" ") ;
#endif
  }
}

void find_index() {  
  int packet_index = 1 ; 
   
  while ( packet_index < PACKET_ID_MAX) {
    int channel_inf = (packet_index-1)*PACKET_SIZE+1 ; 
    int channel_sup = packet_index*PACKET_SIZE ; 
    if ( (channel_inf <=DECOR_DMX_ADRESS)   &(DECOR_DMX_ADRESS <=channel_sup)  ){ 
       start_index = DECOR_DMX_ADRESS - ((packet_index -1)*60) ; 
       start_packet = packet_index ; 
    }
    if (  (channel_inf <=LAST_DMX_ADRESS)   &(LAST_DMX_ADRESS <=channel_sup)  ){
      stop_index = LAST_DMX_ADRESS - ((packet_index -1)*60) ; 
      stop_packet = packet_index ; 
      break ; 
    }
    packet_index++ ; 
  }
}

