/***************************************/
/* Bibliothèques de commande du triac  */
/***************************************/
#include "triac.h"
#include "settings.h"

hw_timer_t *timer = NULL;          // variable timer interne
volatile bool zc_detected = false; // variable de détection des passages par zéro
volatile unsigned long wdt_reset = 0;
bool interrupt_on = false;

bool pulse2=false;

void IRAM_ATTR RATriacClass::pulseTriac()
{
  if (wdt_reset > 0)
    wdt_reset++;
  if (wdt_reset > 800)
    ESP.restart(); // watchdog 4 secondes

  if (zc_detected) 
  {
    if (choixSortie == 0) digitalWrite(pinTriac, HIGH); 
#ifdef Sortie2
    if (choixSortie != 0) digitalWrite(pinSortie2, HIGH);
#endif   
    timerAlarmWrite(timer, 50, true); // reinitialise à 50us pour la largeur de l'impulsion
    timerWrite(timer, 0);
    timerStart(timer);
    zc_detected = false; 
 }
  else
  {
    if (choixSortie == 0)  digitalWrite(pinTriac, LOW); 
#ifdef Sortie2
    if (choixSortie != 0) digitalWrite(pinSortie2, LOW);
#endif   
    
     if (!pulse2) {
          zc_detected = true; // relance la 2eme pulse
          pulse2=true; 
#ifdef F60HZ
           timerAlarmWrite(timer,8300, true); // reinitialise à 10ms pour la largeur de l'impulsion
#endif

#ifdef F50HZ
  timerAlarmWrite(timer,9900, true); // reinitialise à 10ms pour la largeur de l'impulsion
#endif
          
          timerWrite(timer, 0);
          timerStart(timer);
                }   else timerStop(timer);

  }
}

// interruption passage par zéro
void IRAM_ATTR RATriacClass::zeroCross()
{
  if (zc_detected) return;
  if (choixSortie == 0)
    digitalWrite(pinTriac, LOW);
#ifdef Sortie2
  else
    digitalWrite(pinSortie2, LOW); // s'assure que le depart est a zero
#endif
#ifdef F60HZ
  const int MaxCommande = 620; // a verifier pour le 60Hz nombre d'impulsion max pour faire la periode
#endif

#ifdef F50HZ
  const int MaxCommande = 900; // 850
#endif

  if (puissanceGradateur > 0)
  {
    volatile unsigned int brightness = map(puissanceGradateur, 0, 1000, MaxCommande, 10); // angle d'amorçage des triacs
 
    zc_detected = true;
    pulse2=false; 
    timerStop(timer);
    timerAlarmWrite(timer, 10 * brightness, true); // 10us*1000 =10 ms, autoreload true   10
    timerAlarmEnable(timer);                       // enable
    timerWrite(timer, 0);
    timerStart(timer); // enable
  } else timerStop(timer);
}

void IRAM_ATTR RATriacClass::stop_interrupt()
{
  if (!interrupt_on)
    return;
  zc_detected = false;
  detachInterrupt(zeroc);
  if (choixSortie == 0)
    digitalWrite(pinTriac, LOW);
#ifdef Sortie2
  else
    digitalWrite(pinSortie2, LOW);
#endif
  timerStop(timer);
  timerWrite(timer, 0);
  interrupt_on = false;
}

void IRAM_ATTR RATriacClass::restart_interrupt()
{
  if (interrupt_on)
    return;
  attachInterrupt(zeroc, zeroCross, FALLING);
//  attachInterrupt(zeroc, zeroCross, RISING);
  interrupt_on = true;
}

void RATriacClass::start_interrupt()
{
  if (interrupt_on)
    return;
  timer = timerBegin(0, 80, true);                // timer de déclenchement triac
  timerAttachInterrupt(timer, &pulseTriac, true); // renvoi vers
  timerStop(timer);

  pinMode(zeroc, INPUT);
  attachInterrupt(zeroc, zeroCross, FALLING);
//  attachInterrupt(zeroc, zeroCross, RISING);
  interrupt_on = true;
}

void RATriacClass::watchdog(int i)
{
  wdt_reset = i;
}

RATriacClass RATriac;
