#ifndef WirelessShow
#define WirelessShow

#include "Arduino.h"

//#define RadioHead

#ifndef RadioHead
  #include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#else 
  #include <SPI.h>
  #include <RH_RF69.h>
  #include <RHReliableDatagram.h>
#endif

#include "parameters.h"

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 100 // The same on all nodes that talk to each other
#define NODEID 1 // The unique identifier of this node


//******************* LIST OF RECEIVERS *****************
#define NODERECEIVE 2
//*******************************************************

//Match frequency to the hardware version of the radio on your Feather
#ifndef RadioHead
  #define FREQUENCY RF69_433MHZ
#else 
  #define RF69_FREQ 434.0
#endif
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module

#define RFM69_CS 10 // Chip select
#define RFM69_IRQ 2 // interruption
#define RFM69_IRQN 0 // Pin 2 is IRQ 0!
#define RFM69_RST 9 // Reset


/****************** DEFINE LIST ***************************/
#define PACKET_SIZE 60 // number of channels to send per packet
#define PACKET_SIZE_PLUS_ID  PACKET_SIZE+1 // total size of a packet with the ID
#define PACKET_NBR 8 // nombre paquet que l'on souhaite envoyer
#define PACKET_AVAILABLE 8 // nombre de paquets qui dÃ©composent l'ensemble des canaux DMX

/*************************************************************/


void wireless_init(void);
void sendPackets(uint8_t  *trame);
void build_packet(int packet_id, uint8_t  *trame , uint8_t  *packet );
void delete_zeros(uint8_t *packet, int i);




#endif // WirelessShow
