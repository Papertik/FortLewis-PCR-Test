// class for creating a pid system
#include <Arduino.h>
#include "custom.hpp"
Custom my;
class PID {
  private:
  double kp; // higher moves faster
  double ki; // higher fixes ofset and faster
  double kd; // higher settes faster but creastes ofset
  unsigned long currentTime, lastTime; // the time in millisecconds of this timesep and the last timestep
  double pError, lError, iError, dError; // error values for pid calculations. p: porportional, l: last, i: integerl, d: derivitive
  double iErrorLimit = 255; // maximum value for acumulated error
  // also try resetting the accumulated error to 0 if the command changes
  
  public:
  PID (double proportionalGain = 1, double integralGain = 0, double derivativeGain = 0) {
    kp = proportionalGain; // higher moves faster
    ki = integralGain; // higher fixes ofset and faster
    kd = derivativeGain; // higher sets faster but creastes ofset and amplifies noise
  }

  void reset(){
    iError = 0;
  }
  
  double calculate(double currentTemp, double targetTemp, bool antiWindup) { // performs pid calculation returns error
    lastTime = currentTime;
    currentTime = millis();
    lError = pError;
    pError = targetTemp - currentTemp;
    dError = (pError - lError) / (double)(currentTime - lastTime) / 1000;

    if(antiWindup)
    {
      iError = 0;
}else{
      iError = my.myMin(my.myMax(pError * (double)(currentTime - lastTime) / 1000 + iError, -iErrorLimit), iErrorLimit);
}
    return kp * pError + ki * iError + kd * dError;
  }

  bool windup(double UpstreamPWM, double DownstreamPWM, double inputError){
    bool SATcheck;
    bool sameSign;
    if(UpstreamPWM != DownstreamPWM){
      SATcheck=true;
    } else { SATcheck = false;}
    sameSign = (UpstreamPWM * inputError >= 0);
    if(sameSign && SATcheck){
      return true;
    }else{
      return false;
    }
  }
  
  void setKp(float value) {
    kp = value;
  }
  
  void setKi(float value) {
    ki = value;
  }
  
  void setKd(float value) {
    kd = value;
  }
};