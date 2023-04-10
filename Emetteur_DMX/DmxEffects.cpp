#include "DmxEffects.h"

void fulloff(uint8_t  *data){
  for (int i = 0; i<DMXSERIAL_MAX;i++){
    data[i] = 0;
  }
}

void fullOn(uint8_t  *data){
  for (int i = 1; i<DMXSERIAL_MAX-1;i++){
    data[i] = 255;
  }
}

void fullRed(uint8_t  *data){
  for (int i = 1; i<DMXSERIAL_MAX-1;i=i+3){
    data[i] = 255;
    data[i+1] = 0;
    data[i+2] = 0;
  }
}

void fullGreen(uint8_t  *data){
  for (int i = 1; i<DMXSERIAL_MAX-1;i=i+3){
    data[i] = 0;
    data[i+1] = 0;
    data[i+2] = 255;
  }
}

void fullBlue(uint8_t  *data){
  for (int i = 1; i<DMXSERIAL_MAX-1;i=i+3){
    data[i] = 0;
    data[i+1] = 255;
    data[i+2] = 0;
  }
}


void RVBSequence(uint8_t  *data, int _delay){
  static int seqId = 0;
  seqId = seqId + 1;
  if (seqId>4){
    seqId = 0;
  }

  delay(_delay);
  switch (seqId){
    case 0:
      fullRed(data);
      break;
    case 1:
      fullGreen(data);
      break;
    case 2:
      fullBlue(data);
      break;
    case 3:
      fullOn(data);
      break;
    case 4:
      fulloff(data);
      break;
    default:
      break;
  }
}

