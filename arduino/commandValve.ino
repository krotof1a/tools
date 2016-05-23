long allowedSender = 8217031;
int allowedDevice = 5;

struct signal_t
{
  long sender;
  int receptor;
  boolean isSignal;
  boolean state;
} receivedSignal;

// Clockwise and counter-clockwise definitions.
// Depending on how you wired your motors, you may need to swap.
#define CW  0
#define CCW 1

// Motor definitions to make life easier:
#define MOTOR_A 0
#define MOTOR_B 1

// Pin Assignments //
// Don't change these! These pins are statically defined by shield layout
const byte PWMA = 3;  // PWM control (speed) for motor A
const byte PWMB = 11; // PWM control (speed) for motor B
const byte DIRA = 12; // Direction control for motor A
const byte DIRB = 13; // Direction control for motor B
int recepteurPin = 2;
int recepteurVcc = 5;
int recepteurGnd = 7;

void setup()
{
  //Serial.begin(9600);
  setupArdumoto(); // Set all pins as outputs
  setupReceiver();
}

void loop()
{
  //Ecoute des signaux
  listenSignal();
       
  //Si un signal au protocol home easy est recu...
  if(receivedSignal.isSignal){
           //Serial.print("Sender:");
           //Serial.println(receivedSignal.sender);
           //Serial.print("Device:");
           //Serial.println(receivedSignal.receptor);

           //Comparaison signal de reference, signal recu
           if (receivedSignal.sender==allowedSender && receivedSignal.receptor==allowedDevice){
                     //Serial.println("Signal correspondant");
                     
                    //On ferme ou on ouvre le relais en fonction du bit d'etat (on/off) du signal
                    if(receivedSignal.state)
        	        {	
                          //Serial.println("Etat : on, fermeture valve");
                          closeValve();
        	        }
        	        else
        	        {
                          //Serial.println("Etat : off, ouverture valve");	
                          openValve();
        	        }
           } else {
              //Serial.println("Signal inconnu");	
           }
   }
}

void listenSignal(){
        receivedSignal.sender = 0;
        receivedSignal.receptor = 0;
        receivedSignal.isSignal = false;
            
        int i = 0;
	unsigned long t = 0;

	byte prevBit = 0;
	byte bit = 0;

	unsigned long sender = 0;
	bool group = false;
	bool on = false;
	unsigned int recipient = 0;

	// latch 1
	while((t < 9200 || t > 11350))
	{	t = pulseIn(recepteurPin, LOW, 1000000);  
	}
        //Serial.println("latch 1");
      
	// latch 2
	while(t < 2550 || t > 2800)
	{	t = pulseIn(recepteurPin, LOW, 1000000);
	}

        //Serial.println("latch 2");

	// data
	while(i < 64)
	{
		t = pulseIn(recepteurPin, LOW, 1000000);
                 
		if(t > 200 && t < 365)
		{	bit = 0;
                    
		}
		else if(t > 1000 && t < 1400)
		{	bit = 1;
                      
		}
		else
		{	i = 0;
                        //Serial.println("bit mort"+t);
                        break;
		}

		if(i % 2 == 1)
		{
			if((prevBit ^ bit) == 0)
			{	// must be either 01 or 10, cannot be 00 or 11
				i = 0;
                                //Serial.println("Erreur manchester");
				break;
			}

			if(i < 53)
			{	// first 26 data bits
				sender <<= 1;
				sender |= prevBit;
			}	
			else if(i == 53)
			{	// 26th data bit
				group = prevBit;
			}
			else if(i == 55)
			{	// 27th data bit
				on = prevBit;
			}
			else
			{	// last 4 data bits
				recipient <<= 1;
				recipient |= prevBit;
			}
		}

		prevBit = bit;
		++i;
	}

       
	// interpret message
	if(i > 0)
	{
            receivedSignal.sender = sender;
            receivedSignal.receptor = recipient;
            receivedSignal.isSignal = true;
            if(on)
    	    {
             receivedSignal.state = true;
            }else{
              receivedSignal.state = false;
            }
	}
}

void setupReceiver() {
  pinMode(recepteurVcc, OUTPUT);
  pinMode(recepteurGnd, OUTPUT);
  pinMode(recepteurPin, INPUT);
  
  digitalWrite(recepteurVcc, HIGH);
  digitalWrite(recepteurGnd, LOW);
}

void openValve() {
  driveArdumoto(MOTOR_A, CCW, 255);
  delay(500);
  stopArdumoto(MOTOR_A);
}

void closeValve() {
  driveArdumoto(MOTOR_B, CCW, 127); // Necesary on my ardumoto as MOTOR_A CW doesn't work without this
  driveArdumoto(MOTOR_A, CW, 255);
  delay(150);
  stopArdumoto(MOTOR_A);
  stopArdumoto(MOTOR_B);
}

// driveArdumoto drives 'motor' in 'dir' direction at 'spd' speed
void driveArdumoto(byte motor, byte dir, byte spd)
{
  if (motor == MOTOR_A)
  {
    digitalWrite(DIRA, dir);
    analogWrite(PWMA, spd);
  }
  else if (motor == MOTOR_B)
  {
    digitalWrite(DIRB, dir);
    analogWrite(PWMB, spd);
  }  
}

// stopArdumoto makes a motor stop
void stopArdumoto(byte motor)
{
  driveArdumoto(motor, 0, 0);
}

// setupArdumoto initialize all pins
void setupArdumoto()
{
  // All pins should be setup as outputs:
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);

  // Initialize all pins as low:
  digitalWrite(PWMA, LOW);
  digitalWrite(PWMB, LOW);
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, LOW);
}
