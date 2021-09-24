#include <LiquidCrystal.h>

/*Primitive tachometer based on reed switch sensor,
 although it may be usable with other sensor types aswell.
 
 Author: Max Borglowe*/

uint8_t tachoInput = A5;
uint8_t inputSignal = 0;
uint8_t led = A0;
uint16_t count = 0, totalCounter = 0,       /* out of order*/ measureTime = 1000; 
uint8_t prevPulses = 0, currPulses = 0;
uint8_t pulsesPerRevolution = 2; //= how many stimulators (magnets) are attached to the rotor
uint8_t analogEdge = 32;

uint8_t filter = 1;
uint16_t rpm = 0;
float rps = 0;
const uint8_t RPM_SAMPLES = 5;
uint16_t rpm_array[RPM_SAMPLES];

uint8_t i = 0;
float mean = 0;

/*LCD stuff*/
uint8_t rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
  
void setup() {
  for(uint8_t k = 0; k < RPM_SAMPLES; k++){
    rpm_array[k] = 0;
  }
  
  Serial.begin(115200);
  pinMode(tachoInput, INPUT);
  pinMode(led, OUTPUT);
  LiquidCrystal(rs, en, d4, d5, d6, d7);
  
}

void loop() {
  prevPulses = currPulses;
  currPulses += detectMagnet();
  
  totalCounter += count + 1;
  
  rps = (float)currPulses/pulsesPerRevolution*(float)measureTime/totalCounter;
  rpm = rps*60;
  
  /*print status every second*/
  if(totalCounter >= measureTime){ //approximal second lol
    calcMeanRPM();

    Serial.print("current RPS: ");
    Serial.print(rps);
    Serial.print(", current RPM: ");
    Serial.print(rpm);
    Serial.print(", mean RPM: ");
    Serial.println(mean);
    totalCounter = 0;
    currPulses = 0;
    mean = 0;
  }
  count = 0;
  delay(1);
}

/**Calculate mean RPM based on the last [RPM_SAMPLES] measurements. */
void calcMeanRPM(){
  rpm_array[i] = rpm;
  for(uint8_t k = 0; k < RPM_SAMPLES; k++){
      mean += rpm_array[k];
    }
    mean = mean/RPM_SAMPLES;
    i = (i + 1)%RPM_SAMPLES;
}

/**Detects stimulator (magnet) passing the sensor,
filtering out arbitrarily quick pulses that may occur.*/
uint8_t detectMagnet(){
  //latching when magnet passes sensor
  while(analogRead(tachoInput) > analogEdge){ //reasonable midpoint to keep out noise
    count++;
    digitalWrite(led, HIGH);
    delay(1);
  }
  digitalWrite(led, LOW);
  
  if(count <= filter){ //arbitrary spike filter
    return 0;
  }
  return 1;
}
