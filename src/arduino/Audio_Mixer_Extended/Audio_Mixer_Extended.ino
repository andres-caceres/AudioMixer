/*******************************
  AutoMixer Extended Firmware
  Based on deej by Omri Harel
  version: beta 1.3 >> toggle
*******************************/


#include <TM1637.h>
//#include <iostream>
#include <math.h>
//#include <EEPROM.h>

/*  Pinout */

#define SLIDER_0 A3 //A0
#define SLIDER_1 A2 //A1
#define SLIDER_2 A1 //A2
#define SLIDER_3 A0 //A3

#define MUTE_BTN_0 2 //D2
#define MUTE_BTN_1 4 //D4
#define MUTE_BTN_2 7 //D7
#define MUTE_BTN_3 8 //D8

#define ROTPOT_0 A4 //A4 //Master
#define ROTPOT_1 A5 //A5
#define ROTPOT_2 A6 //A6
#define ROTPOT_3 A7 //A7 

#define DEV_BTN_0 10 //D10
#define DEV_BTN_1 11 //D11
#define DEV_BTN_2 12 //D12

#define PWM_0 3 //D3
#define PWM_1 5 //D5
#define PWM_2 6 //D6

#define DISP_CLK 9 //D9
#define DISP_DIO 13 //D13

#define DEFAULT_BRIGHTNESS 2 //for screen

const uint16_t ADC_RESOLUTION = 1023; 
const uint8_t NUM_SLIDERS = 4; //and mute buttons
const uint8_t NUM_ROTPOT = 4;
const uint8_t NUM_INPUTS = NUM_SLIDERS + NUM_ROTPOT;
const uint8_t NUM_DEVBTN = 3; //and pwmLEDs
const unsigned int BLINK_PERIOD = 100; //ms
const unsigned int BLINK_TIMES_LIMIT = 5; //times

const int analogInputs[NUM_INPUTS] = {SLIDER_0, SLIDER_1, SLIDER_2, SLIDER_3, ROTPOT_0, ROTPOT_1, ROTPOT_2, ROTPOT_3};
const int muteBtns[NUM_SLIDERS] = {MUTE_BTN_0, MUTE_BTN_1, MUTE_BTN_2, MUTE_BTN_3};
const int devBtns[NUM_DEVBTN] = {DEV_BTN_0, DEV_BTN_1, DEV_BTN_2};
const int pwmLEDs[NUM_DEVBTN] = {PWM_0, PWM_1, PWM_2};

unsigned long blinkLastTime;
uint16_t times_blinked[NUM_DEVBTN];

//adc reading values
uint16_t analogValues[NUM_INPUTS];
bool  isMuted[NUM_INPUTS];
uint16_t lastAnalogValues[NUM_INPUTS];

//display
uint8_t previousValue[NUM_INPUTS];
unsigned long previousTime = 0;
unsigned long currentTime = 0;
const unsigned int DISPLAY_THRESHOLD = 2;    // set threshold for detecting ADC value change
const unsigned int DISPLAY_TIMEOUT = 3000;    // display threshold in seconds
bool isLocked[NUM_INPUTS] = {true,true,true,true,true,true,true,true};

//ExpAvgFilter
const float alpha = 0.2; //lower is slower
double lastAvg[NUM_INPUTS];

//max and min values for adc calibration
const uint16_t analogMaxValues[NUM_INPUTS] = {1009, 1009, 1009, 1009, 1021, 1021, 1021, 1021};
const uint16_t analogMinValues[NUM_INPUTS] = {30, 30, 30, 30, 0, 0, 0, 0};

//device button status
bool devBtnStatus[NUM_DEVBTN] = {0,0,0};
bool prevDevBtnStatus[NUM_DEVBTN] = {0,0,0};
int ledStatus[NUM_DEVBTN]; //status 0: off, 1:blink on, 2:blink off, 3: on
String toggle = String("");

//leds
int brightness[NUM_DEVBTN]={255,255,255};

TM1637 tm(DISP_CLK, DISP_DIO);

