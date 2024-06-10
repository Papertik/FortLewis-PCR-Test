#include "PIDv3.h"
// next iteration of PID controller to go here
PIDv3::PIDv3(const PIDtuning* pGainSchedule, int minOutput, int maxOutput):
  ipGainSchedule(pGainSchedule),
  iMinOutput(minOutput),
  iMaxOutput(maxOutput),
  iIntegrator(0),
  iPreviousError(0){
  }
//===========================================================================================
double PIDv3::Compute(double target, double currentValue){
  // calculate output values for PID loop
  const PIDtuning* pPIDtuning = DetermineGainSchedule(target);
  double error = target - currentValue;
  
  //perform basic PID calculation
  double pTerm = error;
  double iTerm = iIntegrator + error;
  double dTerm = error - iPreviousError;
  double output = (pPIDtuning->kP * pTerm) + (pPIDtuning->kI * iTerm) + (pPIDtuning->kD * dTerm);  
  
  //reset integrator if pTerm maxed out in drivable direction
  if ((iMaxOutput && pTerm * pPIDtuning->kP > iMaxOutput) ||
   (iMinOutput && pTerm * pPIDtuning->kP < iMinOutput)) {
    iIntegrator = 0;
  //accumulate integrator if output not maxed out in drivable direction
  } else if ((iMinOutput == 0 || output > iMinOutput) && 
             (iMaxOutput == 0 || output < iMaxOutput)) {
    iIntegrator += error;
  }
  
  //latch integrator and output value to controllable range
  LatchValue(&iIntegrator, iMinOutput, iMaxOutput);
  LatchValue(&output,iMinOutput,iMaxOutput);

  //update values for next derivative computation
  iPreviousError = error;

  return output;
}
//======================================================================================
const PIDtuning* PIDv3::DetermineGainSchedule(double target) {
  const PIDtuning* pGainScheduleItem = ipGainSchedule;
  
  while (target > pGainScheduleItem->maxValueInclusive)
    pGainScheduleItem++;
    
  return pGainScheduleItem;
}
//======================================================================================
void PIDv3::LatchValue(double* pValue, double minValue, double maxValue) {
  if (*pValue < minValue)
    *pValue = minValue;
  else if (*pValue > maxValue)
    *pValue = maxValue;
}
//======================================================================================
void PIDv3::reset(){
  iIntegrator = 0;
}