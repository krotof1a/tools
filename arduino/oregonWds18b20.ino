/*
 * Fausse sonde Oregon avec capteur ds18b20
 * From http://easydomoticz.com/forum/viewtopic.php?f=7&t=240
 */
 
#include <OneWire.h>
#include <DallasTemperature.h>

//La sonde de température DS18B20 est branchée au pin 7 de l'atmega
#define TEMPERATURE_PIN 7
#define TEMPERATURE_VCC 6
#define TEMPERATURE_GND 8

//L'émetteur radio 433 mhz est branché au pin 11 de l'atmega
#define TRANSMITTER_PIN 11
#define TRANSMITTER_VCC 10
#define TRANSMITTER_GND 9

// On crée une instance de la classe oneWire pour communiquer avec le materiel on wire (dont le capteur ds18b20)
OneWire oneWire(TEMPERATURE_PIN);
//On passe la reference onewire à  la classe DallasTemperature qui vas nous permettre de relever la temperature simplement
DallasTemperature sensors(&oneWire);

// Ne pas utiliser de delay() pour ne pas rompre la reactivité de l'Arduino
unsigned long TimerALimit = 30000UL; // Attente de 30s
unsigned long TimerA = 0;

/******************************************************************/
/***********************  Oregon utilities    *********************/
/******************************************************************/
const unsigned long TIME = 512;
const unsigned long TWOTIME = TIME*2;
 
#define SEND_HIGH() digitalWrite(TX_PIN, HIGH)
#define SEND_LOW() digitalWrite(TX_PIN, LOW)

byte OregonMessageBuffer[8];
 
/**
 * \brief    Set the sensor type
 * \param    data       Oregon message
 * \param    type       Sensor type
 */
inline void setType(byte *data, byte* type) 
{
  data[0] = type[0];
  data[1] = type[1];
}
 
/**
 * \brief    Set the sensor channel
 * \param    data       Oregon message
 * \param    channel    Sensor channel (0x10, 0x20, 0x30)
 */
inline void setChannel(byte *data, byte channel) 
{
    data[2] = channel;
}
 
/**
 * \brief    Set the sensor ID
 * \param    data       Oregon message
 * \param    ID         Sensor unique ID
 */
inline void setId(byte *data, byte ID) 
{
  data[3] = ID;
}
 
/**
 * \brief    Set the sensor battery level
 * \param    data       Oregon message
 * \param    level      Battery level (0 = low, 1 = high)
 */
void setBatteryLevel(byte *data, byte level)
{
  if(!level) data[4] = 0x0C;
  else data[4] = 0x00;
}
 
/**
 * \brief    Set the sensor temperature
 * \param    data       Oregon message
 * \param    temp       the temperature
 */
void setTemperature(byte *data, float temp) 
{
  // Set temperature sign
  if(temp < 0)
  {
    data[6] = 0x08;
    temp *= -1;  
  }
  else
  {
    data[6] = 0x00;
  }
 
  // Determine decimal and float part
  int tempInt = (int)temp;
  int td = (int)(tempInt / 10);
  int tf = (int)round((float)((float)tempInt/10 - (float)td) * 10);
 
  int tempFloat =  (int)round((float)(temp - (float)tempInt) * 10);
 
  // Set temperature decimal part
  data[5] = (td << 4);
  data[5] |= tf;
 
  // Set temperature float part
  data[4] |= (tempFloat << 4);
}

/**
 * \brief    Sum data for checksum
 * \param    count      number of bit to sum
 * \param    data       Oregon message
 */
int Sum(byte count, const byte* data)
{
  int s = 0;
 
  for(byte i = 0; i<count;i++)
  {
    s += (data[i]&0xF0) >> 4;
    s += (data[i]&0xF);
  }
 
  if(int(count) != count)
    s += (data[count]&0xF0) >> 4;
 
  return s;
}
 
/**
 * \brief    Calculate checksum
 * \param    data       Oregon message
 */
void calculateAndSetChecksum(byte* data)
{
    int s = ((Sum(6, data) + (data[6]&0xF) - 0xa) & 0xff);
    data[6] |=  (s&0x0F) << 4;     data[7] =  (s&0xF0) >> 4;
}