void setup() {

  Serial.begin(19200);
  while(!Serial); //wait for serial ready
  Serial.println("Initializing...");

  //all sliders and pots
  for (int i = 0; i < NUM_INPUTS; i++) {
    pinMode(analogInputs[i], INPUT);
  }

  //mute btns
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(muteBtns[i], INPUT);
  }

  //dev btns
  for (int i = 0; i < NUM_DEVBTN; i++) {
    pinMode(devBtns[i], INPUT);
  }

  //leds
  for (int i = 0; i < NUM_DEVBTN; i++) {
    pinMode(pwmLEDs[i], OUTPUT);
  }
  
  //Display
  pinMode(DISP_CLK, OUTPUT);
  pinMode(DISP_DIO, OUTPUT);

  tm.begin();
  tm.setBrightness(DEFAULT_BRIGHTNESS);
  Serial.println("IO Setup Complete");
  delay(100);
  Serial.println("Initializing Values..."); 
  initValues();
  Serial.println("Done");
  Serial.println("Running welcome animation...");
  LEDWelcome();
  Serial.println("Done");
  Serial.println("Streaming Data...");
}

void loop() {
  updateValues();
  updateScreen();
  updateDevBtnStatus();
  updateDevBtnLEDs2();
  //printInputValues();
  sendValues(); // Send data
  //delay(20);
}

void updateValues() {

  for (uint8_t i = 0; i < NUM_INPUTS; i++) {
    analogValues[i] = readADC(i);
  }
  
  //check for mute buttons
  for (uint8_t i = 0; i < NUM_SLIDERS; i++) {
    isMuted[i] = (bool) digitalRead(muteBtns[i]);
  }

  //for (uint8_t i = NUM_SLIDERS+1; i < NUM_INPUTS; i++) {
  //  isMuted[i] = devBtnStatus[i-(NUM_SLIDERS+1)];
  //}

}

void initValues() {

  for (uint8_t i = 0; i < NUM_SLIDERS; i++) {
    isMuted[i] = (bool) digitalRead(muteBtns[i]);
  }

  for (uint8_t i = NUM_SLIDERS+1; i < NUM_INPUTS; i++) {
    isMuted[i] = 0;
  }
  
  for(int j=0;j<40;j++){
    for (uint8_t i = 0; i < NUM_INPUTS; i++) {
      analogValues[i]=readADC(i);
      lastAnalogValues[i]= analogValues[i];
      previousValue[i]= getSliderPercent(i);
    }
  }
}

void sendValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_INPUTS; i++) {
    builtString += String((int)(analogValues[i]*(int)(!isMuted[i]))); //affected by isMuted

    if (i < NUM_INPUTS - 1) {
      builtString += String("|");
    }
  }

  builtString += String("\;");
  builtString += toggle;
  toggle = String(""); //revert back to empty string
  
  Serial.println(builtString);
}

void printInputValues() {
  for (int i = 0; i < NUM_INPUTS; i++) {
    String printedString = String("CH#") + String(i + 1) + String(": ") + String(" DISP=")+ String(getSliderPercent(i)) + String(" ADC=") + String(analogValues[i]);
    Serial.write(printedString.c_str());

    if (i < NUM_INPUTS - 1) {
      Serial.write(" | ");
    } else {
      Serial.write("\n");
    }
  }
}

void printDevBtnValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_DEVBTN; i++) {
    builtString += String((bool)(devBtnStatus[i]));

    if (i < NUM_DEVBTN - 1) {
      builtString += String("|");
    }
  }

  Serial.println(builtString);
}

void printisMuted() {
  String builtString = String("");

  for (int i = 0; i < NUM_INPUTS; i++) {
    builtString += String(isMuted[i]);

    if (i < NUM_INPUTS - 1) {
      builtString += String("|");
    }
  }

  Serial.println(builtString);
}

void updateDevBtnStatus() {
  
   for (int i = 0; i < NUM_DEVBTN; i++) { //starts in i=1 because of hardware fault with devbutton 0, and now is also used for device switching
    if(i==0){
      if(digitalRead(devBtns[i])==LOW) {
      
        digitalWrite(pwmLEDs[i],HIGH);
        while(digitalRead(devBtns[i])==LOW); //wait for release
        delay(10);
        
        devBtnStatus[i] = !devBtnStatus[i];
        toggle = String("toggle");
        ledStatus[i]=1;
      }      
    } else {
      if(digitalRead(devBtns[i])==HIGH) {
      
        digitalWrite(pwmLEDs[i],HIGH);
        while(digitalRead(devBtns[i])==HIGH); //wait for release
        delay(10);
        
        devBtnStatus[i] = !devBtnStatus[i];
        ledStatus[i]=1;
      }      
    }             
  }
}

