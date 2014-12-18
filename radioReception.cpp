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

// Puis lancer g++ -lwiringPi -lcurl -o radioReception RCSwitch.cpp radioReception.cpp

// Utilisation: sudo ./radioReception http://[ip]:[port]/json.htm [emiter_pin] [[hw_sensor_code] [domoticz_sens_idx]]
// Ex: sudo ./radioReception http://localhost:8080/json.htm 2 10 16 2>&1 /dev/zero

using namespace std;

RCSwitch mySwitch;
int pin;
map<int, int> outcomes;

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
		string varcmd = "?type=command&param=udevice&idx=";
    		float temperature = 0;
		unsigned long emiter = 0;
		unsigned long receiv = 0;
		unsigned long commId = 0;
		unsigned long positive = 0;
		
    		if (mySwitch.available()) {
    
        		int value = mySwitch.getReceivedValue();
           		if (value == 0) {
          			log("Encoding inconnu");
        		} else {    
			   if (mySwitch.getReceivedProtocol()==6) {
				//DIO message
				receiv = mySwitch.getReceivedValue() & 15; //masque sur les 4 derniers bits
				positive = (mySwitch.getReceivedValue() >> 4) & 1; // decalage de 4 à droite et masque sur le dernier bits
 				emiter = mySwitch.getReceivedValue() >> 6; // décalage de 6 digits à droite
 				commId = (emiter << 4) + receiv; // id construit par concat emiter + receiv
			        log("------------------------------");
				log("Donnees DIO detectees");
				log("emetteur  = " + longToString(emiter));
				log("recepteur = " + longToString(receiv));
				log("on/off    = " + longToString(positive));
				log("id        = " + longToString(commId));
                                varcmd.append(longToString(outcomes[commId]));
				if (positive==0) {
					varcmd.append("&nvalue=0");
				} else if (positive==1) {
					varcmd.append("&nvalue=1");
				} else {
					log("Reception incorrecte");
				        mySwitch.resetAvailable();
					continue;
				}	
			   } else {
				//RCSwitch protocol
			        emiter = mySwitch.getReceivedValue() & 15; //masque sur les 4 derniers bits
			        positive = (mySwitch.getReceivedValue() >> 4) & 1; // decalage de 4 à droite et masque sur le dernier bit
			        temperature = (float)(mySwitch.getReceivedValue() >> 5) / (float)100; //decalage de 5 digits à droite
			        log("------------------------------");
				log("Donnees RCSwitch detectees");
				log("code sonde  = " +longToString(emiter));
				log("positif     = " + longToString(positive));
				log("temperature = " + floatToString(temperature));
				
				varcmd.append(longToString(outcomes[emiter]));
				varcmd.append("&nvalue=0");
				if (positive==1) {
					varcmd.append("&svalue="+floatToString(temperature));
				} else {
					varcmd.append("&svalue=-"+floatToString(temperature));
				}
			   }
			   log("Execution de l'URL ...");
			   log((command+varcmd).c_str());
			   openUrl((command+varcmd).c_str());
        		}
		        mySwitch.resetAvailable();
    
		}else{
			//log("Aucune donnee...");
		}
	
	if (usleep(100000) < 0) 
		break; // Ppogram should exit if sleep cannot be done (in order to avoid full CPU consumption)
    }
}

