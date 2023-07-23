#ifndef wirelessDMX_h
#define wirelessDMX_h

#define NO_DATA_SINCE 3000

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

// ***************** Configuration réseau *********************
#define NETWORKID 100 //the same on all nodes that talk to each other
#define TRANSMITTERID 1 // L'adresse réseau de l'émetteur
#define BROADCASTID 2 // L'adresse où le DMX est broadcasté 
#define NODEID 4 // L'adresse réseau du récepteur 
//***********************************************************************

/** TESTEUR ETATS **/
#define STANDBY 0
#define LISTENER 1
#define ONEDIAG 2
/***********************/

/** CODE DIAG **/ 
#define DIAGCODE 321
#define FULLRED 100
#define FULLGREEN 200
#define FULLBLUE 300


/*************** VARIABLES ********************/
int state ; // L'id du paquet qui est attendu
int tst_state ;
int prev_tst_state = -1 ; 
int packet_id ; // L'id du paquet qui vient d'être reçu
int last_packet_id ; // ID du dernier paquet reçu 
int trameCntOk = 0;
int nbr_paquet_perdu = 0; // compteur du nombre de paquets perdu 
int start_index = 0  ; // premier indice dans le premier paquet que le récepteur doit interpréter
int stop_index = 0 ; // dernier indice dans le dernier paquet que le récepteur doit interpréter
int start_packet = 0  ; // id du premier paquet que le récepteur doit interpréter
int stop_packet = 0 ; // id du dernier paquet que le récepteur doit interpréter
unsigned long last_reception = 0 ;
unsigned long package_rcv_delta_t = 0 ; // delta t entre deux réceptions de packet 
bool refresh = false ;
bool first_iter = true ; 
bool bPacketRcv = false;
int broadcast_RSSI = 0;
int packetIdBuff[PACKET_NBR];
float debit = 0; 


typedef struct {
  uint8_t           packetId; //store this nodeId
  uint8_t packet[PACKET_SIZE];
} Payload;
Payload theData;

typedef struct {
  int broadcast_RSSI;
  int trameCntOk;
} DiagStatus;

typedef struct {
  int diagCode;
} DiagBuff;
DiagBuff diagBuff;
/**********************************************/

/************** OBJECTS ***********************/
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, BANDE1, NEO_GRB + NEO_KHZ800); // Création de l'objet pixel qui gère la bande numérique du pin BANDE1
RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN); // Création de l'objet radio qui gère le module RFM69
/***********************************************/


#endif