void updateDevBtnLEDs () {
  
  for (int i = 0; i < NUM_DEVBTN; i++) {
      switch (ledStatus[i]) {
        case 0:  //led off
          digitalWrite(pwmLEDs[i],LOW);
          break;

        case 1:  //led blinking on          
          if (millis() >= (blinkLastTime+(BLINK_PERIOD))){
            ledStatus[i] = 2;
            blinkLastTime = millis();
            //Serial.print("blink off \n");
            }
          digitalWrite(pwmLEDs[i],HIGH);
          break;

        case 2: //led blinking off
          digitalWrite(pwmLEDs[i],LOW);
          if (millis() >= (blinkLastTime+(BLINK_PERIOD))) {
            ledStatus[i] = 1;
            blinkLastTime=millis();
            times_blinked[i]++;
            //Serial.print("blink off \n");
          }

          if (times_blinked[i]>=BLINK_TIMES_LIMIT) {
            ledStatus[i]= 3;
            times_blinked[i]=0;
          } //end blinking
          break;

        case 3:          
          // for (int j = 0; j < NUM_DEVBTN; i++) { //turn off all j leds
          //   ledStatus[j] = 0;
          //   digitalWrite(pwmLEDs[j],LOW);
          // }
          // blinkLastTime=millis();
          // ledStatus[i] = 3;  //except this one, i
          // digitalWrite(pwmLEDs[i],HIGH)
          digitalWrite(pwmLEDs[i],LOW);
        default:
          ledStatus[i]=0;
          break;
      }
    }
}

void updateDevBtnLEDs2 () {
  
  for(int i=0; i< NUM_DEVBTN; i++){    
    if(ledStatus[i]==1){
      if(brightness[i]>=0){
        analogWrite(pwmLEDs[i],brightness[i]);
        brightness[i]=brightness[i]-5;
      } else {        
        brightness[i]=255;
        ledStatus[i]=0;
      } 
    }
  }  
}

void LEDWelcome() {
  
  //pwmledbtn welcome for 3 leds
  for(int i=0;i<NUM_DEVBTN;i++){brightness[i]=0;}//set brightness values to 0
  
  //start led sequence
  for(int i=NUM_DEVBTN-1;i>=0;i--){
    while(brightness[i]<80){      
      analogWrite(pwmLEDs[i],brightness[i]);
      brightness[i]++;
      delay(2);
    }
  }
  while(brightness[0]<=255){   
    for(int i=NUM_DEVBTN-1;i>=0;i--){
      analogWrite(pwmLEDs[i],brightness[i]);
      brightness[i]++;
      delay(2);
    }
  }
  for(int i=0;i<NUM_DEVBTN;i++){brightness[i]=255;} //reset brightness values to 255
  while(brightness[0]>0){   
    for(int i=NUM_DEVBTN-1;i>=0;i--){
      analogWrite(pwmLEDs[i],brightness[i]);
      brightness[i]--;
      delay(2);
    }
  }  
  for(int i=0;i<NUM_DEVBTN;i++){brightness[i]=255;} //reset brightness values to 255

  delay(10);
  digitalWrite(PWM_0,LOW);
  digitalWrite(PWM_1,LOW);
  digitalWrite(PWM_2,LOW);
  
  //start same led sequence 2 times
  // for (int j=0; j<2 ;j++){
  //   //reset brightness values to 0 and display values
  //   for(int i=0;i<NUM_DEVBTN;i++){ 
  //     brightness[i]=0;
  //     analogWrite(pwmLEDs[i],brightness[i]);
  //   }
  //   delay(4);
  //   //same led sequence but fast
  //   for(int i=NUM_DEVBTN-1;i>=0;i--){
  //     while(brightness[i]<80){      
  //       analogWrite(pwmLEDs[i],brightness[i]);
  //       brightness[i]++;
  //       delay(2);        
  //     }    
  //   }    
  // }
  tm.setBrightness(DEFAULT_BRIGHTNESS);
}

