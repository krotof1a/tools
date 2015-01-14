/*
 * Code pour construction d'une sonde de temperature "maison", récupère une temperature et l'envois sur la fréquence de 433 mhz
 * Fréquence : 433.92 mhz
 * Protocole : HRCSwitch
 * Licence : CC -by -sa
 * Matériel associé : Atmega 328 (+résonateur associé) + emetteur RF AM 433.92 mhz + capteur DS18B20 + led d'etat + résistance 4.7 kohms
 * Auteur : C.Safra basé sur le travail d'Idleman et la lib RCSwitch
 */
 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HRCSwitch.h>

//La sonde de température DS18B20 est branchée au pin 7 de l'atmega
#define TEMPERATURE_PIN 7
#define TEMPERATURE_VCC 6
#define TEMPERATURE_GND 8

//L'émetteur radio 433 mhz est branché au pin 11 de l'atmega
#define TRANSMITTER_PIN 11
#define TRANSMITTER_VCC 10
#define TRANSMITTER_GND 9

//Tableaud de stockage du numero de la sonde
char sonde[4]={'1','0','1','0'}; // Code 10 ici 

//Tableaud de stockage du signal binaire à  envoyer
char bit2[17]={};              
 
HRCSwitch mySwitch = HRCSwitch();

// On crée une instance de la classe oneWire pour communiquer avec le materiel on wire (dont le capteur ds18b20)
OneWire oneWire(TEMPERATURE_PIN);
//On passe la reference onewire à  la classe DallasTemperature qui vas nous permettre de relever la temperature simplement
DallasTemperature sensors(&oneWire);

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

  //Lancement de la commande de récuperation de la temperature
  sensors.requestTemperatures();
  unsigned long readTemp = 100*sensors.getTempCByIndex(0);  
  //Affichage de la temparature dans les logs
  Serial.println(readTemp);  
  //Conversion de la temperature en binaire et stockage sur 12 bits dans le tableau bit2
  itob(readTemp,12); 
  //Envois du signal radio comprenant la temperature (on l'envois 5 fois parce qu'on est pas des trompettes :p, et on veux être sur que ça recoit bien)
  mySwitch.send(bit2);
  //delais de 30sc avant le prochain envois
  delay(30000);
}
 
//fonction de conversion d'un nombre entier "integer" en binaire sur "length" bits et stockage dans le tableau bit2 + stockage signe + stockage code emetteur
void itob(unsigned long integer, int length)
{  
  int positive;
  if(integer>0){
   positive = true;
   Serial.println("positif ");
 }else{
  positive = false;
   Serial.println("negatif ");
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
