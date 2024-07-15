#include "PPMEncoder.h"
#include <WS2812FX.h>

// IO DEFINITION
#define PPM_OUT_PIN 10
#define LED_PIN 11
#define CMD_PIN 12
#define DECOR_PULSE_OUT_PIN 5
#define DECOR_IN_PIN 6


// RC channels pins 
#define ST_CH 7 // CH1
#define TH_CH 8 // CH2
#define CH3_ 9

bool decor_cmd = false ; 
#define TRIG_PULSE_US 1800
#define TRIG_MAX_PULSE_US 1900

#define SERIAL_BAUD 115200

// RC data struct
struct rc {
 long int ST;
 long int TH;
 long int CH3;
};

rc g2tb ;


// Strip LEDS
#define LED_COUNT 10
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

void setup() {
    initRC();
    pinMode(CMD_PIN, OUTPUT);
    pinMode(DECOR_IN_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(DECOR_PULSE_OUT_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN,LOW);

    Serial.begin(SERIAL_BAUD);
    
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
    ws2812fx.setBrightness(255);
    ws2812fx.setSpeed(9000);
    ws2812fx.setMode(FX_MODE_STATIC);
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
  //dispRC();
  ppmEncoder.setChannel(3, g2tb.ST);
  ppmEncoder.setChannel(2, g2tb.TH);
  ppmEncoder.setChannel(4, g2tb.CH3);
}

void dispRC(void){
  Serial.print("ST: "); Serial.print(g2tb.ST); Serial.print("  TH: "); Serial.print(g2tb.TH); Serial.print("  CH3: "); Serial.println(g2tb.CH3);
}


void decor_ctrl(void){
  // Strip LEDs control
  ws2812fx.service();
  stripSeq();
  //ws2812fx.start();
  
  //bool bRE = risingEdge(DECOR_IN_PIN);
  //toggle(bRE);
  //toggle(commandDetection(DECOR_IN_PIN)); 
  //decor_cmd = digitalRead(DECOR_IN_PIN);
  unsigned long decor_in_pulse = pulseIn(DECOR_IN_PIN,HIGH);
  bool request = (decor_in_pulse>TRIG_PULSE_US) && (decor_in_pulse <TRIG_MAX_PULSE_US);
  static bool last_request = false;
  
  //decor_cmd = (decor_in_pulse>TRIG_PULSE_US);
  bool trig_cmd= risingEdge(last_request, request) ; 
  last_request = request;

  if (trig_cmd){
    //Serial.println(decor_in_pulse);
    ws2812fx.start();
  }
  toggle(trig_cmd);
  
  //control pulse deco
  if (request){
    digitalWrite(DECOR_PULSE_OUT_PIN,HIGH);  
    digitalWrite(LED_BUILTIN,HIGH);   
  }
  else {
    digitalWrite(DECOR_PULSE_OUT_PIN,LOW);
    digitalWrite(LED_BUILTIN,LOW);
  }
  
  // control toggle deco 
  if (decor_cmd){
    digitalWrite(CMD_PIN,HIGH);      
  }
  else{
    digitalWrite(CMD_PIN,LOW);
    ws2812fx.stop();
  }
}

bool risingEdge(bool prev, bool new_){
  return (!prev && new_);
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


void stripSeq(void){
  static int step = 1; 
  int step_nbr = 3;
  int main_delay = 8000; // ms
  static int _delay = main_delay; // ms
  static unsigned long   _last_date = 0;
  unsigned long date = millis();
  unsigned long  _timer = date - _last_date ; 
  
  if (_timer>_delay){
    _last_date = date;
    step++;
    if (step>step_nbr){
      step = 1;
    }
    switch (step){
    case 1:
      ws2812fx.setMode(FX_MODE_COLOR_WIPE);
      ws2812fx.setColor(WHITE);
      ws2812fx.setSpeed(2500);
      break;
    case 2:
      ws2812fx.setMode(FX_MODE_FADE);
      ws2812fx.setColor(WHITE);
      ws2812fx.setSpeed(2500);
      break;
    case 3:
      ws2812fx.setMode(FX_MODE_RAINBOW);
      ws2812fx.setSpeed(2500);
      break;
    }
  }
}