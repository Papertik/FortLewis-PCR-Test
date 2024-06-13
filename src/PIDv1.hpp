#include <Arduino.h>
// a stripped down simple PID controller for debugging purposes
struct TuningStruct{
  float kp;
  float ki;
  float kd;
};

class PIDv1{
private:
float kp;
float ki;
float kd;
int highClamp;
int lowClamp;
float error=0, previousError = 0;
float P,I,D;
int output =0;


public:
PIDv1 (TuningStruct tuning, int minOutput, int maxOutput){
  highClamp = maxOutput;
  lowClamp = minOutput;
  kp = tuning.kp;
  ki = tuning.ki;
  kd = tuning.kd;
}


// function to calculate the PID stuff
int calculate(double targetTemp, double currentTemp){
  error = (targetTemp)-currentTemp;
  P = error;
  I +=error;
  D = error-previousError;
  previousError=error;
  float rawOutput = kp*P+ki*I+kd*D;
  if (rawOutput > highClamp) {
    rawOutput = highClamp;
  } else if (rawOutput < lowClamp) {
    rawOutput = lowClamp;
  }
    // Anti-windup: only integrate if the output is not at the bounds
  if (rawOutput >= highClamp || rawOutput <= lowClamp) {
      I -= error;
  }
  output = static_cast<int>(rawOutput);
  return output;
}
void reset(){
  P=0;
  I=0;
  D=0;
}

};