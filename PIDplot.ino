// Include necessary libraries
#include <math.h>
#include <Arduino.h>
#include "TemperatureSensor.hpp"
#include "PIDv1.hpp"

// Pin definitions
const int inA = 13;
const int inB = 12;
const int ppwm = 11;
const int thermP = A0;

// Control variables
float targetTemp = 50;
float input, output;
double currentPeltierTemp;
unsigned long startTime;
float avgTempSampleSize = 15;
float avgTemp = 0;

RegressionFit RedLOW = {79, 1.0266,-2.8129};
RegressionFit RedHIGH = {79, 1.04932,-4.6107};

TemperatureSensor RedSensor(thermP);
TuningStruct testTuning = {20,.02,860};
PIDv1 Controller(testTuning, 0,150);

void TempAq(){
 if(targetTemp >= RedHIGH.breakpoint){
  currentPeltierTemp = RedHIGH.slope*RedSensor.getTemp()+RedHIGH.intercept;
 }else{
    currentPeltierTemp = RedLOW.slope*RedSensor.getTemp()+RedLOW.intercept;
 }
 avgTemp = ((avgTempSampleSize - 1) * avgTemp + currentPeltierTemp) / avgTempSampleSize; // average
}
void PeltierControl(int PWM, float temp){
  if(temp>=100) analogWrite(ppwm,0);// shouldnt need any temps higher than 100
  digitalWrite(inA,HIGH);
  digitalWrite(inB,LOW);
  analogWrite(ppwm, abs(PWM));
  // analogWrite(fpwm, (255-PWM));
}

void setup() {
  Serial.begin(115200);
  pinMode(inA, OUTPUT);
  pinMode(inB, OUTPUT);
  pinMode(ppwm, OUTPUT);
  pinMode(thermP, INPUT);


  startTime = millis();
}

void loop() {
  unsigned long currentTime = millis() - startTime;

  TempAq();
  output = Controller.calculate(targetTemp,avgTemp);
  PeltierControl(output,currentPeltierTemp);

  // Output in a format the Serial Plotter can understand
    // Output in CSV format to Serial
    Serial.print(currentTime);
    Serial.print(",");
    Serial.print(avgTemp);
    Serial.print(",");
    Serial.print(targetTemp);
    Serial.print(",");
    Serial.println(output);
  delay(200);
}