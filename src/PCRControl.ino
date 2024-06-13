
// #include "TemperatureSensor.hpp"
// #include "PIDv2.hpp"
// #include "PIDv3.h"


// #include <math.h>
// #include "custom.hpp"
// Custom CustomPCRControl; // creating instance of Custom Class to handle min and max funcitons
// // version is year.month.date.revision
// #define SOFTWARE_VERSION "2021.3.5.4"

// const int inA = 13; // pin connected to INA on VHN5019
// const int inB = 12; // pin connected to INB on VHN5019
// const int ppwm = 11; // pin connected to PWM on VHN5019
// const int fpwm = 10; // pin connected to PWM on fan
// const int thermP = A0; // pin connected to block thermal resistor neetwork. see elegooThermalResistorSch.png
// const int LidP = A1; // pin for thermal resistor conneccted to lid
// const int cPin = A2; // for curent recording
// const int ssr = 9; // solid state relay signal

// bool pPower = false; // software pielter on/off
// bool lPower = false; // software lid on/off

// bool verboseState = false; // spam serial with state every loop?
// bool verbosePID = false;  // spam serial with target and curent temperature?

// double peltierPWM = 0; // the PWM signal * curent direction to be sent to curent drivers for peltier
// int limitPWMH = 180;
// int limitPWMC = -180;
// double peltierPWMClamped= 0;

// float avgPTemp = 0; // last average for peltier temperature
// int avgPTempSampleSize = 100; // sample size for peltier temperature moving average

// float avgPPWM = 0; // last average for peltier temperature
// int avgPPWMSampleSize = 2; // sample size for peltier temperature moving average was 4

// double targetPeltierTemp = 29; // the tempature the system will try to move to, in degrees C
// double currentPeltierTemp; // the tempature curently read from the thermoristor connected to thermP, in degrees C
// bool windupTrigger = false; // this will trigger and kill the integral term if it gets too out of hand.

// double currentLidTemp; 
// double LastLidTemp;
// double LidOffset = 0;// offset calculated by 2 point temp test
// double BaseOffset = 0 ;// offset calculated by 2 point temp test

// //========================================================
// // calibration values determined with two point test
// float rawLowO = 1.93325; // average of 40 values taken in ice water
// float rawHighO = 90.65175; // average of 40 values taken at boiling
// float rawLowG = .3425;
// float rawHighG = 89.86975;

// float refHigh = 93.06; // boiling point of water in DURANGO CO
// float refLow = 0; // Freezing point of water

// // filling in regression models with empericly determined coeffeceints
// RegressionFit RedLOW = {79, 1.0266,-2.8129};
// RegressionFit RedHIGH = {79, 1.04932,-4.6107};
// RegressionFit BlackLOW = {66.9, 1.04689, 2.85};
// RegressionFit BlackHIGH = {66.9, 1.22624,-9.15267};
// //========================================================

// // setup pieltier tempature sensor
// TemperatureSensor peltierT(thermP);
// TemperatureSensor LidT(LidP); // JD setup for thermo resistor temp 

// const PIDtuning LID_PID_GAIN_SCHEDULE[] = {
//   //maxTemp, kP, kI, kD
//   { 70, 40, 0.15, 60 },
//   { 200, 80, 1.1, 10 }
// };

// PID Controller;
// // PIDv3* PIDcontroller;

// void setup() {
//   // setup serial
//   Serial.begin(115200);
//   while (!Serial) {
//     ; // wait for serial port to connect. Needed for native USB port only
//   }

//   // setup pins
//   pinMode(inA, OUTPUT);
//   pinMode(inB, OUTPUT);
//   pinMode(ppwm, OUTPUT);
//   pinMode(fpwm, OUTPUT);
//   pinMode(thermP, INPUT);
//   pinMode(LidP, INPUT);
//   pinMode(ssr, OUTPUT);
//   // set initial pin state to off
//   digitalWrite(inA, LOW);
//   digitalWrite(inB, LOW);
//   analogWrite(ppwm, 0);
//   analogWrite(fpwm, 0);
//   digitalWrite(ssr,LOW);
// }