/**
 * \brief    Send logical "0" over RF
 * \details  azero bit be represented by an off-to-on transition
 * \         of the RF signal at the middle of a clock period.
 * \         Remenber, the Oregon v2.1 protocol add an inverted bit first 
 */
inline void sendZero(void) 
{
  SEND_HIGH();
  delayMicroseconds(TIME);
  SEND_LOW();
  delayMicroseconds(TWOTIME);
  SEND_HIGH();
  delayMicroseconds(TIME);
}
 
/**
 * \brief    Send logical "1" over RF
 * \details  a one bit be represented by an on-to-off transition
 * \         of the RF signal at the middle of a clock period.
 * \         Remenber, the Oregon v2.1 protocol add an inverted bit first 
 */
inline void sendOne(void) 
{
   SEND_LOW();
   delayMicroseconds(TIME);
   SEND_HIGH();
   delayMicroseconds(TWOTIME);
   SEND_LOW();
   delayMicroseconds(TIME);
}
 
/**
* Send a bits quarter (4 bits = MSB from 8 bits value) over RF
*
* @param data Source data to process and sent
*/
 
/**
 * \brief    Send a bits quarter (4 bits = MSB from 8 bits value) over RF
 * \param    data   Data to send
 */
inline void sendQuarterMSB(const byte data) 
{
  (bitRead(data, 4)) ? sendOne() : sendZero();
  (bitRead(data, 5)) ? sendOne() : sendZero();
  (bitRead(data, 6)) ? sendOne() : sendZero();
  (bitRead(data, 7)) ? sendOne() : sendZero();
}
 
/**
 * \brief    Send a bits quarter (4 bits = LSB from 8 bits value) over RF
 * \param    data   Data to send
 */
inline void sendQuarterLSB(const byte data) 
{
  (bitRead(data, 0)) ? sendOne() : sendZero();
  (bitRead(data, 1)) ? sendOne() : sendZero();
  (bitRead(data, 2)) ? sendOne() : sendZero();
  (bitRead(data, 3)) ? sendOne() : sendZero();
}

/**
 * \brief    Send a buffer over RF
 * \param    data   Data to send
 * \param    size   size of data to send
 */
void sendData(byte *data, byte size)
{
  for(byte i = 0; i < size; ++i)
  {
    sendQuarterLSB(data[i]);
    sendQuarterMSB(data[i]);
  }
}
 
/**
 * \brief    Send an Oregon message
 * \param    data   The Oregon message
 */
void sendOregon(byte *data, byte size)
{
    sendPreamble();
    sendData(data, size);
    sendPostamble();
}
 
/**
 * \brief    Send preamble
 * \details  The preamble consists of 16 "1" bits
 */
inline void sendPreamble(void)
{
  byte PREAMBLE[]={0xFF,0xFF};
  sendData(PREAMBLE, 2);
}
 
/**
 * \brief    Send postamble
 * \details  The postamble consists of 8 "0" bits
 */
inline void sendPostamble(void)
{
  sendQuarterLSB(0x00);
}

/******************************************************************/
/**************************** Main Part  **************************/

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
  //On initialise la fausse sonde Oregon
  SEND_LOW(); 
  setType(OregonMessageBuffer, {0xEA,0x4C});
  setChannel(OregonMessageBuffer, 0x20);
  setId(OregonMessageBuffer, 0xCC);
  setBatteryLevel(OregonMessageBuffer, 1);

}

//Fonction qui boucle à  l'infinis
void loop(void)
{
 // Partie gestion de temperature
 if (millis()-TimerA >= TimerALimit) {
   //Lancement de la commande de récuperation de la temperature
   sensors.requestTemperatures();
   unsigned long readTemp = sensors.getTempCByIndex(0);
   setTemperature(OregonMessageBuffer, readTemp);  
   //Affichage de la temparature dans les logs
   Serial.print("Temperature : ");Serial.print(readTemp);Serial.write(176);Serial.write('C');Serial.println();  
   // Calcul du checksum
   calculateAndSetChecksum(OregonMessageBuffer);
   //Envoi
   sendOregon(OregonMessageBuffer, sizeof(OregonMessageBuffer));
   SEND_LOW();
   delayMicroseconds(TWOTIME*8);
   sendOregon(OregonMessageBuffer, sizeof(OregonMessageBuffer));
   SEND_LOW();
   //Reinit Timer
   TimerA=millis();
 }
}
