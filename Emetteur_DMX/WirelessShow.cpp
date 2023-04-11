#include "WirelessShow.h"

typedef struct {
  uint8_t           packetId; //store this nodeId
  uint8_t packet[PACKET_SIZE];
} Payload;
Payload theData;

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

void wireless_init(void){
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
}

void sendPackets(uint8_t  *trame){
  uint8_t radiopacket[PACKET_SIZE_PLUS_ID] ;

  for (int i = 1 ; i <=PACKET_NBR ; i++){
      build_packet(i, trame, radiopacket); // construction et envoi du paquet i
      radio.send(NODERECEIVE, (const void*)radiopacket, strlen(radiopacket), false) ; // envoi du paquet de donnÃ©es
    }
}

/*
 * Construit un paquet de plusieurs canaux DMX selon le packet_id donnÃ©es en paramÃ¨tre.
 * @param int packet_id : l'identifiant du paquet Ã  envoyer
 * @return None.
 */
void build_packet(int packet_id, uint8_t  *trame , uint8_t  *packet ) {
  packet[0] = packet_id ; // L'identifiant du paquet est le premier byte du paquet Ã  envoyer
  int channel_offset = (packet_id - 1) * PACKET_SIZE ; // calcul de l'offset du canal DMX selon le packet_id

  for (int i = 1 ; i < (PACKET_SIZE + 1) ; i++) { // construction du paquet Ã  envoyer avec les canaux DMX
    packet[i] = trame[i + channel_offset] ;
    delete_zeros(packet, i) ;
  }
}

/*
 * delete_zeros regarde si la valeur du canal donnÃ©e en paramÃ¨tre est nulle. Si oui elle la chnage en 1. Cette function est utilisÃ©e pour permettre l'envoi des donnÃ©es via
 * le module RFM69. Le module ne prends pas les valeurs nulles.
 * @param int i : le canal concernÃ©
 * @return None.
 */
void delete_zeros(uint8_t *packet, int i){
   if (packet[i] == 0 )
      packet[i] = 1 ; // rejet des zeros sur les canaux DMX pour Ã©viter les erreurs de transmission sur la liason RFM69
}