// // checks UART serial for any commands and executes them
// void handleSerialInput() {
//   if (Serial.available() > 0) {
//     String incomingCommand = Serial.readString();
//     if (incomingCommand == "whoami\n") { // print out software ID
//       Serial.print("FLC-PCR software version: ");
//       Serial.print(SOFTWARE_VERSION);
//       Serial.print("\n");
//     }
//     if (incomingCommand == "verbose\n") { // toggle sending curent temp, pwm and current lid temp every loop
//       if (verboseState) {
//         verboseState = false;
//       } else {
//         verboseState = true;
//       }
//     }
//     if (incomingCommand == "pid\n") { // toggle sending target temp, curent temp and pwm every loop
//       if (verbosePID) {
//         verbosePID = false;
//       } else {
//         verbosePID = true;
//       }
//     }
//     if (incomingCommand == "d\n") { // request a single sample of the curent temp, pwm and lid temp
//       // request system data
//       Serial.print(avgPTemp);
//       Serial.print(" ");
//       Serial.print(avgPPWM);
//       Serial.print(" ");
//       Serial.print(currentLidTemp);
//       Serial.print("\n");
//     }
//     if (incomingCommand == "state\n") { // check weather or not the pieltiers are on or off
//       Serial.print(pPower);
//       Serial.print("\n");
//     }
//     if (incomingCommand == "offl\n") { // turn off lid
//       lPower = false;
//     } else if (incomingCommand == "offp\n") { // turn off pieltiers
//       pPower = false;
//     } else if (incomingCommand == "off\n") { // turn off both lid and pieltiers
//       pPower = false;
//       lPower = false;
//     }
//     if (incomingCommand == "onl\n") { // turn on the lid
//       lPower = true;
//     } else if (incomingCommand == "onp\n") { // turn on the pieltiers
//       pPower = true;
//       // PIDcontroller->reset();
//       Controller.reset();
//     } else if (incomingCommand == "on\n") { // turn on both lid and pieltiers
//       // PIDcontroller->reset();
//       Controller.reset();
//       pPower = true;
//       lPower = true;
//     }
//     // if (incomingCommand.substring(0,2) == "kp") { // set proportional gain
//     //   peltierPID.setKp(incomingCommand.substring(2).toFloat());
//     // }
//     // if (incomingCommand.substring(0,2) == "ki") { // set integral gain
//     //   peltierPID.setKi(incomingCommand.substring(2).toFloat());
//     // }
//     // if (incomingCommand.substring(0,2) == "kd") { // set derivitive gain
//     //   peltierPID.setKd(incomingCommand.substring(2).toFloat());
//     // }
//     if (incomingCommand.substring(0,2) == "pt") { // set pieltier temperature
//       // PIDcontroller->reset();
//       Controller.reset();
//       targetPeltierTemp = incomingCommand.substring(2).toFloat();
//     }
//     if (incomingCommand.substring(0,3) == "pia") { // set size of pieltier temperature low pass filter
//       avgPTempSampleSize = incomingCommand.substring(3).toInt();
//     }
//     if (incomingCommand.substring(0,3) == "poa") { // set size of pwm low pass filter
//       avgPPWMSampleSize = incomingCommand.substring(3).toInt();
//     }
//     if (incomingCommand.substring(0,3) == "plc") { // set the max pwm for coling
//       limitPWMC = incomingCommand.substring(3).toInt();
//     }
//     if (incomingCommand.substring(0,3) == "plh") { // set the max pwm for heating
//       limitPWMH = incomingCommand.substring(3).toInt();
//     }
//   }
// }

// void TempAq(){
//    if(targetPeltierTemp >= BlackHIGH.breakpoint){
//   currentLidTemp = BlackHIGH.slope*LidT.getTemp()+BlackHIGH.intercept;
//  }else{
//   currentLidTemp = BlackLOW.slope*LidT.getTemp()+BlackLOW.intercept;
//  }
//  if(targetPeltierTemp >= RedHIGH.breakpoint){
//   currentPeltierTemp = RedHIGH.slope*peltierT.getTemp()+RedHIGH.intercept;
//  }else{
//     currentPeltierTemp = RedLOW.slope*peltierT.getTemp()+RedLOW.intercept;
//  }
// if (isnan(currentPeltierTemp) || isinf(currentPeltierTemp)) { // reset nan and inf values
//     currentPeltierTemp = avgPTemp;
//   }

// avgPTemp = ((avgPTempSampleSize - 1) * avgPTemp + currentPeltierTemp) / avgPTempSampleSize; // average

// }

// void loop() {
//   handleSerialInput();
// ///////////////////////////////////////////TEMP SENSOR CALIBRATION///////////////////////////////////////////////////
//   // old temperature calibration
//   //currentPeltierTemp = 0.9090590064070043 * currentPeltierTemp + 3.725848396176527; // estimate vial temperature
//   //currentPeltierTemp = 0.6075525829531135 * currentPeltierTemp + 15.615801552818361; // seccond estimate

//   // 5/28/2024 temperature 2 point calibration calabration
//   // currentPeltierTemp = peltierT.CalibrationMath(rawHighO,rawLowO,refHigh,refLow,currentPeltierTemp);

//   ///6/3/2024 Temperature proved to be pretty accurate with a 6.6k resistor and the logrithmic conversion equation in the Temperaturesensor.getTemps()

// // 6/11/24 multiple linear regression using regressions loaded into the tuning structs
// // **NOTE** Red heatshrinked sensor is peltier sensor, and black heat shrinked sensor is lid temp
//  TempAq();

//   peltierPWM = Controller.calculate(targetPeltierTemp, avgPTemp); // calculate pid and set to output

