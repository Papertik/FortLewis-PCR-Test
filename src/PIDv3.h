
struct PIDtuning{
  int maxValueInclusive;
  double kP;
  double kI;
  double kD;
};

class PIDv3{
  public:
  PIDv3(const PIDtuning* pGainSchedule, int minOuptut, int maxOutput);

  double GetIntegrator(){ return iIntegrator;}

  double Compute(double target, double currentValue);

  void reset();

  private:
    // adjusts tuning
    const PIDtuning* DetermineGainSchedule(double target);
    // clamping function
    void LatchValue(double* pvalue, double minValue, double maxValue);

private:
  const PIDtuning* ipGainSchedule;
  int iMinOutput, iMaxOutput;
  double iPreviousError;
  double iIntegrator;
};