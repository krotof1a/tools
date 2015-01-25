/*
 * Code pour construction d'une sonde de temperature "maison", 
 * récupère une temperature et une luminosité et l'envoit sur la fréquence de 433 mhz
 * Protocole : HRCSwitch (https://github.com/maditnerd/HRCSwitch)
 * Licence : CC -by -sa
 * Matériel associé : Atmega 328 (+résonateur associé) + emetteur RF AM 433.92 mhz + capteur DS18B20 + led d'etat + résistance 4.7 kohms + LDR
 * Auteur : C.Safra basé sur le travail d'Idleman et la lib RCSwitch
 */
 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RCSwitch.h>

//La sonde de température DS18B20 est branchée au pin 7 de l'atmega
#define TEMPERATURE_PIN 7
#define TEMPERATURE_VCC 6
#define TEMPERATURE_GND 8

//L'émetteur radio 433 mhz est branché au pin 11 de l'atmega
#define TRANSMITTER_PIN 11
#define TRANSMITTER_VCC 10
#define TRANSMITTER_GND 9

//La LDR est connectée en A0
#define LDR_PIN 0
#define LIGHT_LEVEL 500
int LDRPreviousState = 0;

//Tableaud de stockage du numero de la sonde
char sondeT[4]={'0','1','0','1'}; // Code 5 ici
char sondeL[4]={'0','1','1','0'}; // Code 6 ici
char* sonde;

//Tableaud de stockage du signal binaire à  envoyer
char bit2[17]={};              
 
RCSwitch mySwitch = RCSwitch();

// On crée une instance de la classe oneWire pour communiquer avec le materiel on wire (dont le capteur ds18b20)
OneWire oneWire(TEMPERATURE_PIN);
//On passe la reference onewire à  la classe DallasTemperature qui vas nous permettre de relever la temperature simplement
DallasTemperature sensors(&oneWire);

// Ne pas utiliser de delay() pour ne pas rompre la reactivité de l'Arduino
unsigned long TimerALimit = 30000UL; // Attente de 30s
unsigned long TimerA = 0;
unsigned long TimerBLimit = 50UL;    // Attente de 0.050s
unsigned long TimerB = 0;

//Fonction lancée à  l'initialisation du programme
void setup(void)
{
  //Reglage des alim : Le bloc peu etre commenté/supprimé si vous alimentez vos composants autrement.
  pinMode(TEMPERATURE_VCC, OUTPUT);
  digitalWrite(TEMPERATURE_VCC,HIGH);
  pinMode(TEMPERATURE_GND, OUTPUT);
  digitalWrite(TEMPERATURE_GND,LOW);
  pinMode(TRANSMITTER_VCC, OUTPUT);
  digitalWrite(TRANSMITTER_VCC,HIGH);
  pinMode(TRANSMITTER_GND, OUTPUT);
  digitalWrite(TRANSMITTER_GND,LOW);
  //On definis les logs à  9600 bauds
  Serial.begin(9600);
  //On initialise le capteur de temperature
  sensors.begin();
  //On définis le pin relié à  l'emetteur en tant que sortie
  mySwitch.enableTransmit(TRANSMITTER_PIN);
  mySwitch.setRepeatTransmit(5);
  mySwitch.disableReceive();
}

//Fonction qui boucle à  l'infinis
void loop(void)
{
 // Partie gestion de temperature
 if (millis()-TimerA >= TimerALimit) {
   //Lancement de la commande de récuperation de la temperature
   sensors.requestTemperatures();
   unsigned long readTemp = 100*sensors.getTempCByIndex(0);  
   //Affichage de la temparature dans les logs
   Serial.println(readTemp);  
   //Conversion de la temperature en binaire et stockage sur 12 bits dans le tableau bit2
   sonde=sondeT;
   itob(readTemp,12); 
   //Envois du signal radio comprenant la temperature (on l'envois 5 fois parce qu'on est pas des trompettes :p, et on veux être sur que ça recoit bien)
   mySwitch.setProtocol(1);
   mySwitch.send(bit2);
   //Reinit Timer
   TimerA=millis();
 }
 
 // Partie gestion de luminosite
 if (millis()-TimerB >= TimerBLimit) {
  int LDRValue = analogRead(LDR_PIN);
  int LDRState = 0;
  if (LDRValue < LIGHT_LEVEL) {
   //Lumiere detectee
   LDRState = 1;
  }
  if (LDRPreviousState!=LDRState) {
   //Changement d etat a notifier
   Serial.println("Light state change"); 
   LDRPreviousState=LDRState;
   sonde=sondeL;
   if (LDRState==0) {
     itob(100,12);
   } else {
     itob(0,12);
   }
   mySwitch.setProtocol(2);
   mySwitch.send(bit2);
  }
  //Reinit Timer
   TimerB=millis();
 }
}
 
//fonction de conversion d'un nombre entier "integer" en binaire sur "length" bits et stockage dans le tableau bit2 + stockage signe + stockage code emetteur
void itob(unsigned long integer, int length)
{  
  int positive;
  if(integer<0){
   positive = false;
   Serial.println("negatif ");
  }else{
   positive = true;
   Serial.println("positif ");
  }
  //needs bit2[length]
  // Convert long device code into binary (stores in global bit2 array.)
 for (int i=0; i<length; i++){
   if ((integer / power2(length-1-i))==1){
     integer-=power2(length-1-i);
     bit2[i]='1';
   }
   else {
     bit2[i]='0';
   }
 }
 //Définit le signe (+ ou -)
 if(positive){
   bit2[length]='1';
 }else{
   bit2[length]='0';
 }
 //Ajout code télécommande
 for (int j=0; j<4; j++) {
   bit2[length+j+1]=sonde[j];
 }
}

//Calcule 2^"power"
unsigned long power2(int power){    
 unsigned long integer=1;          
 for (int i=0; i<power; i++){      
   integer*=2;
 }
 return integer;
}
