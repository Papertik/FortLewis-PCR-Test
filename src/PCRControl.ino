#include "TemperatureSensor.hpp"
#include "PIDv1.hpp"

// version is year.month.date.revision
#define SOFTWARE_VERSION "2024.6.17.4"

const int inA = 13; // pin connected to INA on VHN5019 ( current driver )
const int inB = 12; // pin connected to INB on VHN5019 ( current driver )
const int ppwm = 11; // pin connected to PWM on VHN5019
const int fpwm = 10; // pin connected to PWM on fan
const int thermP = A0; // pin connected to block thermal resistor network
const int LidP = A1; // pin for thermal resistor connected to lid
const int ssr = 9; // solid state relay signal

bool pPower = false; // software peltier on/off
bool lPower = false; // software lid on/off

int highClamp = 225; // PWM clamp for high temps
int lowClamp = 150; // PWM clamp for low temps

bool verboseState = false; // spam serial with state every loop?
bool verbosePID = false;  // spam serial with target and current temperature?

double peltierPWM = 0; // the PWM signal * current direction to be sent to current drivers for peltier
float avgTemp = 0; // last average for peltier temperature
int avgTempSampleSize = 15; // sample size for peltier temperature moving average

float avgPPWM = 0; // last average for peltier temperature
int avgPPWMSampleSize = 2; // sample size for peltier temperature moving average

double targetTemp = 0; // the temperature the system will try to move to, in degrees C
double currentPeltierTemp = 0; // the temperature currently read from the thermistor connected to thermP, in degrees C
double currentLidTemp = 0;

unsigned long startTime;

//===============================class creation and struct filling======================================
// filling in regression models with empirically determined coefficients
RegressionFit RedLOW = {79, 1.0266, -2.8129}; // linear regression coefficients for red sensor LOW range
RegressionFit RedHIGH = {79, 1.04932, -4.6107}; // linear regression coefficients for red sensor HIGH range
RegressionFit BlackLOW = {66.9, 1.04689, 2.85}; // linear regression coefficients for black sensor LOW range
RegressionFit BlackHIGH = {66.9, 1.22624, -9.15267}; // linear regression coefficients for black sensor HIGH range
// setup peltier temperature sensor
TemperatureSensor peltierT(thermP);
TemperatureSensor LidT(LidP); // setup for thermo resistor temp

// set up tuning models for PID controller
TuningStruct lowTune = {20, 0.02, 860}; // tuning for low range temps
TuningStruct highTune = {30, 0.015, 1000}; // tuning for high range temps (above 75°C)

PIDv1 LowController(lowTune, 0, lowClamp); // creating a PID controller to handle low temps
PIDv1 HighController(highTune, 0, highClamp); // creating a PID controller to handle high temps

//===============================function declarations======================================

// checks UART serial for any commands and executes them
 void handleSerialInput() {
  if (Serial.available() > 0) {
    String incomingCommand = Serial.readString();
    if (incomingCommand == "whoami\n") { // print out software ID
      Serial.print("FLC-PCR software version: ");
      Serial.print(SOFTWARE_VERSION);
      Serial.print("\n");
    }
    if (incomingCommand == "verbose\n") { // toggle sending curent temp, pwm and current lid temp every loop
      if (verboseState) {
        verboseState = false;
      } else {
        verboseState = true;
      }
    }
    if (incomingCommand == "pid\n") { // toggle sending target temp, curent temp and pwm every loop
      if (verbosePID) {
        verbosePID = false;
      } else {
        verbosePID = true;
      }
    }
    if (incomingCommand == "d\n") { // request a single sample of the curent temp, pwm and lid temp
      // request system data
      Serial.print(avgTemp);
      Serial.print(" ");
      Serial.print(avgPPWM);
      Serial.print(" ");
      Serial.print(currentLidTemp);
      Serial.print("\n");
    }
    if (incomingCommand == "state\n") { // check weather or not the pieltiers are on or off
      Serial.print(pPower);
      Serial.print("\n");
    }
    if (incomingCommand == "offl\n") { // turn off lid
      lPower = false;
    } else if (incomingCommand == "offp\n") { // turn off pieltiers
      pPower = false;
    } else if (incomingCommand == "off\n") { // turn off both lid and pieltiers
      pPower = false;
      lPower = false;
    }
    if (incomingCommand == "onl\n") { // turn on the lid
      lPower = true;
    } else if (incomingCommand == "onp\n") { // turn on the pieltiers
      pPower = true;
      LowController.reset();
      HighController.reset();
    } else if (incomingCommand == "on\n") { // turn on both lid and pieltiers
     LowController.reset();
    HighController.reset();
      pPower = true;
      lPower = true;
    }
    if (incomingCommand.substring(0,2) == "pt") { // set pieltier temperature
      LowController.reset();
      HighController.reset();
      targetTemp = incomingCommand.substring(2).toFloat();
    }
    if (incomingCommand.substring(0,3) == "pia") { // set size of pieltier temperature low pass filter
      avgTempSampleSize = incomingCommand.substring(3).toInt();
    }
    if (incomingCommand.substring(0,3) == "poa") { // set size of pwm low pass filter
      avgPPWMSampleSize = incomingCommand.substring(3).toInt();
    }
  }
}

