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
