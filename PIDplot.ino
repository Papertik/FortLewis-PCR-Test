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
int targetTemp = 70;
int lastTarget;
int highClamp = 200;
int lowClamp = 175;
float input, output;
double currentPeltierTemp;
unsigned long startTime;
float avgTempSampleSize = 15;
float avgTemp = 0;

RegressionFit RedLOW = {79, 1.0266,-2.8129};
RegressionFit RedHIGH = {79, 1.04932,-4.6107};

TemperatureSensor RedSensor(thermP);
TuningStruct lowTune = {25,.02,860};
TuningStruct highTune = {25,.02,840};
PIDv1 HighController(highTune, 0,highClamp);// creating instance of the controller
PIDv1 LowController(lowTune, 0, lowClamp); // creating a controller for low temps

//////////////////////////////////////////////functions/////////////////////////////////////////
int SetPointLoop(int set1, int set2, int set3){
    unsigned long currentTime = millis() - startTime;
    if(currentTime>=120000 && currentTime<=2*120000){
      return set2;
    }else if(currentTime>=2*120000&&currentTime<=3*120000){
      return set3;
    }else{
      return set1;
    }
}
// this function grabs calibrates and averages temperatures from the thermistors
void TempAq(){
 if(targetTemp >= RedHIGH.breakpoint){
  currentPeltierTemp = RedHIGH.slope*RedSensor.getTemp()+RedHIGH.intercept;
 }else{
    currentPeltierTemp = RedLOW.slope*RedSensor.getTemp()+RedLOW.intercept;
 }
 avgTemp = ((avgTempSampleSize - 1) * avgTemp + currentPeltierTemp) / avgTempSampleSize; // average
}
void PeltierControl(int PWM, float temp){// loop to convert PID PWM output to peltier.
  if(temp>=100) analogWrite(ppwm,0);// shouldnt need any temps higher than 100
  digitalWrite(inA,HIGH);
  digitalWrite(inB,LOW);
  analogWrite(ppwm, abs(PWM));
  // analogWrite(fpwm, (255-PWM));
}
// loop to conditionally determine the appropriate PID to use
float ChoosePID(int targetTemp){
  if(targetTemp>=80){  
    return HighController.calculate(targetTemp,avgTemp);
}else{
    return LowController.calculate(targetTemp,avgTemp);
}
}
//////////////////////////////////////////////////////////////////////////////////////////////
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
  targetTemp = SetPointLoop(55,80,90);// function to change setpoints
  TempAq();
  output = ChoosePID(targetTemp);
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