void screenWelcome() {
  
  // welcome display animation
  int t = 60; //delay time
  const uint8_t rawBuffer[6][4] = {
    {DisplayDigit().setE(), 0x00, 0x00, DisplayDigit().setB()},
    {DisplayDigit().setF(), 0x00, 0x00, DisplayDigit().setC()},
    {DisplayDigit().setA(), 0x00, 0x00, DisplayDigit().setD()},
    {0x00, DisplayDigit().setA(), DisplayDigit().setD(), 0x00},
    {0x00, DisplayDigit().setD(), DisplayDigit().setA(), 0x00},
    {DisplayDigit().setD(), 0x00, 0x00, DisplayDigit().setA()},
    };
  auto d = DisplayDigit().setA().setE().setF().setD();
  const uint8_t ring[4] = {d, 0x09, DisplayDigit().setA().setD(), 15};
  const uint8_t dash[4] = {0x40, 0x40, 0x40, 0x40};
  tm.clearScreen();
  tm.setBrightness(2);
  //main display animation loop
  for(int i=0; i<60 ; i++){
    for(int j =0; j<2;j++){
      for(int k=0; k<6 ; k++){
        tm.displayRawBytes(rawBuffer[k], 4);
        delay(t);
      }
       if(t>=10){
        t=t-10;
      } else {
        if(t>=5){t--;}
      }
    }
  }
  //display end flash
  tm.setBrightness(1);
  tm.displayRawBytes(ring, 4);
  delay(60);
  tm.setBrightness(2);
  tm.displayRawBytes(ring, 4); 
  delay(40);
  tm.setBrightness(3);
  tm.displayRawBytes(ring, 4);
  delay(20);
  tm.setBrightness(4);
  tm.displayRawBytes(ring, 4);
  delay(20);
  tm.setBrightness(3);
  tm.displayRawBytes(dash, 4);
  delay(10);
  tm.setBrightness(1);
  tm.displayRawBytes(dash, 4);
  delay(10);
  tm.clearScreen();
  tm.setBrightness(DEFAULT_BRIGHTNESS);
}


uint16_t getSliderPercent (uint8_t i){

  uint32_t adc_value = analogValues[i];
  //uint16_t adc_max = analogMaxValues[i];
  //uint16_t adc_min = analogMinValues[i];
  //uint8_t disp_num = (((adc_value - adc_min + 5) * 100) / (adc_max - adc_min + 5)); //using 5 as an offset to get 100 with values lower than 1023 
  //if (disp_num > 100) { disp_num = 100;} // just to be safe
  //if (disp_num < 0) { disp_num = 0;} // just to be safe

  uint16_t disp_num = (adc_value*100)/1023;
  return disp_num;
}

// Exponential Moving Average filter implementation for i ADCs
double expMovingAverage(uint8_t i) {
  int cur_sample = 0;
  double cur_avg = 0.0;
  cur_sample = analogRead(analogInputs[i]);
  cur_avg = ((cur_sample*alpha) + (lastAvg[i])*(1-alpha));
  lastAvg[i] = cur_avg;
  delayMicroseconds(200);
  return lastAvg[i];
}

// Reades ADCs with i pin index and does a linear interpolation with max and min adc values
uint16_t readADC(uint8_t i){
  uint32_t adc_value = expMovingAverage(i);
  uint16_t adc_max = analogMaxValues[i];
  uint16_t adc_min = analogMinValues[i];
  uint16_t offset = 1;
  long reading = map(adc_value,adc_min+offset,adc_max,0,1023);
  if (reading > 1023) { reading = 1023;} // just to be safe
  if (reading < 0) { reading = 0;} // just to be safe
  return (uint16_t) reading;
}

// Reades ADCs with i pin index and does a linear interpolation with max and min adc values
uint16_t readADC_raw(uint8_t i){
  uint16_t adc_value = expMovingAverage(i);
  return (uint16_t) adc_value;
}

// Updates the TM1637 Screen when a display value changes, waits for screen timeout
void updateScreen(){
  for (int i = 0; i<NUM_INPUTS; i++){

    uint8_t num=getSliderPercent(i);    
    if (num != previousValue[i]){ //check for value change
    //display num
      if (!isMuted[i]){
        displayNumberAndCh(num,i+1);
      } else {
        displayMuted(i+1);
      }
    previousTime= millis();
    previousValue[i]= num;
    } else {
      currentTime = millis();
      if ((currentTime - previousTime) >= DISPLAY_TIMEOUT){ //check for inactivity timeout
        tm.clearScreen();
        tm.setDp(4);
      }
    }   
  }
}

void displayNumber(uint8_t num) {

    if (num<10){
      tm.display(num,true,true,3);    
    }
    else if(num<100){
      tm.display(num,true,true,2);    
    }
    else{
      tm.display(num,true,true,1);
    }
}

void displayNumberAndCh(uint16_t num, uint8_t ch) {

  String mystring = "";

  if (num<10){
    mystring = String(ch) + String("  ") + String(num);   
  }
  else if(num<100){
    mystring = String(ch) + String(" ") + String(num);    
  }
  else{
    mystring = String(ch) + String(num);      
  }
  tm.display(mystring,true,true,0);
}

void displayMuted(uint8_t ch){
  
  String mystring = String(ch) + String(" --");
  tm.display(mystring,true,true,0);
}