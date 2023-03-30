/*******************************
  AutoMixer Extended Firmware
  Based on deej by Omri Harel
  version: alpha 1.1
*******************************/
/*  Pinout */

#include <TM1637.h>
//#include <iostream>
#include <math.h>

#define SLIDER_0 A0 //A0
#define SLIDER_1 A1 //A1
#define SLIDER_2 A2 //A2
#define SLIDER_3 A3 //A3

#define MUTE_BTN_0 2 //D2
#define MUTE_BTN_1 4 //D4
#define MUTE_BTN_2 7 //D7
#define MUTE_BTN_3 8 //D8

#define ROTPOT_0 A4 //A4 
#define ROTPOT_1 A5 //A5
#define ROTPOT_2 A6 //A6
#define ROTPOT_3 A7 //A7 //Master

#define DEV_BTN_0 10 //D10
#define DEV_BTN_1 11 //D11
#define DEV_BTN_2 12 //D12

#define PWM_0 3 //D3
#define PWM_1 5 //D5
#define PWM_2 6 //D6

#define DISP_CLK 9 //D9
#define DISP_DIO 13 //D13

#define SMA_WINDOW_SIZE 10

const uint16_t ADC_RESOLUTION = 1023; 
const uint8_t NUM_SLIDERS = 4; //and mute buttons
const uint8_t NUM_ROTPOT = 4;
const uint8_t NUM_INPUTS = NUM_SLIDERS + NUM_ROTPOT;
const uint8_t NUM_DEVBTN = 3; //and pwmLEDs
const unsigned int BLINK_PERIOD = 500; //ms
const unsigned int BLINK_TIMES_LIMIT = 5; //ms

const int analogInputs[NUM_INPUTS] = {SLIDER_0, SLIDER_1, SLIDER_2, SLIDER_3, ROTPOT_0, ROTPOT_1, ROTPOT_2, ROTPOT_3};
const int muteBtns[NUM_SLIDERS] = {MUTE_BTN_0, MUTE_BTN_1, MUTE_BTN_2, MUTE_BTN_3};
const int devBtns[NUM_DEVBTN] = {DEV_BTN_0, DEV_BTN_1, DEV_BTN_2};
const int pwmLEDs[NUM_DEVBTN] = {PWM_0, PWM_1, PWM_2};

unsigned long blinkLastTime;
uint16_t times_blinked[NUM_DEVBTN];

//adc reading values
uint16_t analogValues[NUM_INPUTS];
uint16_t lastAnalogValues[NUM_INPUTS];
//sma array
int smaWindow[SMA_WINDOW_SIZE] = {0};

//max and min values for adc calibration
uint16_t analogMaxValues[NUM_INPUTS] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 994};
uint16_t analogMinValues[NUM_INPUTS] = {0, 0, 0, 0, 0, 0, 0, 53};

//device button status
bool devBtnStatus[NUM_DEVBTN] = {0,0,0};
//bool prevDevBtnStatus[NUM_DEVBTN] = {0,0,0};
int ledStatus[NUM_DEVBTN]; //status 0: off, 1:blink on, 2:blink off, 3: on


TM1637 tm(DISP_CLK, DISP_DIO);

void setup() {

  Serial.begin(9600);
  Serial.write("Initializing...");  

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
  Serial.write("Setup Complete");  
  delay(1000);

  HelloWorld();
}

void loop() {
  updateValues();
  updateDevBtnStatus();
  updateLEDs();
  //sendValues(); // Actually send data (all the time)
  
  //printAllValues();
  printInputValues(); // For debug
  delay(10);
  //read current time
  long current_time = millis()
  if (current_time=)

}

void updateValues() {
  
  for (uint8_t i = 0; i < NUM_INPUTS; i++) {
    analogValues[i] = analogRead(analogInputs[i]);
  }
  
  //check for mute buttons
  for (uint8_t i = 0; i < NUM_SLIDERS; i++) {
    if (!digitalRead(muteBtns[i])) { //the mute button is pressed
      analogValues[i] = 0;
    }
  }
    uint8_t i=7;
    uint8_t disp_value = getSliderPercent(i)/10;    
      tm.display(disp_value*10);
  
  /*
  for (int i = NUM_SLIDERS; i < NUM_INPUTS; i++) { //this mutes the track just like the mute buttons do with the dev buttons
    if (devBtnStatus[i]==1) { //the dev button was pressed
      analogValues[i] = 0;
    }
  }
  */
}

void sendValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_INPUTS; i++) {
    builtString += String((int)analogValues[i]);

    if (i < NUM_INPUTS - 1) {
      builtString += String("|");
    }
  }
  
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

void printAllValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_DEVBTN; i++) {
    builtString += String((bool)(devBtnStatus[i]));

    if (i < NUM_DEVBTN - 1) {
      builtString += String("|");
    }
  }

  Serial.println(builtString);
}

void updateDevBtnStatus() {
  
    for (int i = 0; i < NUM_DEVBTN; i++) {
      if(digitalRead(devBtns[i])==LOW) { //TODO: change logic
        
        digitalWrite(pwmLEDs[i],HIGH);
        while(digitalRead(devBtns[i])==LOW); //wait for release
        delay(10);
        
        devBtnStatus[i] = !devBtnStatus[i];
        ledStatus[i]=1;
      }  
    }
}

void updateLEDs () {
  
  for (int i = 0; i < NUM_DEVBTN; i++) {
    //Serial.print("blinkLastTime= ");
    //Serial.println(blinkLastTime);
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
            //Serial.print("blink off \n");
            }

          if (times_blinked[i]>=BLINK_TIMES_LIMIT) {
            ledStatus[i]= 3;
            times_blinked[i]=0;} //end blinking
          else {times_blinked[i]++;} //blinked another time
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

void HelloWorld() {
  tm.display("0000");
    
}

uint8_t getSliderPercent (uint8_t i){

  uint32_t adc_value = analogValues[i];
  uint32_t adc_lastvalue = lastAnalogValues[i];
  uint16_t adc_max = analogMaxValues[i];
  uint16_t adc_min = analogMinValues[i];
  uint8_t disp_num = (((adc_value - adc_min + 5) * 100) / (adc_max - adc_min + 5)); //using 5 as an offset to get 100 with values lower than 1023 
  if (disp_num > 100) { disp_num = 100;} // just to be safe
  if (disp_num < 0) { disp_num = 0;} // just to be safe
  return disp_num;
  
}

/*
void sma_push (int val){
  int i=0;
  for (i=1;i<SMA_WINDOW_SIZE-1; i++) {
    smaWindow[i-1]=smaWindow[i];
  }
  smaWindow[SMA_WINDOW_SIZE-1]=val;
}

void sma_takeAvg (int val){
  long sum= 0;
  int i=0;
  for(i=0;i<SMA_WINDOW_SIZE)



}

uint16_t sma (uint8_t adc_pin){
  uint8_t numSamples = 10;
  int avg[numSamples]={0};
  int i=0;
  //10bit (0-1023)
  unsigned int   analog_val = 0;
  unsigned int cur_avg = 0;  
  long wholeSum = 0;

  for (i=0; i<numSamples; i++) {
    wholeSum += analogRead(adc_pin);
  }
  
  cur_avg = wholeSum / numSamples;

  return cur_avg;
}
*/