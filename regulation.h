/***************************************/
/******** pilotage et regulation   ********/
/***************************************/
#ifndef RA_REGULATION_H
#define RA_REGULATION_H
class RARegulationClass
{

public:
  int mesureDerive(float x, float seuil);

  int regulGrad(float consigne, float Kp, float Ki, float Kd);

  /**************************************/
  /******** Pilotage exterieur*******/
  /**************************************/
  int pilotage();
};
extern RARegulationClass RARegulation;
#endif
