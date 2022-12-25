#ifndef wirelessDMX_h
#define wirelessDMX_h

#define NO_DATA_SINCE 60

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      50.0
#define CHANNELS_PER_PIXEL 3 // RVB
#define PIX_PER_GROUP 1 // number of pixels together 
#define PACKET_SIZE 60 // size of a packet received
#define DECOR_DMX_ADRESS  1 // adresse DMX du récepteur 
#define PACKET_ID_MAX 9 // nombre de paquets maximal que peut envoyer l'émetteur (dépend de la taille des paquets)  
#define PACKET_NBR (int)8 // nombre de paquets envoyés par l'emetteur 

#define CHANNELS_NBR (NUMPIXELS*CHANNELS_PER_PIXEL/PIX_PER_GROUP) // on détermine le nombre de canaux nécessaires
#define LAST_DMX_ADRESS (DECOR_DMX_ADRESS+CHANNELS_NBR-1)

/*************** VARIABLES ********************/
int state ; // L'id du paquet qui est attendu
int packet_id ; // L'id du paquet qui vient d'être reçu
int last_packet_id ; // ID du dernier paquet reçu 
bool paquet_perdu = false; // True si un paquet n'a pas été reçu, false sinon
int nbr_paquet_perdu = 0; // compteur du nombre de paquets perdu 
int start_index = 0  ; // premier indice dans le premier paquet que le récepteur doit interpréter
int stop_index = 0 ; // dernier indice dans le dernier paquet que le récepteur doit interpréter
int start_packet = 0  ; // id du premier paquet que le récepteur doit interpréter
int stop_packet = 0 ; // id du dernier paquet que le récepteur doit interpréter
unsigned long last_reception = 0 ;
unsigned long package_rcv_delta_t = 0 ; // delta t entre deux réceptions de packet 
bool refresh = false ;
bool first_iter = true ; 
/**********************************************/

/************** OBJECTS ***********************/
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, BANDE1, NEO_GRB + NEO_KHZ800); // Création de l'objet pixel qui gère la bande numérique du pin BANDE1
RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN); // Création de l'objet radio qui gère le module RFM69
/***********************************************/


#endif
