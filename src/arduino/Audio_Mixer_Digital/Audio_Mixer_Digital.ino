/*******************************
  Audio Mixer Digital Firmware
  by AC
  Based on deej by Omri Harel
  version: pre-alpha 1.0
*******************************/
#define DEBUG 0
int print_time=0;
const int offset = 16;
const int NUM_SLIDERS = 1;
const char clkInputs[NUM_SLIDERS] = {4};
const char dtInputs[NUM_SLIDERS] = {3};
const char swInputs[NUM_SLIDERS] = {12};

int sliderValues[NUM_SLIDERS];
bool currentStateCLK[NUM_SLIDERS];
bool previousStateCLK[NUM_SLIDERS];

void setup() { 
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(clkInputs[i], INPUT);
    pinMode(dtInputs[i], INPUT);
    pinMode(swInputs[i], INPUT);
  }

  Serial.begin(9600);
}

void loop() {
  updatesliderValues();
  sendsliderValues(); // Actually send data (all the time)
  delay(1);
}

void updatesliderValues() {

  for (int i = 0; i < NUM_SLIDERS; i++) {
    
   String encdir ="";
   
     // Read the current state of clkInputs
   currentStateCLK[i] = digitalRead(clkInputs[i]);
    
   // If the previous and the current state of the inputCLK are different then a pulse has occured
   if (currentStateCLK[i] != previousStateCLK[i]){ 
       
     // If the inputDT state is different than the inputCLK state then 
     // the encoder is rotating counterclockwise
     if (digitalRead(dtInputs[i]) != currentStateCLK[i]) {  
       if (sliderValues[i]<(1023)){sliderValues[i]++;}          
     } else {
       // Encoder is rotating clockwise
       if (sliderValues[i]>0){sliderValues[i]--;}
     }
   } 
   // Update previousStateCLK with the current state
   previousStateCLK[i] = currentStateCLK[i];
  }
}

void sendsliderValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)sliderValues[i]);

    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  
  Serial.println(builtString);
}

void printsliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(sliderValues[i]);
    Serial.write(printedString.c_str());

    if (i < NUM_SLIDERS - 1) {
      Serial.write(" | ");
    } else {
      Serial.write("\n");
    }
  }
}
