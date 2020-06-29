//**************************************************************/
// L'équipe de bidouilleurs discord reseautonome vous présente
// la réalisation d'un gradateur pour chauffe-eau en off grid
// gradateur synchronisé sur la production photovoltaique
// sans tirage sur les batteries pour utiliser le surplus dans
// un ballon d'eau chaude
//**************************************************************/

const char *version_soft = "Version 1.5 AC-DC avec 1 diode ZC"  ;

//**************************************************************/
// Initialisation
//**************************************************************/
#include "settings.h"
struct param routeur;
const int zeroc = 33;            // GPIO33  passage par zéro de la sinusoide
const int pinTriac = 27;         // GPIO27 triac
// analogique
const int pinPince = 32;         // GPIO32   pince effet hall
const int pinPinceAC = 32;         // GPIO32   pince effet hall
const int pinPinceACref = 39;    // GPIO39   ref tension pour pince AC 30A/1V
const int pinPinceRef = 34;      // GPIO34   pince effet hall ref 2.5V
const int pinPotentiometre = 35; // GPIO35   potentiomètre
const int pinTension = 36;       // GPIO36   capteur tension
const int pinTemp = 23;          // GPIO23  capteurTempérature
const int pinSortie2 = 13;       // pin13 pour  2eme "gradateur utilisée pour synchroniser le ZéroCrossing
const int pinRelais = 19;        // Pin19 pour sortie relais


bool marcheForcee = false;
short int marcheForceePercentage = 25;
short int sortieActive = 1;
unsigned long temporisation = 60;
float intensiteBatterie = 0;
float capteurTension = 0;
int puissanceGradateur = 0;
float temperatureEauChaude = 0;
float puissanceDeChauffe = 0;
float puissanceAC = 0;
float intensiteAC = 0;
bool etatRelaisStatique = false;
bool modeparametrage = false;

int resetEsp = 0;
int testwifi = 0;
int choixSortie = 0;
int paramchange = 0;
bool SAP = false;
bool MQTT = false;
bool serverOn = false;

/**********************************************/
/********** déclaration des librairiess *******/
/**********************************************/
#include "triac.h"

#ifdef WifiMqtt
#include "modemqtt.h"
#endif

#ifdef WifiServer
#include "modeserveur.h"
#endif

#ifdef EEprom
#include "prgEEprom.h"
#endif

#ifdef EcranOled
#include "afficheur.h"
#endif

#ifdef Bluetooth
#include "modeBT.h"
#endif

//#define simulation // utiliser pour faire les essais sans les accessoires
#ifdef simulation
#include "simulation.h"
#endif

#include "mesure.h"
#include "regulation.h"

/***************************************/
/******** Programme principal   ********/
/***************************************/
void setup()
{

  Serial.begin(115200);

  pinMode(pinTriac, OUTPUT);  digitalWrite(pinTriac, LOW); // mise à zéro du triac au démarrage
  pinMode(pinSortie2, OUTPUT);  digitalWrite(pinSortie2, LOW); // mise à zéro du triac de la sortie 2
  pinMode(pinRelais, OUTPUT);  digitalWrite(pinRelais, HIGH); // mise à zéro du relais statique
  pinMode(zeroc,INPUT);
  pinMode(pinTemp, INPUT);
  pinMode(pinTension, INPUT);
  pinMode(pinPotentiometre, INPUT);
  pinMode(pinPince, INPUT);
  pinMode(pinPinceRef, INPUT);
  pinMode(pinPinceACref, INPUT);
  Serial.println();
  Serial.println(F("definition ok "));
  Serial.println(version_soft);
  Serial.println();

#ifdef EcranOled
  RAAfficheur.setup();
#endif


#ifdef EEprom
  RAPrgEEprom.setup();
#endif

  delay(500);
  marcheForcee = false; // mode forcé retirer au démarrage
  marcheForceePercentage = false;
  temporisation = 0;
  
#ifdef WifiMqtt
  RAMQTT.setup();
#endif

#ifdef WifiServer
  RAServer.setup(); // activation de la page Web de l'esp
#endif

#ifdef Bluetooth // bluetooth non autorise avec serveur web ou MQTT
  RABluetooth.setup();
#endif

#ifdef EcranOled
  RAAfficheur.setup();
#endif

#ifdef MesureTemperature
  RAMesure.setup();
#endif

#ifdef Pzem04t
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // redefinition des broches du serial2
#endif

#ifdef parametrage
  modeparametrage = true;
#endif

#ifdef relaisPWM
  ledcAttachPin(pinRelais, 0); // broche 18, canal 0.
  ledcSetup(0, 5000, 10); // canal = 0, frequence = 5000 Hz, resolution = 10 bits 1024points
#endif


  delay(500);
  RATriac.watchdog(0);       // initialise le watchdog
  RATriac.start_interrupt(); // demarre l'interruption de zerocrossing et du timer pour la gestion du triac

}

int iloop = 0; // pour le parametrage par niveau


void loop()
{
  RATriac.watchdog(1);                  //chien de garde à 4secondes dans timer0
    RAMesure.mesureTemperature(); // mesure la temperature sur le ds18b20
    RAMesure.mesure_puissance();  // mesure la puissance sur le pzem004t
    intensiteBatterie=RAMesure.mesurePinceAC(pinPinceAC,0.350,true);

    RARegulation.pilotage();
     if (!marcheForcee) 
        {
          puissanceGradateur = RARegulation.regulGrad(-routeur.toleranceNegative,5,5,5);
         }
  
  // affichage traceur serie
 float c = puissanceGradateur;
  Serial.print(" ");
  Serial.print(c / 100);
   Serial.print(',');
  Serial.print(intensiteAC);
   Serial.print(',');
  Serial.println(-0.2);
 /* Serial.print(',');
  Serial.print(routeur.seuilDemarrageBatterie / 5);
  Serial.print(',');
  Serial.print(capteurTension / 5);
  Serial.print(',');
  Serial.println(-routeur.toleranceNegative);
*/
//  Serial.print(',');
//  Serial.print(intensiteAC) );
//  Serial.print(',');
//  Serial.println(puissanceAC/100);

//RAMesure.mesurePinceAC(pinPince,0.321,false);
//puissanceGradateur=300;

#ifdef relaisPWM
  ledcWrite(0, long(puissanceGradateur*1024/1000));  //  canal = 0, rapport cyclique = 
#endif

#ifdef EcranOled
 RAAfficheur.affichage_oled(); // affichage de lcd
#endif

#ifdef WifiMqtt
  RAMQTT.loop();
#endif

#ifdef WifiServer
  RAServer.loop();
  RAServer.coupure_reseau();
#endif


#ifdef EEprom
  
  if (resetEsp == 1)
  {
    RATriac.stop_interrupt();
    RAPrgEEprom.close_param(); 
    delay(5000);
  }
#endif

  if (resetEsp == 1)
  {
    Serial.println("Restart !");
    ESP.restart(); // redemarrage
  }
}
