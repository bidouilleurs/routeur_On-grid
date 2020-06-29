/***************************************/
/******** pilotage et regulation   ********/
/***************************************/
#include "regulation.h"
#include "settings.h"
#include <Arduino.h>
float xmax = 0;
float xmin = 0;
int devlente = 0;
int devdecro = 0;


int RARegulationClass::mesureDerive(float x, float seuil)
{
  int dev = 0;
  return dev;
}

int calPuis = 0;
int calPuisav = 0;
int devprevious = 0;
int devcount = 0;
int puisGradmax = 0;
int tesTension = 0;
int chargecomp = 0;
#define variation_lente 1    // config 5
#define variation_normale 10 // config 10
#define variation_rapide 20  // config 20
#define bridePuissance 900   // sur 1000


  float integraldif=0;
  int ineg = 0;
  float difav=0;

int RARegulationClass::regulGrad(float consigne, float Kp, float Ki, float Kd)
{
   float dif=consigne - intensiteAC;  // consigne - mesure
   float prop = Kp*dif;

 if (dif<0) ineg++; else ineg=0;
  if (ineg>15) integraldif=0;

  integraldif+=dif;
  if (abs(integraldif)>100) integraldif-=dif;
  float integ=Ki*integraldif;

  float deri=Kd*(dif-difav);
  if ((deri<0)&& (dif>0)) deri=10*deri;
  
    calPuis = prop + integ + deri;
   calPuis = min(max(0, calPuis), bridePuissance);
  return (calPuis);
}

/**************************************/
/******** Pilotage exterieur*******/
/**************************************/
unsigned long tempdepart;
int tempo = 0;

int RARegulationClass::pilotage()
{
  // pilotage du 2eme triac
#ifndef MesureTemperature
#ifdef Sortie2
#ifdef Pzem04t
  if (routeur.utilisation2Sorties)
  {
    if ((puissanceDeChauffe == 0) && (puissanceGradateur > 100))
      tempo2 = ++;
    else
      tempo2 = 0; // demarre la tempo chauffe-eau temp atteinte
    if (tempo2 > 10)
    {
      choixSortie = 1;
      sortieActive = 2;
      tempo2++;
    } // après 2s avec i=0 bascule sur triac2
    if (tempo2 > 200)
    {
      choixSortie = 0;
      sortieActive = 1;
      tempo2 = 0;
    } // après qques minutes bascule sur 1er triac
  }
#endif
#endif
#endif

#ifdef MesureTemperature
#ifdef Sortie2
  if (routeur.utilisation2Sorties)
  {
    if ((temperatureEauChaude > routeur.temperatureBasculementSortie2) && (choixSortie == 0))
    {
      choixSortie = 1;
      sortieActive = 2;
    }
    // commande du gradateur2
    if ((temperatureEauChaude < routeur.temperatureRetourSortie1) && (choixSortie == 1))
    {
      choixSortie = 0;
      sortieActive = 1;
    }
    // commande du gradateur1
  }
  else
  {
    choixSortie = 0;
    sortieActive = 1;
  }
#endif

  if (routeur.relaisStatique && strcmp(routeur.tensionOuTemperature, "D") == 0)
  {
    if (temperatureEauChaude < routeur.seuilMarche)
    {
      digitalWrite(pinRelais, LOW); // mise à un du relais statique
      etatRelaisStatique = true;
    }
    if ((temperatureEauChaude > routeur.seuilArret) && etatRelaisStatique)
    {
      digitalWrite(pinRelais, HIGH); // mise à zéro du relais statique
      etatRelaisStatique = false;
    }
  }
#endif

  if (routeur.relaisStatique && strcmp(routeur.tensionOuTemperature, "V") == 0)
  {
    if (capteurTension > routeur.seuilMarche)
    {
      digitalWrite(pinRelais, LOW); // mise à un du relais statique
      etatRelaisStatique = true;
    }
    if (capteurTension < routeur.seuilArret)
    {
      digitalWrite(pinRelais, HIGH); // mise à zéro du relais statique
      etatRelaisStatique = false;
    }
  }
  if ((marcheForcee) && (tempo == 0))
  {
    tempdepart = millis(); //  memorisation de moment de depart
    tempo = 1;
  }
  if ((marcheForcee) && (tempo == 1))
  {
    puissanceGradateur = marcheForceePercentage * 9; //limitation 90%
    if (millis() > tempdepart + 60000)
    { // decremente toutes les minutes
      tempdepart = millis();
      temporisation--;
      paramchange = 1;
    }                       // durée de forcage en milliseconde
    if (temporisation == 0) // fin de temporisation
    {
      marcheForcee = false;
      tempo = 0;
    }
  }
}

RARegulationClass RARegulation;
