#include "PPMEncoder.h"
#include <WS2812FX.h>

// IO DEFINITION
#define PPM_OUT_PIN 10
#define CMD_PIN 12
#define DECOR_IN_PIN 6
#define LED_PIN 11

// RC channels pins 
#define ST_CH 7 // CH1
#define TH_CH 8 // CH2
#define CH3_ 9

bool decor_cmd = false ; 

// RC data struct
struct rc {
 long int ST;
 long int TH;
 long int CH3;
};

rc g2tb ;

// Strip LEDS
#define LED_COUNT 3
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

void setup() {
    initRC();
    pinMode(CMD_PIN, OUTPUT);
    pinMode(DECOR_IN_PIN, INPUT);
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

    // Strip LEDs init
    ws2812fx.init();
    ws2812fx.setBrightness(150);
    ws2812fx.setSpeed(9000);
    ws2812fx.setMode(FX_MODE_RAINBOW);
    ws2812fx.start();

}

void initRC(void){
  pinMode(ST_CH, INPUT);
  pinMode(TH_CH, INPUT);
  pinMode(CH3_, INPUT);
}

void loop() {
  RcToPPM();
  decor_ctrl();
}


// RC TO PPM CONVERSION
unsigned long readRcChannel(int pin){
    return pulseIn(pin, HIGH);
}

void readRC(void){
  g2tb.ST = readRcChannel(ST_CH);
  g2tb.TH = readRcChannel(TH_CH);
  g2tb.CH3 = readRcChannel(CH3_);
}

void RcToPPM(void){
  // PPM conversion
  readRC();
  ppmEncoder.setChannel(3, g2tb.ST);
  ppmEncoder.setChannel(2, g2tb.TH);
  ppmEncoder.setChannel(4, g2tb.CH3);
}


void decor_ctrl(void){
  // Strip LEDs control
  ws2812fx.service();
  
  
  //bool bRE = risingEdge(DECOR_IN_PIN);
  //toggle(bRE);
  //toggle(commandDetection(DECOR_IN_PIN)); 
  decor_cmd = digitalRead(DECOR_IN_PIN);
  if (decor_cmd){
    digitalWrite(CMD_PIN,HIGH);
    digitalWrite(LED_BUILTIN,HIGH);
    //ws2812fx.start();
  }
  else{
    digitalWrite(CMD_PIN,LOW);
    digitalWrite(LED_BUILTIN,LOW);
    //ws2812fx.stop();
  }
}

bool risingEdge(int pin){
  static bool lastState = false;
  bool state;
  bool input = digitalRead(DECOR_IN_PIN);
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
  decor_cmd = state;

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