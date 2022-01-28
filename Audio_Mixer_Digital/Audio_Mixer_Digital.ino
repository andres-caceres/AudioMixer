/*******************************
  Audio Mixer Digital Firmware
  Based on deej by Omri Harel
  version: pre-alpha 1.0
*******************************/

#define DEBUG 0

const int NUM_ENCODERS = 5;
const int clkInputs[NUM_SLIDERS] = {A0, A1, A2, A3, A4};
const int dtInputs[NUM_SLIDERS] = {A0, A1, A2, A3, A4};
const int swInputs[NUM_SLIDERS] = {A0, A1, A2, A3, A4};

int values[NUM_ENCODERS];

void setup() { 
  for (int i = 0; i < NUM_ENCODERS; i++) {
    
    pinMode(clkInputs[i], INPUT);
    pinMode(dtInputs[i], INPUT);
    pinMode(swInputs[i], INPUT);
    
  }

  Serial.begin(9600);
}

void loop() {
  
  updateValues();
  sendValues(); // Actually send data (all the time)
  printValues(); // For debug
  
  delay(10);
}

void updateValues() {
  for (int i = 0; i < NUM_ENCODERS; i++) {

     currentStateCLK = digitalRead(clkInputs);
     
     if (currentStateCLK != previousStateCLK){
        
        if (digitalRead(inputDT) != currentStateCLK) {
           // Encoder is rotating counterclockwise
           counter --;
        }
        else {
           // Encoder is rotating clockwise
           counter ++;
        }
     }
     values[i] =counter;
  }
}

void sendSliderValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)analogSliderValues[i]);

    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  
  Serial.println(builtString);
}

void printSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(analogSliderValues[i]) + String(" mV");
    Serial.write(printedString.c_str());

    if (i < NUM_SLIDERS - 1) {
      Serial.write(" | ");
    } else {
      Serial.write("\n");
    }
  }
}
