//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-404: TempCS                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of receiving from and transmitting to RCSwitch Protocol 1 devices
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 \*********************************************************************************************/
#define TempCS_RawSignalLength       36        // regular TempCS packet length
#define TempCS_mT                   600/RAWSIGNAL_SAMPLE_RATE // us, approx. in between 1T and 4T 

#ifdef PLUGIN_404
boolean Plugin_404(byte function, char *string) {
      if (RawSignal.Number != TempCS_RawSignalLength) return false;
      boolean Bit;
      int i=1;
      byte P0,P1;
      unsigned long bitstream=0L;  
      do {
          P0=RawSignal.Pulses[i]  ; // * RawSignal.Multiply;
          P1=RawSignal.Pulses[i+1]; // * RawSignal.Multiply;
          
          if (P0<TempCS_mT && P1>TempCS_mT) {  
              Bit=0; // short-long
          } else if (P0>TempCS_mT && P1<TempCS_mT) {
              Bit=1; // long-short
          }
          bitstream=(bitstream<<1) | Bit;     
          i+=2;                                                    // Next 2 pulses
      } while(i<RawSignal.Number-2);                               //-2 to exclude the stopbit 
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
      Serial.print("TempCS;");                            // Label
      sprintf(pbuffer, "ID=%01x;",((bitstream) & 15) );   // ID   
      Serial.print( pbuffer );
      int positive = ((bitstream) >> 1) & 1;
      int temperature = ((bitstream) >> 5) / 10;
      if (positive == 0) temperature = temperature * -1;  // to be reworked
      sprintf(pbuffer, "TEMP=%04x;", temperature);
      Serial.print( pbuffer );
      Serial.println();
      // ----------------------------------
      RawSignal.Repeats=true;                              // suppress repeats of the same RF packet         
      RawSignal.Number=0;
      return true;
}
#endif // Plugin_404

#ifdef PLUGIN_TX_404
void RCS1_Send(unsigned long address);           // sends 0 and 1

boolean PluginTX_404(byte function, char *string) {
      boolean success=false;

      if (strncasecmp(InputBuffer_Serial+3,"KAKU;",5) == 0) { // KAKU Command ex. 10;Kaku;000041;1;ON
           if (InputBuffer_Serial[14] != ';') return false;
           
           int group, inter;
           byte command=0;
           
           if (strncasecmp(InputBuffer_Serial+12,"41",2) == 0) {
            group=42;
           } else if (strncasecmp(InputBuffer_Serial+12,"42",2) == 0) {
            group=138;
           } else if (strncasecmp(InputBuffer_Serial+12,"43",2) == 0) {
            group=162;
           } else if (strncasecmp(InputBuffer_Serial+12,"44",2) == 0) {
            group=168;
           } else return false;
           
           if (strncasecmp(InputBuffer_Serial+15,"1",1) == 0) {
            inter=10;
           } else if (strncasecmp(InputBuffer_Serial+15,"2",1) == 0) {
            inter=34;
           } else if (strncasecmp(InputBuffer_Serial+15,"3",1) == 0) {
            inter=40;
           } else return false;
 
           group = group << 15;
	   inter = inter << 9;
	   int separator = 42;
	   separator = separator << 3;
	   long localCode = group + inter + separator;

           if (str2cmd(InputBuffer_Serial+17)==VALUE_ON) {
            localCode += 7;
           } else {
            localCode += 4;
           }

           RCS1_Send(localCode);
           success=true;
      }
      return success;
}

void RCS1_Send(unsigned long data) {
    int fpulse   = 350;                             // Pulse width in microseconds
    int fretrans = 10;                              // Number of code retransmissions

    unsigned long bitstream = 0L;
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
					delayMicroseconds(fpulse);
					digitalWrite(PIN_RF_TX_DATA, LOW);
					delayMicroseconds(fpulse*3);
					break;
				case 1:
					digitalWrite(PIN_RF_TX_DATA, HIGH);
					delayMicroseconds(fpulse*3);
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
		delayMicroseconds(fpulse*31);
    }
    // End transmit
    delayMicroseconds(TRANSMITTER_STABLE_DELAY);    // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
    digitalWrite(PIN_RF_TX_VCC,LOW);                // Turn thew 433Mhz transmitter off
    digitalWrite(PIN_RF_RX_VCC,HIGH);               // Turn the 433Mhz receiver on
    RFLinkHW();
}

#endif //PLUGIN_TX_404
