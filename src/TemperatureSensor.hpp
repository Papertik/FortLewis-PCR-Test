// for creating a temperature sensor
// see elegooThermalResistorSch.png for wireing
#include <arduinoSTL.h>

class TemperatureSensor {
  private:
  int pin;
  public:
  TemperatureSensor(int iPin) { 
    pin = iPin; // the pin that conected to the tempature network
  }

  float getTemp() { // returns the tempature from thermoristor connected to thermP in degrees C, includes noise reduction
    int tempReading = analogRead(pin);
    double tempK = log(6600 * ((1024.0 / tempReading - 1)));
    tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK ); // kelvin
    return (tempK - 273.15); // convert kelvin to celcius
  }
  // two point calibration
  float CalibrationMath(float highIN, float lowIN, float highRef, float lowRef, float DataIN){
    float RawRange;
    float ReferenceRange;
    float CorrectedValue;
    RawRange = highIN - lowIN;
    ReferenceRange = highRef - lowRef;                                                                             
    CorrectedValue = (((DataIN - lowIN) * ReferenceRange) / RawRange) + lowRef;
    return CorrectedValue;
}

};