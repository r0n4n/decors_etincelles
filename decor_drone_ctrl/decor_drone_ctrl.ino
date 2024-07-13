#include "PPMEncoder.h"


#define PPM_OUT_PIN 10
#define CMD_PIN 5
#define INPUT_PIN 6

// RC channels pins 
#define ST_CH 7 // CH1
#define TH_CH 8 // CH2
#define CH3_ 9

bool cmd = false ;

// RC data struct
struct rc {
 long int ST;
 long int TH;
 long int CH3;
};

rc g2tb ;


void setup() {
    initRC();
    pinMode(CMD_PIN, OUTPUT);
    pinMode(INPUT_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN,LOW);
    
    // init PPM
    ppmEncoder.begin(PPM_OUT_PIN,5);
    ppmEncoder.setChannel(0, 1500);  
    ppmEncoder.setChannel(1, 1500);
    ppmEncoder.setChannel(2, 1500);
    ppmEncoder.setChannel(3, 1500);
    ppmEncoder.setChannel(4, 1500);
    ppmEncoder.setChannel(5, 1500);

}

void initRC(void){
  pinMode(ST_CH, INPUT);
  pinMode(TH_CH, INPUT);
  pinMode(CH3_, INPUT);
}

void loop() {
  readRC();
  ppmEncoder.setChannel(3, g2tb.ST);
  ppmEncoder.setChannel(2, g2tb.TH);
  ppmEncoder.setChannel(4, g2tb.CH3);
  
  //bool bRE = risingEdge(INPUT_PIN);
  //toggle(bRE);
  toggle(commandDetection(INPUT_PIN)); 
  //cmd = digitalRead(INPUT_PIN);
  if (cmd){
    digitalWrite(CMD_PIN,HIGH);
  }
  else{
    digitalWrite(CMD_PIN,LOW);
  }

}

unsigned long readRcChannel(int pin){
    return pulseIn(pin, HIGH);
}

void readRC(void){
  g2tb.ST = readRcChannel(ST_CH);
  g2tb.TH = readRcChannel(TH_CH);
  g2tb.CH3 = readRcChannel(CH3_);
}


bool risingEdge(int pin){
  static bool lastState = false;
  bool state;
  bool input = digitalRead(INPUT_PIN);
  if (input && lastState == false){
    state = true;
  }
  else {
    state = false;
  }
  lastState = input;
  return state;

}

void toggle(bool input){
  static bool state = false ;
  if (input){
    state = !state;
  } 
  cmd = state;

}

bool commandDetection(int pin){
  const unsigned long delay = 10;
  static unsigned long  last_date = 0 ; 
  static bool last_state = false; 
  unsigned long diff = millis() - last_date;
  bool result = false;
  bool input = digitalRead(pin);

  if(diff>delay && !last_state){
      result = true;
      last_state = true;
  }


  if (!input){ // reset counter 
    last_date = millis();
    last_state = false ;
  }
  return result;
  
}