//   // peltierPWM = PIDcontroller->Compute(targetPeltierTemp, avgPTemp); // calculate pid and set to output
//   // clamp output between -180 and 180. These were determined by the current driver having an output of 12 amps and the peltiers having a max of 9 amps with a bit of a margin below 9 amps (8.5/12 = 180/255 )
//   peltierPWMClamped = CustomPCRControl.myMin(double (limitPWMH), CustomPCRControl.myMax(double (limitPWMC), peltierPWM)); 
//   if (isnan(peltierPWM ) || isinf(peltierPWM )) { // reset nan and inf values
//     peltierPWM = avgPPWM;
//   }

//   avgPPWM = ((avgPPWMSampleSize - 1) * avgPPWM + peltierPWM) / avgPPWMSampleSize; // moving average of PWM

//   // Logic to determine anti windup. If things are looking weird it will set integral term to 0
//   // windupTrigger = peltierPID.windup(avgPPWM,peltierPWMClamped,targetPeltierTemp-avgPTemp);

//   // print out verbose data to serial if set
//   if (verboseState) {
//     Serial.print(avgPTemp);
//     Serial.print(" ");
//     Serial.print(avgPPWM);
//     Serial.print(" ");
//     Serial.print(currentLidTemp);
//     Serial.print("\n");
//   }
//   if (verbosePID) {
//     Serial.print(avgPTemp);
//     Serial.print(" ");
//     Serial.print(targetPeltierTemp);
//     Serial.print(" ");
//     Serial.print(avgPPWM);
//     Serial.print("\n");
//   }
// // lid control not used in current design
//   // // lid controll
//   // if (lPower) {
//   //   if(currentLidTemp < 70){ 
//   //     digitalWrite(ssr, HIGH);
//   //   } else {
//   //     digitalWrite(ssr, LOW);
//   //   }
//   // } else {
//   //   digitalWrite(ssr, LOW);
//   // }
  
//   // pieltier control
//   if (!pPower || currentPeltierTemp > 150) { // pieltiers on, shut down if over 150C
//     digitalWrite(inA, LOW);
//     digitalWrite(inB, LOW);
//     analogWrite(fpwm, 255);
//     analogWrite(ppwm, 0);

//     // PIDcontroller->reset();
//     Controller.reset();
//     return;
//   } else { // pieltiers on
//     // convert pieltierDelta to pwm, inA, inB
//     analogWrite(ppwm, abs(peltierPWM));
//     analogWrite(fpwm, 255);
//     if (peltierPWM > 0) {
//       digitalWrite(inA, HIGH);
//       digitalWrite(inB, LOW);
//     } else {// flips to cooling if PWM is below 0
//       digitalWrite(inA, LOW);
//       digitalWrite(inB, HIGH);
//     }
//   }
// }
#include <Arduino.h>
#include "PIDv1.hpp"
#include "TemperatureSensor.hpp"

const int inA = 13; // pin connected to INA on VHN5019
const int inB = 12; // pin connected to INB on VHN5019
const int ppwm = 11; // pin connected to PWM on VHN5019
const int tempPin = A0;

float targetTemp = 90;
float currentTemp;
float avgTemp;
int avgPTempSampleSize = 100; // sample size for peltier temperature moving average
int PWM;



TuningStruct testTuning = {40,.15,60};

// filling in regression models with empericly determined coeffeceints
RegressionFit RedLOW = {79, 1.0266,-2.8129};
RegressionFit RedHIGH = {79, 1.04932,-4.6107};
RegressionFit BlackLOW = {66.9, 1.04689, 2.85};
RegressionFit BlackHIGH = {66.9, 1.22624,-9.15267};

///////////////////////////////////////////////////FUNCTIONS//////////////////////////////////////////////////
// creating class instances and such
PIDv1 TestController(testTuning, 0, 200);
TemperatureSensor RedSensor(tempPin);

void TempAq(){

 if(targetTemp >= RedHIGH.breakpoint){
  currentTemp = RedHIGH.slope*RedSensor.getTemp()+RedHIGH.intercept;
 }else{
    currentTemp = RedLOW.slope*RedSensor.getTemp()+RedLOW.intercept;
 }
}
void PeltierControl(int PWM){
  //
  digitalWrite(inA,HIGH);
  digitalWrite(inB,LOW);
  analogWrite(ppwm, abs(PWM));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // Setup serial for debugging
  Serial.begin(115200);

  // Setup pins
  pinMode(inA, OUTPUT);
  pinMode(inB, OUTPUT);
  pinMode(ppwm, OUTPUT);

  // Set initial pin state to off
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);
  analogWrite(ppwm, 0);

  // Indicate setup is done
  Serial.println("Setup done");
}



void loop() {
  TempAq();
  PWM = TestController.calculate(targetTemp,currentTemp);
  PeltierControl(PWM);
  delay(100);
}
