//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-405: BFT                                             ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of receiving from and transmitting to "Klik-Aan-Klik-Uit" devices
 * working according to the learning code system. This protocol is also used by the Home Easy devices.
 * It includes direct DIM functionality.
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
20;7D;DEBUG;Pulses=26;Pulses(uSec)=1050,780,1920,1860,900,870,1890,1890,900,900,1860,1920,840,900,1860,1890,870,930,1830,1920,840,1920,840,1950,870,6990;
20;7E;DEBUG;Pulses=26;Pulses(uSec)=1050,810,1890,1860,900,870,1860,1890,870,900,1860,1920,870,930,1860,1920,870,930,1830,1920,840,1920,840,1920,840,6990;

 \*********************************************************************************************/
#define BFT_RawSignalLength       26        // regular KAKU packet length
#define BFT_mT                   1500/RAWSIGNAL_SAMPLE_RATE // us, approx. in between 1T and 4T 

#ifdef PLUGIN_405
boolean Plugin_405(byte function, char *string) {
      return false;
/*
      if (RawSignal.Number != BFT_RawSignalLength) return false;
      boolean Bit;
      int i=1;
      byte P0,P1;
      unsigned long bitstream=0L;  
      do {
          P0=RawSignal.Pulses[i]  ; // * RawSignal.Multiply;
          P1=RawSignal.Pulses[i+1]; // * RawSignal.Multiply;
          
          if (P0<BFT_mT && P1<BFT_mT) {  
              Bit=1; // T,T,T,4T
          } else if (P0>BFT_mT && P1>BFT_mT) {
              Bit=0; // T,4T,T,T
          }
          bitstream=(bitstream<<1) | Bit;     
          i+=2;                                                    // Next 2 pulses
      } while(i<RawSignal.Number-6);                               //-6 to exclude the stopbit 
      //==================================================================================
      // Prevent repeating signals from showing up
      //==================================================================================
      if(SignalHash!=SignalHashPrevious || ((RepeatingTimer+700)<millis() ) || SignalCRC != bitstream ) { // 1000        
         SignalCRC=bitstream;
      } else {
         // already seen the RF packet recently
         //Serial.println("Skip");
         return true;
      } 
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(pbuffer, "20;%02X;", PKSequenceNumber++);    // Node and packet number 
      Serial.print( pbuffer );
      // ----------------------------------
      Serial.print("BSB;");                         // Label
      sprintf(pbuffer, "ID=%06x;SWITCH=1;CMD=ON;",bitstream);       // ID   
      Serial.print( pbuffer );
      Serial.println();
      // ----------------------------------
      RawSignal.Repeats=true;                              // suppress repeats of the same RF packet         
      RawSignal.Number=0;
      return true;
*/
}
#endif // Plugin_405

#ifdef PLUGIN_TX_405
void BFT_Send(unsigned long data, byte cmd);

boolean PluginTX_405(byte function, char *string) {
        boolean success=false;
        //10;BSB;000200;1;ON;
        if (strncasecmp(InputBuffer_Serial+3,"BSB;",4) == 0) { 
           byte x=15;                               // pointer to the switch number
          
           unsigned long bitstream=0L;
           // -----            
           InputBuffer_Serial[x-1]=0x00;            
           bitstream=strtoul(InputBuffer_Serial+7, NULL, 16);
           // -----
           BFT_Send(bitstream, 0xff);
           success=true;
        }
        // --------------------------------------
        return success;
}

void BFT_Send(unsigned long data, byte cmd) {
    int fpulse   = 950;                             // Pulse width in microseconds
    int fretrans = 10;                              // Number of code retransmissions

    unsigned long bitstream = 0L;
    byte command;
    // prepare data to send	
	for (unsigned short i=0; i<10; i++) {           // reverse data bits
		bitstream<<=1;
		bitstream|=(data & B1);
		data>>=1;
	}

    // Prepare transmit
    digitalWrite(PIN_RF_RX_VCC,LOW);                // Turn off power to the RF receiver 
    digitalWrite(PIN_RF_TX_VCC,HIGH);               // Enable the 433Mhz transmitter
    delayMicroseconds(TRANSMITTER_STABLE_DELAY);    // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
    // send bits
    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {
        	data=bitstream; 
		for (unsigned short i=0; i<10; i++) {
			switch (data & B1) {
				case 0:
					digitalWrite(PIN_RF_TX_DATA, HIGH);
					delayMicroseconds(fpulse*2);
					digitalWrite(PIN_RF_TX_DATA, LOW);
					delayMicroseconds(fpulse*2);
					break;
				case 1:
					digitalWrite(PIN_RF_TX_DATA, HIGH);
					delayMicroseconds(fpulse);
					digitalWrite(PIN_RF_TX_DATA, LOW);
					delayMicroseconds(fpulse);
					break;
			}
			//Next bit
			data>>=1;
		}
        
		//Send termination/synchronisation-signal. Total length: 26 periods
		digitalWrite(PIN_RF_TX_DATA, HIGH);
		delayMicroseconds(fpulse);
		digitalWrite(PIN_RF_TX_DATA, LOW);
		delayMicroseconds(fpulse*2);
		digitalWrite(PIN_RF_TX_DATA, HIGH);
		delayMicroseconds(fpulse);
		digitalWrite(PIN_RF_TX_DATA, LOW);
		delayMicroseconds(fpulse*2);
		digitalWrite(PIN_RF_TX_DATA, HIGH);
		delayMicroseconds(fpulse);
		digitalWrite(PIN_RF_TX_DATA, LOW);
		delayMicroseconds(fpulse*32);
    }
    // End transmit
    delayMicroseconds(TRANSMITTER_STABLE_DELAY);    // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
    digitalWrite(PIN_RF_TX_VCC,LOW);                // Turn thew 433Mhz transmitter off
    digitalWrite(PIN_RF_RX_VCC,HIGH);               // Turn the 433Mhz receiver on
    RFLinkHW();
}
#endif // Plugin_TX_405