// function to acquire, calibrate and average raw temperatures
void TempAcquisition() {
  // getting and calibrating from black (lid) sensor
  if (targetTemp >= BlackHIGH.breakpoint) {
    currentLidTemp = BlackHIGH.slope * LidT.getTemp() + BlackHIGH.intercept;
  } else {
    currentLidTemp = BlackLOW.slope * LidT.getTemp() + BlackLOW.intercept;
  }
  // getting and calibrating from red (peltier) sensor
  if (targetTemp >= RedHIGH.breakpoint) {
    currentPeltierTemp = RedHIGH.slope * peltierT.getTemp() + RedHIGH.intercept;
  } else {
    currentPeltierTemp = RedLOW.slope * peltierT.getTemp() + RedLOW.intercept;
  }

  if (isnan(currentPeltierTemp) || isinf(currentPeltierTemp)) { // reset nan and inf values
    currentPeltierTemp = avgTemp;
  }
  avgTemp = ((avgTempSampleSize - 1) * avgTemp + currentPeltierTemp) / avgTempSampleSize; // moving average of peltier temp
}

// loop to convert PID PWM output to peltier
void PeltierControl(int PWM, float temp) {
  if (temp >= 100) {
    analogWrite(ppwm, 0); // shouldn't need any temps higher than 100
  } else {
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);
    analogWrite(ppwm, abs(PWM));
  }
}

// control lid, simple conditional statement turns it off if over 70°C
void LidControl(float temp) {
  if (lPower) {
    if (temp < 70) {
      digitalWrite(ssr, HIGH);
    } else {
      digitalWrite(ssr, LOW);
    }
  } else {
    digitalWrite(ssr, LOW);
  }
}

// loop to conditionally determine the appropriate PID to use based on target temperature
float ChoosePID(int targetTemp) {
  if (targetTemp >= 80) {
    return HighController.calculate(targetTemp, avgTemp);
  } else {
    return LowController.calculate(targetTemp, avgTemp);
  }
}

//======================================================================================================================

void setup() {
  startTime = millis();
  // setup serial
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // setup pins
  pinMode(inA, OUTPUT);
  pinMode(inB, OUTPUT);
  pinMode(ppwm, OUTPUT);
  pinMode(fpwm, OUTPUT);
  pinMode(thermP, INPUT);
  pinMode(LidP, INPUT);
  pinMode(ssr, OUTPUT);
  // set initial pin state to off
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);
  analogWrite(ppwm, 0);
  analogWrite(fpwm, 0);
  digitalWrite(ssr, LOW);
}

void loop() {
  unsigned long currentTime = millis() - startTime; // get current time for plotting purposes
  handleSerialInput();
  TempAcquisition();
  peltierPWM = ChoosePID(targetTemp); // chooses a PID controller based on the target temp.

  // print out verbose data to serial if set
    // print out verbose data to serial if set
  if (verboseState) {
    Serial.print(avgTemp);
    Serial.print(" ");
    Serial.print(avgPPWM);
    Serial.print(" ");
    Serial.print(currentLidTemp);
    Serial.print("\n");
  }
  if (verbosePID) {
    Serial.print(avgTemp);
    Serial.print(" ");
    Serial.print(targetTemp);
    Serial.print(" ");
    Serial.print(avgPPWM);
    Serial.print("\n");
  }
  LidControl(currentLidTemp);
  PeltierControl(peltierPWM, currentPeltierTemp);
}
