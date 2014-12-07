#include <wiringPi.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <sched.h>
#include <sstream>
#include "RCSwitch.h"

// Pour compiler, récurérer RCSwitch.h et RCSwitch.cpp depuis https://github.com/ninjablocks/433Utils/tree/master/RPi_utils
// Puis lancer g++ -o radioReception RCSwitch.cpp radioReception.cpp

// Utilisation: sudo ./radioReception http://[yana_serveur]:[yana_port]/[yana-folder]/action.php
// Ex: sudo ./radioReception http://localhost/yana-server/action.php

using namespace std;

RCSwitch mySwitch;
int pin;

void log(string a){
	//Décommenter pour avoir les logs
	cerr << a << endl;
}

string longToString(long mylong){
    stringstream mystream;
    mystream << mylong;
    return mystream.str();
}

string floatToString(float mylong){
    stringstream mystream;
    mystream << mylong;
    return mystream.str();
}

int main (int argc, char** argv)
{
	string command = "wget -O - \"";
	command.append(argv[1]);
	pin = atoi(argv[2]);
	string idx = argv[3];
	log("Demarrage du programme");

    	if(wiringPiSetup() == -1)
    	{
        	log("Librairie Wiring PI introuvable, veuillez lier cette librairie...");
        	return -1;
    	}
    	mySwitch = RCSwitch();
    	mySwitch.enableReceive(pin);
	log("Pin GPIO configure en entree");
	
    	log("Attente d'un signal du transmetteur ...");
	for(;;)
    	{
		string varcmd = "";
    		float temperature = 0;
		unsigned long emiter = 0;
		unsigned long positive = 0;
		
    		if (mySwitch.available()) {
    
        		int value = mySwitch.getReceivedValue();
           		if (value == 0) {
          			log("Encoding inconnu");
        		} else {    
			        emiter = mySwitch.getReceivedValue() & 15; //masque sur les 4 derniers bits
			        positive = (mySwitch.getReceivedValue() >> 4) & 1; // decalage de 4 à droite et masque sur le dernier bit
			        temperature = (float)(mySwitch.getReceivedValue() >> 5) / (float)100; //decalage de 5 digits à droite
			        log("------------------------------");
				log("Donnees detectees");
				log("temperature = " + floatToString(temperature));
				log("positif = " + longToString(positive));
				log("code sonde = " +longToString(emiter));
				
				varcmd.append("?type=command&param=udevice&idx="+idx);
				//varcmd.append("&capteur="+longToString(emiter));
				//varcmd.append("&value="+floatToString(temperature));
				varcmd.append("&nvalue=0");
				if (positive==1) {
					varcmd.append("&svalue="+floatToString(temperature));
				} else {
					varcmd.append("&svalue=-"+floatToString(temperature));
				}
				varcmd.append("\" > /dev/zero 2>&1");

				log("Execution de la commande PHP...");
				log((command+varcmd).c_str());
				system((command+varcmd).c_str());
        		}
		        mySwitch.resetAvailable();
    
		}else{
			log("Aucune donnee...");
		}
	
    	delay(3000);
    }
}

