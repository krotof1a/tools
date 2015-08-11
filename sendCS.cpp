#include <wiringPi.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <sched.h>
#include <sstream>
#include "RCSwitch.h"
#include <string.h>
#include <unistd.h>

/*
Script basé sur radioReception.cpp d'Idleman pour la partie DIO.
Compil.: g++ sendCS.cpp RCSwitch.cpp -o sendCS -lwiringPi
Usage:   ./sendCS <gpioPin> <senderCode> <deviceCode/"portal"> <"on"/"off"/"pulse"/portalCode> <pulseDuration>
 Ex:
 	./sendCS 0 12345 portal 1110001110
 	./sendCS 0 12345 1 on
 	./sendCS 0 12345 2 pulse 50
	./sendCS 0 12345 a1 off
*/

using namespace std;

// DIO protocol constants
int hiLenght = 275;             //275 orinally, but tweaked to 310 on Pi A
int low0Length = 275;		//275 orinally, but tweaked to 310 on Pi A
int low1Length = 1225;		//1225 orinally, but tweaked to 1340 on Pi A
int lowStartLength = 2675;
int lowEndLength = 10000;
int repeatNumber = 10;

int pin;
bool bit2[26]={};               // 26 bit Identifiant emetteur
bool bit2Interruptor[4]={};     //  4 bit Identifiant recepteur
int sender;
long group;
int interruptor;
string onoff;
int pulse;

void log(string a){
	//Décommenter pour avoir les logs
	cout << a << endl;
}

//Envois d'une pulsation (passage de l'etat haut a l'etat bas)
void sendBit(bool b) {
 if (b) {
   digitalWrite(pin, HIGH);
   delayMicroseconds(hiLenght);
   digitalWrite(pin, LOW);
   delayMicroseconds(low1Length);
 }
 else {
   digitalWrite(pin, HIGH);
   delayMicroseconds(hiLenght);
   digitalWrite(pin, LOW);
   delayMicroseconds(low0Length);
 }
}

//Calcul le nombre 2^chiffre indiqué, fonction utilisé par itob pour la conversion decimal/binaire
unsigned long power2(int power){
unsigned long integer=1;
for (int i=0; i<power; i++){
  integer*=2;
}
return integer;
} 

//Convertis un nombre en binaire, nécessite le nombre, et le nombre de bits souhaité en sortie (ici 26)
// Stocke le résultat dans le tableau global "bit2"
void itob(unsigned long integer, int length)
{
	for (int i=0; i<length; i++){
	  if ((integer / power2(length-1-i))==1){
		integer-=power2(length-1-i);
		bit2[i]=1;
	  }
	  else bit2[i]=0;
	}
}

void itobInterruptor(unsigned long integer, int length)
{
	for (int i=0; i<length; i++){
	  if ((integer / power2(length-1-i))==1){
		integer-=power2(length-1-i);
		bit2Interruptor[i]=1;
	  }
	  else bit2Interruptor[i]=0;
	}
}

//Envoie d'une paire de pulsation radio qui definissent 1 bit réel : 0 =01 et 1 =10
//c'est le codage de manchester qui necessite ce petit bouzin, ceci permet entre autres de dissocier les données des parasites
void sendPair(bool b) {
 if(b)
 {
   sendBit(true);
   sendBit(false);
 }
 else
 {
   sendBit(false);
   sendBit(true);
 }
}

//Fonction d'envois du signal
//recoit en parametre un booleen définissant l'arret ou la marche du matos (true = on, false = off)
void transmit(int blnOn)
{
 int i;

 // Sequence de verrou anoncant le départ du signal au recepeteur
 digitalWrite(pin, HIGH);
 delayMicroseconds(hiLenght);
 digitalWrite(pin, LOW);
 delayMicroseconds(lowStartLength);

 // Envoie du code emetteur (272946 = 1000010101000110010  en binaire)
 for(i=0; i<26;i++)
 {
   sendPair(bit2[i]);
 }

 // Envoie du bit définissant si c'est une commande de groupe ou non (26em bit)
 sendPair(false);

 // Envoie du bit définissant si c'est allumé ou eteint 27em bit)
 sendPair(blnOn);

 // Envoie des 4 derniers bits, qui représentent le code interrupteur, ici 0 (encode sur 4 bit donc 0000)
 for(i=0; i<4;i++)
 {
   sendPair(bit2Interruptor[i]);
 }
 
 // Sequence de verrou anoncant la fin du signal au recepeteur
 digitalWrite(pin, HIGH);
 delayMicroseconds(hiLenght);
 digitalWrite(pin, LOW);
 delayMicroseconds(lowEndLength);
}

void action (bool b) {
	if (b) {
		log("envois du signal ON");
 	} else {
	        log("envois du signal OFF");
	}
	for(int i=0; i<repeatNumber; i++) {
		transmit(b);       // Envoi le message X fois d'affilé
	}
}

int main (int argc, char** argv)
{
	if (setuid(0))
	{
		perror("setuid");
		return 1;
	}

	piHiPri(99); // Put process in real-time mode

	log("Demarrage du programme");
	pin = atoi(argv[1]);
	sender = atoi(argv[2]);
	if (strcmp(argv[3],"portal")==0) {
		interruptor = -1;
	} else {
		if (argv[3][0]=='a')
			group=42;
		else if (argv[3][0]=='b')
			group=138;
		else if (argv[3][0]=='c')
			group=162;
		else if (argv[3][0]=='d')
			group=168;
		else {
			group=-1;
			interruptor = atoi(argv[3]);
		}
		if (group != -1) {
			if (argv[3][1]=='1')
				interruptor=10;
			else if (argv[3][1]=='2')
				interruptor=34;
			else if (argv[3][1]=='3')
				interruptor=40;
		}
	}
	onoff = argv[4];
	pulse =(argc==6)? atoi(argv[5]) : 0;

	//Si on ne trouve pas la librairie wiringPI, on arrête l'execution
    if(wiringPiSetup() == -1)
    {
        log("Librairie Wiring PI introuvable, veuillez lier cette librairie...");
        return -1;
    }
	
	if (interruptor == -1) {
		log("Lancement en mode BFT ...");
		RCSwitch mySwitch = RCSwitch();
		mySwitch.enableTransmit(pin);
		mySwitch.setProtocol(RCSWITCH_ENCODING_BFT);
		mySwitch.setRepeatTransmit(10);
		mySwitch.send(const_cast<char*>(onoff.c_str()));
		if (pulse > 0) {
			delay(pulse);
			mySwitch.send(const_cast<char*>(onoff.c_str()));
		}
	} else if (group != -1) {
		log("Lancement en mode Casto ...");
                RCSwitch mySwitch = RCSwitch();
                mySwitch.enableTransmit(pin);
                mySwitch.setProtocol(RCSWITCH_ENCODING_RCS1);
                mySwitch.setRepeatTransmit(10);
		group = group << 15;
		interruptor = interruptor << 9;
		int separator = 42;
		separator = separator << 3;
		long localCode = group + interruptor + separator;
		if(onoff=="on"){
			localCode += 7;
		} else if (onoff=="off") {
			localCode += 4;
		} else
			log("Mode not implemented.");
                mySwitch.send(localCode,24);
	} else {
		log("Lancement en mode DIO ...");
		pinMode(pin, OUTPUT);
		log("Pin GPIO configure en sortie");
		itob(sender,26);
		itobInterruptor(interruptor,4);
		if(onoff=="on"){
			action(true);
		} else if (onoff=="off"){
			action(false);	 
		} else {
			action(true);
			delay(pulse);
			action(false);
		}
	}
	log("Fin du programme");    // execution terminée.

}
