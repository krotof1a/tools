#include <wiringPi.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <sched.h>
#include <sstream>
#include "RCSwitch.h"
#include <map>
#include <curl/curl.h> 

// Pour compiler, récurérer RCSwitch.h et RCSwitch.cpp depuis https://github.com/ninjablocks/433Utils/tree/master/RPi_utils
// Puis lancer g++ -lwiringPi -lcurl -o radioReception RCSwitch.cpp radioReception.cpp

// Utilisation: sudo ./radioReception http://[ip]:[port]/json.htm [emiter_pin] [[hw_sensor_code] [domoticz_sens_idx]]
// Ex: sudo ./radioReception http://localhost:8080/json.htm 2 10 16 2>&1 /dev/zero

using namespace std;

RCSwitch mySwitch;
int pin;
map<int, int> outcomes;

void log(string a){
	//Décommenter pour avoir les logs
	//cerr << a << endl;
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

// 
// This is the callback function that is called by 
// curl_easy_perform(curl) 
// 
size_t handle_data(void *ptr, size_t size, size_t nmemb, void *stream){
	return size*nmemb; 
}

int openUrl (string urlToOpen) {
	CURL* curl = curl_easy_init(); 
    	if(curl) { 
        	// Tell libcurl the URL 
        	curl_easy_setopt(curl,CURLOPT_URL, urlToOpen.c_str()); 
        	// Tell libcurl what function to call when it has data 
        	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,handle_data); 
        	// Do it! 
        	CURLcode res = curl_easy_perform(curl); 
        	curl_easy_cleanup(curl); 
        	if (res != 0) { 
            		//cerr << "Error: " << res << endl;
			return -1;
		} 
        } 
    return 0; 
}

int main (int argc, char** argv)
{
	//string command = "wget -O /dev/zero \"";
	string command = "";
	command.append(argv[1]);
	pin = atoi(argv[2]);
	for (int i = 3; i < argc; i+=2)
    	{
         	outcomes[atoi(argv[i])]=atoi(argv[i+1]);
    	}
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
		unsigned long receiv = 0;
		unsigned long positive = 0;
		
    		if (mySwitch.available()) {
    
        		int value = mySwitch.getReceivedValue();
           		if (value == 0) {
          			log("Encoding inconnu");
        		} else {    
			   if (mySwitch.getReceivedProtocol()==6) {
				//DIO message
				receiv = mySwitch.getReceivedValue() & 255; //masque sur les 8 derniers bits
				positive = (mySwitch.getReceivedValue() >> 8) & 3; // decalage de 8 à droite et masque sur les 2 derniers bits
 				emiter = mySwitch.getReceivedValue() >> 10; // décalage de 10 digits à droite
			        log("------------------------------");
				log("Donnees detectees");
				log("emetteur = " + longToString(emiter));
				log("recepteur = " + longToString(receiv));
				varcmd.append("?type=command&param=switchlight&idx=");
                                varcmd.append(longToString(outcomes[receiv]));
				if (positive==1) {
					log("off");
					varcmd.append("&switchcmd=Off&level=0");
				} else {
					log("on");
					varcmd.append("&switchcmd=On&level=0");
				}
				//varcmd.append("\" > /dev/zero 2>&1");

				log("Execution de la commande PHP...");
				log((command+varcmd).c_str());
				//system((command+varcmd).c_str());
				openUrl((command+varcmd).c_str());
			   }else{
				//RCSwitch protocol
			        emiter = mySwitch.getReceivedValue() & 15; //masque sur les 4 derniers bits
			        positive = (mySwitch.getReceivedValue() >> 4) & 1; // decalage de 4 à droite et masque sur le dernier bit
			        temperature = (float)(mySwitch.getReceivedValue() >> 5) / (float)100; //decalage de 5 digits à droite
			        log("------------------------------");
				log("Donnees detectees");
				log("temperature = " + floatToString(temperature));
				log("positif = " + longToString(positive));
				log("code sonde = " +longToString(emiter));
				
				varcmd.append("?type=command&param=udevice&idx=");
				varcmd.append(longToString(outcomes[emiter]));
				//varcmd.append("&capteur="+longToString(emiter));
				//varcmd.append("&value="+floatToString(temperature));
				varcmd.append("&nvalue=0");
				if (positive==1) {
					varcmd.append("&svalue="+floatToString(temperature));
				} else {
					varcmd.append("&svalue=-"+floatToString(temperature));
				}
				//varcmd.append("\" > /dev/zero 2>&1");

				log("Execution de la commande PHP...");
				log((command+varcmd).c_str());
				//system((command+varcmd).c_str());
				openUrl((command+varcmd).c_str());
			   }
        		}
		        mySwitch.resetAvailable();
    
		}else{
			//log("Aucune donnee...");
		}
	
    	//delay(10);
	int tst = usleep(10000);
	if (tst<0) break;
    }
}

