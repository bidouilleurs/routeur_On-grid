# routeur_On-grid

Version du routeur DC modifié avec 1 diode pour connaitre le sens du transfert de puissance ( sens du courant ) 

-les entrées de la pince DC sont utilisées pour raccorder des pinces AC ( 30A, 1V) 

-la régulation se fait sur le courant inverse (injection) règlé avec le tolérance-négative 

-sur la page web il faut lire Le Courant AC dans intensité batterie 

-la fonction relais statique et thermostat sont inversé si la température du ballon descent en dessous d'une valeur, le relais s'enclenche 

-on pourra donc l'associer au relais Heures creuses pour maintenir la température si le soleil n'est pas au rendez-vous 

-Les informations MQTT restent identiques

Fichiers modifiés du routeur off-grid
source.ino
triac.cpp ( tient compte de la diode ) 
mesure.cpp et .h
regulation.cpp et .h
plus de simulation.cpp
le reste est identique 
