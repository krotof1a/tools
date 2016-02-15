//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-48 Oregon V1/2/3                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This protocol takes care of receiving Oregon Scientific outdoor sensors that use the V1, V2 and V3 protocol
 *
 * models: THC238, THC268, THN132N, THWR288A, THRN122N, THN122N, AW129, AW131, THGR268, THGR122X,
 *         THGN122N, THGN123N, THGR122NX, THGR228N, THGR238, WTGR800, THGR918, THGRN228NX, THGN500, RTGN318
 *         THGR810, RTGR328N, THGR328N, Huger BTHR918, BTHR918N, BTHR968, RGR126, RGR682, RGR918, PCR122
 *         THWR800, THR128, THR138, THC138, OWL CM119, OWL CM180, cent-a-meter, OWL CM113, Electrisave, RGR928 
 *         UVN128, UV138, UVN800, Huger-STR918, WGR918, WGR800, PCR800, WGTR800, RGR126, BTHG968, BTHGN129 
 *
 * Author(s)          : StuntTeam, Thibaut Girka, Snips
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!  
 *********************************************************************************************
 * Core code originally from https://github.com/Cactusbone/ookDecoder/blob/master/ookDecoder.ino => heavily modified now!
 * Copyright (c) 2014 Charly Koza cactusbone@free.fr Copyright (c) 2012 Olivier Lebrun olivier.lebrun@connectingstuff.net 
 * Copyright (c) 2012 Dominique Pierre (zzdomi) Copyright (c) 2010 Jean-Claude Wippler jcw@equi4.com
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
 * is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 \*********************************************************************************************/
 //Rajouter dans Base.ino : unsigned long long previous_total = 0LL;
 
#define OSV3_PULSECOUNT_MIN 50  // 126
#define OSV3_PULSECOUNT_MAX 290 // make sure to check the max length in plugin 1 as well..!

//OSV1 pulses
#define OSV_PULSE900    900/RAWSIGNAL_SAMPLE_RATE
#define OSV_PULSE3400   3390/RAWSIGNAL_SAMPLE_RATE
#define OSV_PULSE2000   1980/RAWSIGNAL_SAMPLE_RATE
#define OSV_PULSE5900   5880/RAWSIGNAL_SAMPLE_RATE
//OSV2 pulses
#define OSV_PULSE150    150/RAWSIGNAL_SAMPLE_RATE
#define OSV_PULSE600    600/RAWSIGNAL_SAMPLE_RATE
#define OSV_PULSE1200   1200/RAWSIGNAL_SAMPLE_RATE
#define OSV_PULSE2500   2490/RAWSIGNAL_SAMPLE_RATE
//OSV3 pulses
#define OSV_PULSE200    180/RAWSIGNAL_SAMPLE_RATE
#define OSV_PULSE675    660/RAWSIGNAL_SAMPLE_RATE
//#define OSV_PULSE1200   1200/RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_048
// ==================================================================================
class DecodeOOK {
protected:
	byte total_bits, bits, flip, state, pos, data[25];
	virtual char decode(byte width) = 0;
public:
	enum { UNKNOWN, T0, T1, T2, T3, OK, DONE };
    // -------------------------------------
	DecodeOOK() { resetDecoder(); }
    // -------------------------------------
	bool nextPulse(byte width) {
		if (state != DONE)
			switch (decode(width)) {
			case -1: resetDecoder(); break;
			case 1:  done(); break;
		}
		return isDone();
	}
    // -------------------------------------
	bool isDone() const { return state == DONE; }
    // -------------------------------------
	const byte* getData(byte& count) const {
		count = pos;
		return data;
	}
    // -------------------------------------
	void resetDecoder() {                           // reset all variables
		total_bits = bits = pos = flip = 0;         // zero all variables 
		state = UNKNOWN;                            // set state to begin position
	}
    // -------------------------------------
	// add one bit to the packet data buffer
    // -------------------------------------
	virtual void gotBit(char value) {
		total_bits++;
		byte *ptr = data + pos;
		*ptr = (*ptr >> 1) | (value << 7);

		if (++bits >= 8) {
			bits = 0;
			if (++pos >= sizeof data) {
				resetDecoder();
				return;
			}
		}
		state = OK;
	}
    // -------------------------------------
	// store a bit using Manchester encoding
    // -------------------------------------
	void manchester(char value) {
		flip ^= value; // manchester code, long pulse flips the bit
		gotBit(flip);
	}
    // -------------------------------------
	// move bits to the front so that all the bits are aligned to the end
    // -------------------------------------
	void alignTail(byte max = 0) {
		// align bits
		if (bits != 0) {
			data[pos] >>= 8 - bits;
			for (byte i = 0; i < pos; ++i)
				data[i] = (data[i] >> bits) | (data[i + 1] << (8 - bits));
			bits = 0;
		}
		// optionally shift bytes down if there are too many of 'em
		if (max > 0 && pos > max) {
			byte n = pos - max;
			pos = max;
			for (byte i = 0; i < pos; ++i)
				data[i] = data[i + n];
		}
	}
    // -------------------------------------
	void reverseBits() {
		for (byte i = 0; i < pos; ++i) {
			byte b = data[i];
			for (byte j = 0; j < 8; ++j) {
				data[i] = (data[i] << 1) | (b & 1);
				b >>= 1;
			}
		}
	}
    // -------------------------------------
	void reverseNibbles() {
		for (byte i = 0; i < pos; ++i)
			data[i] = (data[i] << 4) | (data[i] >> 4);
	}
    // -------------------------------------
	void done() {
		while (bits)
			gotBit(0); // padding
		state = DONE;
	}
};

/* Original routine, replaced by improved version
class OregonDecoderV1_org : public DecodeOOK {
	public:
		OregonDecoderV1_org() {}
			virtual char decode(word width) {
			if (200 <= width && width < 1200) {
				byte w = width >= 700;
				switch (state) {
					case UNKNOWN:
						if (w == 0)
							++flip;
						else if (10 <= flip && flip <= 50) {
							flip = 1;
							manchester(1);
						}
						else
							return -1;
						break;
					case OK:
						if (w == 0)
							state = T0;
						else
							manchester(1);
						break;
					case T0:
						if (w == 0)
							manchester(0);
						else
							return -1;
						break;
				}
				return 0;
			}
			if (width >= 2500 && pos >= 9)
				return 1;
			return -1;
		}
}; 
*/
class OregonDecoderV1 : public DecodeOOK {
	public:
		OregonDecoderV1() {}
			virtual char decode(byte width) {
			if (OSV_PULSE900 <= width && width < OSV_PULSE3400) {
				byte w = width >= OSV_PULSE2000;
				switch (state) {
					case UNKNOWN:                   // Detect preamble
						if (w == 0)
							++flip;
						else 
                        	return -1;
						break;
					case OK:
						if (w == 0)
							state = T0;
						else
							manchester(1);
						break;
					case T0:
						if (w == 0)
							manchester(0);
						else
							return -1;
                        break;
					default:						// Unexpected state
						return -1;                                
				}
				return (pos == 4) ? 1 : 0; // Messages are fixed-size
			}
			if (width >= OSV_PULSE3400) {
				if (flip < 10 || flip > 50)
					return -1; // No preamble
				switch (state) {
					case UNKNOWN:
						// First sync pulse, lowering edge
						state = T1;
						break;
					case T1:
						// Second sync pulse, lowering edge
						state = T2;
						break;
					case T2:
						// Last sync pulse, determines the first bit!
						if (width <= OSV_PULSE5900) {
							state = T0;
							flip = 1;
						} else {
							state = OK;
							flip = 0;
                            manchester(0);
						}
					break;
				}
 				return 0;
 			}
 			return -1;
 		}
};

class OregonDecoderV2 : public DecodeOOK {
public:
	OregonDecoderV2() {}
    // -------------------------------------
    // add one bit to the packet data buffer
    // -------------------------------------
    virtual void gotBit(char value) {
		if (!(total_bits & 0x01)) {
           data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
        }
        total_bits++;                               // increase bit counter
        pos = total_bits >> 4;
        if (pos >= sizeof data) {                   // did we receive the maximum number of bits for our container?
           resetDecoder();                          // clean
           return;
        }
        state = OK;
    }
    // -------------------------------------
	virtual char decode(byte width) {
		if (OSV_PULSE150 <= width && width < OSV_PULSE1200) {         // pulse length is between 150 & 1200? Then it is a valid 0/1 bit pulse length
			byte w = width >= OSV_PULSE600;                  // long/short switch point  (previously was set to 200/675/1200)
			switch (state) {
             case UNKNOWN:
				if (w != 0) {                       // Long pulse
					++flip;
				} else
				//if (w == 0 && 24 <= flip) {       // count 24 start pulses
                if (w == 0 && 8 <= flip) {          // count 8 start pulses 
					flip = 0;                       // Short pulse, start bit
					state = T0;
				} else {                            // Reset decoder
					return -1;                      // not a valid sequence
				}
				break;
             case OK:
				if (w == 0) {                       // Short pulse
                   state = T0;                      // if short pulse is followed by another short pulse it will become a 0 bit
				} else {                            // Long pulse
                   manchester(1);                   // 1 bit
				}
				break;
             case T0:
				if (w == 0) {                       // Second short pulse
                   manchester(0);                   // 0 bit
				} else {                            // Reset decoder
                   return -1;                       // not a valid sequence
				}
				break;
			} // end: switch (state)
		} else {                                    // pulse out of normal range
           if (width >= OSV_PULSE2500 && pos >= 8) {         // found a "very" long pulse at the end of the packet?
              //if (total_bits > 0) {
              //   Serial.print("E0: ");
              //   Serial.println(total_bits);
              //}     
              return 1;                             // end of packet, notify caller 
           } else {                                 // found a pulse we do not recognize?            
              //return -1;                          // reject entire packet
              //if (total_bits > 0) {
              //   Serial.print("E1: ");
              //   Serial.println(total_bits);
              //}
              //return  (total_bits <160 && total_bits>=40  ) ? 1: -1;
              return  (total_bits <200 && total_bits>=40  ) ? 1: -1;      // 
		   }
        }
        //if (total_bits > 100) {                       // Check packet length, have we reached the limit yet? 
        //   Serial.print("E2 [");
        //   Serial.print(width);
        //   Serial.print("] (");
        //   Serial.print(pos);
        //   Serial.print(") :");
        //   Serial.println(total_bits);
        //}
        //if (total_bits > 159) {
        //   Serial.print("Data: ");
        //   PrintHex8( data, 24);
        //   Serial.println();
        //}
		return total_bits == 200 ? 1 : 0;           // reached maximum length? notify caller
		//return total_bits == 160 ? 1 : 0;
	}
};

class OregonDecoderV3 : public DecodeOOK {
public:
	OregonDecoderV3() {}
    // -------------------------------------
	// add one bit to the packet data buffer
    // -------------------------------------
	virtual void gotBit(char value) {
		data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
		total_bits++;
		pos = total_bits >> 3;
		if (pos >= sizeof data) {
			resetDecoder();
			return;
		}
		state = OK;
	}
    // -------------------------------------
	virtual char decode(byte width) {
		if (OSV_PULSE200 <= width && width < OSV_PULSE1200) { 
			byte w = width >= OSV_PULSE675;
			switch (state) {
			case UNKNOWN:
				if (w == 0)
					++flip;
				//else if (32 <= flip) {
				else if (8 <= flip) {
					flip = 1;
					manchester(1);
				}
				else
					return -1;
				break;
			case OK:
				if (w == 0)
					state = T0;
				else
					manchester(1);
				break;
			case T0:
				if (w == 0)
					manchester(0);
				else
					return -1;
				break;
			}
		} else {
			//return -1;
			return  (total_bits <104 && total_bits>=40  ) ? 1: -1;
		}
		//return  total_bits == 80 ? 1 : 0;
		return  total_bits == 104 ? 1 : 0;
	}
};

OregonDecoderV1 orscV1;
OregonDecoderV2 orscV2;
OregonDecoderV3 orscV3;

// =====================================================================================================
// =====================================================================================================
byte osdata[25];                                    // Global Oregon data array 

uint16_t reportSerial(class DecodeOOK& decoder) {
	byte pos;                                       // should be renamed to len to avoid confusion?
	const byte* data = decoder.getData(pos);
    if (pos > 24) pos=24;                           // overflow protection, should not really be needed but limit to max array size anyway
	for (byte i = 0; i < pos; ++i) {                // copy bytes from the collected data to the global array
        osdata[i]=data[i];
	}
	decoder.resetDecoder();                         // clean everything else
	return pos;                                     // note that pos is a local variable, not the pos variable used in the decoder routines
}
// =====================================================================================================
// calculate a packet checksum by performing an addition of nibbles and comparing with a provided byte value
// =====================================================================================================
byte checksum(byte type, int count, byte check) {
     byte calc=0;
     // type 1, add all nibbles, deduct 10
     if (type == 1) {
        for(byte i = 0; i<count;i++) {
           calc += (osdata[i]&0xF0) >> 4;
           calc += (osdata[i]&0xF);
        }
        calc=calc-10;
     } else 
     // type 2, add all nibbles up to count, add the 13th nibble , deduct 10
     if (type == 2) {
        for(byte i = 0; i<count;i++) {
           calc += (osdata[i]&0xF0) >> 4;
           calc += (osdata[i]&0xF);
        }
        calc += (osdata[6]&0xF);
        calc=calc-10;
     } else 
     // type 3, add all nibbles up to count, subtract 10 only use the low 4 bits for the compare
     if (type == 3) {
        for(byte i = 0; i<count;i++) {
           calc += (osdata[i]&0xF0) >> 4;
           calc += (osdata[i]&0xF);
        }
        calc=calc-10;
        calc=(calc&0x0f);
     } else 
     if (type == 4) {
        for(byte i = 0; i<count;i++) {
           calc += (osdata[i]&0xF0) >> 4;
           calc += (osdata[i]&0xF);
        }
        calc=calc-10;
        
     } 
     if (check == calc ) return 0;     
     return 1;
}
// =====================================================================================================
// Main Routine that handles all protocol versions of the Oregon Scientific Sensors
// =====================================================================================================
boolean Plugin_048(byte function, char *string) {
      if ((RawSignal.Number < OSV3_PULSECOUNT_MIN) || (RawSignal.Number > OSV3_PULSECOUNT_MAX) ) return false; 

      byte found = 0;
      int temp = 0;
      byte hum = 0;
      int comfort = 0;
      int baro = 0;
      int forecast = 0;
      int uv = 0;
      int wdir = 0;
      int wspeed = 0;
      int awspeed = 0;
      int rain = 0;
      int raintot = 0;
	  int datLen=0;

      //word p = pulse;
      //word p = 0;
      byte p = 0;
      // ==================================================================================
      for (int x = 1; x < RawSignal.Number+1; x++) {
          p = RawSignal.Pulses[x];                  // Get pulse duration
          if (p != 0) {
             if (orscV1.nextPulse(p)) {
                datLen=reportSerial(orscV1);
                found=1; 
             } 
             if (orscV2.nextPulse(p)) { 
                datLen=reportSerial(orscV2);
                found=2;
             } 
             if (orscV3.nextPulse(p)) { 
                datLen=reportSerial(orscV3);
                found=3;
             } 
          }
      }
      if (found == 0) return false;
      // ==================================================================================
      // Create device ID for the following compare:
      unsigned int id=(osdata[0]<<8)+ (osdata[1]);
      // ==================================================================================
      // Process the various Oregon device types:
      // ==================================================================================
      // Oregon V1 packet structure
      // SL-109H, AcuRite 09955
      // OSV1 : TEMP + CRC
      // 0 1 2 3
      // 8487101C
      // 84+87+10=11B > 1B+1 = 1C
      // ==================================================================================
      if (found==1) {                               // OSV1 
         int sum = osdata[0]+osdata[1]+osdata[2];   // max. value is 0x2FD
         sum= (sum &0xff) + (sum>>8);               // add overflow to low byte
         if (osdata[3] != (sum & 0xff) ) {
            return false;
         }
         // -------------       
         temp = ((osdata[2] & 0x0F) * 100)  + ((osdata[1] >> 4) * 10) + ((osdata[1] & 0x0F));
         if ((osdata[2] & 0x20) == 0x20) temp=temp | 0x8000;  // bit 1 set when temp is negative, set highest bit on temp valua
         // ----------------------------------
         // Output
         // ----------------------------------
         Serial.print("20;");
         PrintHexByte(PKSequenceNumber++);
         Serial.print(F(";OregonV1;ID=00"));         // Label
         PrintHexByte((osdata[0])&0xcf);                    // rolling code + channel
         // ----------------------------------
         sprintf(pbuffer, ";TEMP=%04x;", temp);      // temperature
         Serial.print( pbuffer );
         if (osdata[2] & 0x80) {                     // battery state
            Serial.print(F("BAT=LOW;"));
         } else {
            Serial.print(F("BAT=OK;"));
         }
         Serial.println();
         RawSignal.Repeats=true;                    // suppress repeats of the same RF packet 
         RawSignal.Number=0;
         return true;
      }
      // ==================================================================================
      // ea4c   Outside (Water) Temperature: THC238, THC268, THN132N, THWR288A, THRN122N, THN122N, AW129, AW131
      // ca48   Pool (Water) Temperature: THWR800
      // 0a4d   Indoor Temperature: THR128, THR138, THC138 
      // OSV2 : TEMP + BAT + CRC
      //      0 1 2 3 4 5 6 7 8 9 a
      //      0123456789012345678901
      // OSV2 EA4C20725C21D083 // THN132N
      // ==================================================================================
      if(id == 0xea4c || id == 0xca48 || id == 0x0a4d) {
        byte sum=(osdata[7]&0x0f) <<4; 
        sum=sum+(osdata[6]>>4);
        if ( checksum(2,6, sum) !=0) {  // checksum = all nibbles 0-11+13 results is nibbles 15 <<4 + 12
            return false;
        }
        // -------------       
        temp = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + ((osdata[4] >> 4));
        if ((osdata[6] & 0x0F) >= 8) temp=temp | 0x8000;
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        Serial.print(F(";Oregon Temp;ID="));           // Label
        PrintHexByte(osdata[3]);                       // address
        PrintHexByte(osdata[2]);                       // channel
        // ----------------------------------
        sprintf(pbuffer, ";TEMP=%04x;", temp);          
        Serial.print( pbuffer );
        if (((osdata[4])&0x04) == 0) {
           Serial.print(F("BAT=OK;")); 
        } else {
           Serial.print(F("BAT=LOW;")); 
        }        
        Serial.println();
      } else
      // ==================================================================================
      // 1a2d   Indoor Temp/Hygro: THGN122N, THGN123N, THGR122NX, THGR228N, THGR238, THGR268, THGR122X
      // 1a3d   Outside Temp/Hygro: THGR918, THGRN228NX, THGN500, RTGN318 
      // fa28   Indoor Temp/Hygro: THGR810
      // *aac   Outside Temp/Hygro: RTGR328N
      // ca2c   Outside Temp/Hygro: THGR328N
      // fab8   Outside Temp/Hygro: WTGR800
      // aad1   Oregon Simulator
      // OSV2 : TEMP + HUM + BAT + CRC
      // 0 1 2 3 4 5 6 7 8 9 a
      // 0123456789012345678901
      // 1a2d40f6201610263b55
      // 1+a+2+d+4+0+f+6+2+0+1+6+1+0+2+6=45-a=3b  [3b]55
      // Sample: pulses=234: 960,900,900,840,900,870,900,870,900,840,900,870,900,840,900,840,900,840,900,870,900,870,900,870,900,870,900,840,900,870,900,360,420,870,420,360,900,360,420,870,420,360,900,840,900,360,420,870,900,870,900,840,420,360,900,360,420,840,420,360,900,840,900,360,420,840,420,390,900,360,420,870,900,840,900,870,900,840,900,870,900,840,900,870,900,870,420,360,900,360,420,870,900,870,420,360,900,870,900,360,420,840,420,360,900,840,900,840,900,870,900,360,420,840,900,840,900,870,900,870,900,870,420,360,900,360,420,840,900,840,900,870,420,360,900,870,900,360,420,870,420,360,900,360,420,840,900,870,900,870,900,870,900,840,900,870,900,840,420,360,900,360,420,870,900,870,900,840,900,870,420,360,900,870,900,360,420,840,900,840,420,360,900,360,420,840,900,870,420,360,900,840,900,360,420,840,420,360,900,840,900,840,900,360,420,840,900,840,420,360,900,360,420,840,420,360,900,360,420,870,420,360,900,360,420,870,420,360,900,360,420,840,300,6990
      // ==================================================================================
      if(id == 0xfa28 || id == 0x1a2d || id == 0x1a3d || (id&0xfff)==0xACC || id == 0xca2c || id == 0xfab8 || id == 0xaad1) {
        if (id == 0xaad1) { // "The Oregon simulator", conversion code
           //The Oregon simulator: [Oregon V2.1 encoder]
           //THGR228N - temperature/humidity
           //Temperature = 21.00C
           //Humidity = 41.50%
           //1A2D20BB0021100430
           //1+a+2+d+2+0+b+b+0+0+2+1+1+4=3a - a = 30
           //20;01;Oregon Unknown;DEBUG=aad102b20b2002422003000000;
           // M.G. Remove 4 Sync bits (lower 4 bits of first byte)
           for(byte x=0; x<13;x++) {
              osdata[x] = (osdata[x] >> 4);
              if (x < 12) osdata[x] += (osdata[x+1] & 0x0f) << 4;
           }
           //id=(osdata[0]<<8)+ (osdata[1]);
           //rc=osdata[0];
        } // end conversion of simulator data     
        if ( checksum(1,8,osdata[8]) !=0) return false;   // checksum = all nibbles 0-15 results is nibbles 16.17
        // -------------       
        temp = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + ((osdata[4] >> 4));
        if ((osdata[6] & 0x0F) >= 8) temp=temp | 0x8000;
        // -------------       
        hum = ((osdata[7] & 0x0F)*16)+ (osdata[6] >> 4);
        comfort=(osdata[7])>>6;
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        Serial.print(F(";Oregon TempHygro;ID="));           // Label
        PrintHexByte(osdata[1]);
        PrintHexByte(osdata[3]);
        // ----------------------------------
        sprintf(pbuffer, ";TEMP=%04x;", temp);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "HUM=%02x;", hum);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "HSTATUS=%d;", comfort);
        Serial.print( pbuffer );
        if (((osdata[4])&0x04) == 0) {
           Serial.print(F("BAT=OK;")); 
        } else {
           Serial.print(F("BAT=LOW;")); 
        }        
        Serial.println();
      } else
      // ==================================================================================
      // 5a5d   Indoor Temp/Hygro/Baro: Huger - BTHR918, BTHGN129 
      // 5a6d   Indoor Temp/Hygro/Baro: BTHR918N, BTHR968. BTHG968 
      // OSV2 : TEMP + HUM + BARO + FORECAST + BAT + CRC
      // 0 1 2 3 4 5 6 7 8 9 a
      // 0123456789012345678901
      // 5a5d43cb00147006cc2061
      // 5+a+5+d+4+3+c+b+0+0+1+4+7+0+0+6+c+c+2+0=6b-a=61  [61]
      // ==================================================================================
      if(id == 0x5a6d || id == 0x5a5d || id == 0x5d60) {
        if ( checksum(1,10,osdata[10]) !=0) return false;   // checksum = all nibbles 0-19 result is nibbles 20.21
        // -------------       
        temp = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + ((osdata[4] >> 4));
        if ((osdata[6] & 0x0F) >= 8) temp=temp | 0x8000;
        // -------------       
        hum = ((osdata[7] & 0x0F)*16)+ (osdata[6] >> 4);
        // -------------       
        comfort=(osdata[7])>>6;                     //highest 2 bits:  00: normal, 01: comfortable, 10: dry, 11: wet
        // -------------       
        baro = (osdata[8] + 856);                   // max value = 1111 / 0x457  in hPa
        // -------------       
        int tmp_forecast = osdata[9]>>4;            //2: cloudy, 3: rainy, 6: partly cloudy, C: sunny
        if (tmp_forecast == 0x02) forecast = 3;     // 0010
        else 
        if (tmp_forecast == 0x03) forecast = 4;     // 0011
        else 
        if (tmp_forecast == 0x06) forecast = 2;     // 0110
        else 
        if (tmp_forecast == 0x0C) forecast = 1;     // 1100
        else forecast = 0;
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        Serial.print(F(";Oregon BTH;ID="));        // Label
        PrintHexByte(osdata[0]);
        PrintHexByte(osdata[2]);
        // ----------------------------------
        sprintf(pbuffer, ";TEMP=%04x;", temp);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "HUM=%02x;", hum);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "HSTATUS=%d;", comfort);
        Serial.print( pbuffer );
        sprintf(pbuffer, "BARO=%04x;", baro);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "BFORECAST=%d;", forecast);
        Serial.print( pbuffer );
        // ----------------------------------
        if (((osdata[4])&0x04) == 0) {              // Battery byte 4, low nibble is 100-10*bits 0-3 = battery %
           Serial.print(F("BAT=OK;")); 
        } else {
           Serial.print(F("BAT=LOW;")); 
        }        
        Serial.println();
      } else
      // ==================================================================================
      // 2914   Rain Gauge:
      // 2d10   Rain Gauge:
      // 2a1d   Rain Gauge: RGR126, RGR682, RGR918, RGR928, PCR122
      // OSV. : RAIN + BAT 
      // NO CRC YET
      // 0 1 2 3 4 5 6 7 8 9
      // 01234567890123456789
      // 2A1D0065502735102063 
      // 2+A+1+D+0+0+6+5+5+0+2+7+3+5+1+0+2+0=3e-a=34 != 63 
      // 2+A+1+D+0+0+6+5+5+0+2+7+3+5+1+0+2+0+6=44-a=3A 
      // ==================================================================================
      if(id == 0x2a1d || id == 0x2d10 || id == 0x2914) { // Rain sensor
        //Checksum - add all nibbles from 0 to 8, subtract 9 and compare
        /*
        int cs = 0;
        for (byte i = 0; i < pos-2; ++i) { //all but last byte
            cs += data[i] >> 4;
            cs += data[i] & 0x0F;
        }
        int csc = (data[8] >> 4) + ((data[9] & 0x0F)*16);    
        cs -= 9;  //my version as A fails?
        Serial.print(csc); 
        Serial.print(" vs ");   
        Serial.println(cs);
        */
        rain = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + (osdata[4] >> 4);
        raintot = ((osdata[7]  >> 4) * 10)  + (osdata[6]>>4);
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        Serial.print(F(";Oregon Rain;ID="));           // Label
        PrintHexByte(osdata[0]);
        PrintHexByte(osdata[3]);
        // ----------------------------------
        sprintf(pbuffer, ";RAIN=%04x;", rain);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "RAINTOT=%04x;", raintot);     
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print(F("BAT=LOW;")); 
        } else {        
           Serial.print(F("BAT=OK;")); 
        }        
        Serial.print(F("DEBUG="));                 // Label
        PrintHexByte(datLen);
        PrintHexByte(found);
        PrintHex8( osdata, datLen+1);
        Serial.println(";");  
        //Serial.println();
      } else
      // ==================================================================================
      // 2a19   Rain Gauge: PCR800
      // OSV3 : RAIN + BAT + CRC
      // 0 1 2 3 4 5 6 7 8 9 a b c 
      // 01234567890123456789012345
      // 2A19048E399393250010 
      // 2a190445000040573510f30500
      // 2+A+1+9+0+4+8+E+3+9+9+3+9+3+2+5+0+0=5b-A=51 => [1]0 
      // 2+a+1+9+0+4+4+5+0+0+0+0+4+0+5+7+3+5=3b-a=31 => [1]0f30500
      // ==================================================================================
      // Sample Data: 
      //  0        1        2        3        4        5        6        7        8        9 
      //  2A       19       04       05       39       93       33       13       01       80 
      //  0   1    2   3    4   5    6   7    8   9    A   B    C   D    E   F    0   1    2   3     
      //  00101010 00011001 00000100 00000101 00111001 10010011 00110011 00010011 00000001 10000000  
      //  [id----- -------] bbbb---  RRRRRRRR 99998888 BBBBAAAA DDDDCCCC FFFFEEEE 11110000 cccc2222
      //   
      // bbbb       = Battery indicator??? (7) 
      // RRRRRRRR   = Rolling Code ID 
      // 210.fed    = Total rain fall (inches)
      // BA98       = Current Rain Rate (inches per hour) 
      // cccc       = CRC 
      //
      //Three tips caused the following 
      //Rain total: 11.72   rate: 39.33   tips: 300.41 
      //Rain total: 11.76   rate: 0.31   tips: 301.51 
      //Rain total: 11.80   rate: 0.31   tips: 302.54 
      //1 tip=0.04 inches or mm? 
      //My experiment 
      //24.2 converts reading below to mm (Best calibration so far) 
      //0.127mm per tip 
      
      //        ba98  0000               210.fed: 035.574 
      //      2a190445000040573510f30500
      //              98badcfe10c2   
      // OSV3 2A19048E399393250010 
      //      01234567890123456789
      //      0 1 2 3 4 5 6 7 8 9
      // 2+A+1+9+0+4+8+E+3+9+9+3+9+3+2+5=5b-A=51 => 10 
      // 1+9+0+4+8+E+3+9+9+3+9+3+2=4a+5=4f
      // 2+A+1+9+0+4+0+5+3+9+9+3+3+3+1+3=    01       80 
      // 1a8904cc70c0186002[52]620000
      // 1+a+8+9+4+c+c+7+c+1+8+6+2=5c-a=52
      // 2a1904450000405735[10]f30500
      // 1+9+0+4+4+5+0+0+0+0+4+0+5+7+3+5+1=30   0f30500
      // 2+a+1+9+0+4+4+5+0+0+0+0+4+0+5+7+3+5=3b
      if(id == 0x2a19) { // Rain sensor
        int sum = (osdata[9] >> 4);  
        if ( checksum(3,9,sum) !=0) { // checksum = all nibbles 0-17 result is nibble 18
            //Serial.print("CRC Error, "); 
            return false;
        }
        rain = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + (osdata[4] >> 4);
        //Serial.print(" RainTotal=");
        //raintot = ((osdata[7]  >> 4) * 10)  + (osdata[6]>>4);
        //Serial.print(raintot); 
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        Serial.print(F(";Oregon Rain2;ID="));           // Label
        PrintHexByte(osdata[0]);
        PrintHexByte(osdata[4]);
        // ----------------------------------
        sprintf(pbuffer,";RAIN=%04x;", rain);     
        Serial.print( pbuffer );
        //sprintf(pbuffer, "RAINTOT=%04x;", raintot);     
        //Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print(F("BAT=LOW;")); 
        } else {        
           Serial.print(F("BAT=OK;")); 
        }        
        Serial.print(F(";DEBUG="));                 // Label
        PrintHexByte(datLen);
        PrintHexByte(found);
        PrintHex8( osdata, datLen+1);
        Serial.println(";");  
        
        //Serial.println();
      } else
      // ==================================================================================
      // 1a89  Anemometer: WGR800 - Wind speed sensor  (aka id 1984)
      // WIND DIR + SPEED + AV SPEED + CRC
      // ==================================================================================
      // Sample Data: 
      // 0        1        2        3        4        5        6        7        8        9 
      // 1A       89       04       e8       00       c0       07       40       00       43 
      // 0   1    2   3    4   5    6   7    8   9    A   B    C   D    E   F    0   1    2   3 
      // 00011010 10001001 00000100 11101000 00000000 11000000 00000111 01000000 00000000 01000011
      // [id----- -------] ----bbbb RRRRRRRR dddd---- xxxxEEEE DDDDCCCC HHHHGGGG FFFF---- cccc---- 
      // Av Speed 0.400m/s WindGust: 0.7m/s  Direction: N  
      //
      // bbbb       = Battery indicator???  
      // RRRRRRRR   = Rolling Code 
      // dddd       = Direction 
      // CD.E       = Gust Speed (m per sec)     multiply by 3600/1000 for km/hr 
      // FG.H       = Avg Speed(m per sec) 
      // cccc       = crc
      // 
      // OSV3 1A89048800C026400543
      // OSV3 1A89048800C00431103B
      // OSV3 1a89048848c00000003e W
      // OSV3 1a890488c0c00000003e E     
      //      1A89042CB0C047000147      
      //      1A89042CC0C019800250
      //      0 1 2 3 4 5 6 7 8 9 
      // 1+A+8+9+0+4+8+8+0+0+C+0+0+4+3+1+1+0=45-a=3b
      //20;05;Oregon Wind;ID=1ACC;WINDIR=0007;WINGS=032a;WINSP=0006;BAT=OK;;DEBUG=0D03 1a8904cc70c018600252620000 0000;
      // 1a8904cc70c0186002[52]620000
      // 1+a+8+9+4+c+c+7+c+1+8+6+2=5c-a=52
      if(id == 0x1a89) { // Wind sensor
        if ( checksum(1,9,osdata[9]) !=0) return false;
        wdir=((osdata[4] >> 4) & 0x0f);
        // -------------       
        wspeed = (osdata[6] >> 4) * 10;
        wspeed = wspeed + (osdata[6] &0x0f) * 100;
        wspeed = wspeed + (osdata[5] &0x0f);
        // -------------       
        awspeed = (osdata[8] >> 4) * 100;
        awspeed = awspeed + (osdata[7] &0x0f) * 10;
        awspeed = awspeed + (osdata[7] >> 4);
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        Serial.print(F(";Oregon Wind;ID="));           // Label
        PrintHexByte(osdata[0]);
        PrintHexByte(osdata[3]);
        // ----------------------------------
        sprintf(pbuffer, ";WINDIR=%04d;", wdir);       // pass decimal value 0-15
        Serial.print( pbuffer );
        sprintf(pbuffer, "WINGS=%04x;", wspeed);       // gust windspeed
        Serial.print( pbuffer );
        sprintf(pbuffer, "WINSP=%04x;", awspeed);      // average windspeed
        Serial.print( pbuffer );
        if ((osdata[2] & 0x0F) > 7) {  // high bit of nibble only?
           Serial.print(F("BAT=LOW;")); 
        } else {        
           Serial.print(F("BAT=OK;")); 
        }        
        Serial.print(F(";DEBUG="));                 // Label
        PrintHexByte(datLen);
        PrintHexByte(found);
        PrintHex8( osdata, datLen+1);
        Serial.println(";");  
        //Serial.println();
      } else
      // ==================================================================================
      // 3a0d  Anemometer: Huger-STR918, WGR918
      // WIND DIR + SPEED + AV SPEED + BAT + CRC
      // 3A0D006F400800000031
      // ==================================================================================
      if(id == 0x3A0D || id == 0x1984 || id == 0x1994 ) {
        if ( checksum(1,9,osdata[9]) !=0) {
            return false;
        }
        wdir = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F * 10) ) + (osdata[4] >> 4);    
        wdir=wdir / 22.5;
        wspeed = ((osdata[7] & 0x0F) * 100)  + ((osdata[6]>>4) * 10)  + ((osdata[6] & 0x0F)) ;
        awspeed = ((osdata[8]>>4) * 100)  + ((osdata[8] & 0x0F) * 10)+((osdata[7] >>4)) ;      
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        Serial.print(F(";Oregon Wind2;ID="));           // Label
        PrintHexByte(osdata[0]);
        PrintHexByte(osdata[2]);
        // ----------------------------------
        sprintf(pbuffer, ";WINDIR=%04d;", wdir);        // pass decimal value 0-15
        Serial.print( pbuffer );
        sprintf(pbuffer, "WINGS=%04x;", wspeed);       // gust windspeed
        Serial.print( pbuffer );
        sprintf(pbuffer, "WINSP=%04x;", awspeed);      // average windspeed
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print(F("BAT=LOW;")); 
        } else {        
           Serial.print(F("BAT=OK;")); 
        }        
        Serial.println();      
	  } else
      // ==================================================================================
      // ea7c  UV Sensor: UVN128, UV138
      // UV + BAT
      // NO CRC YET
      // ==================================================================================
      if(id == 0xea7c) { 
        uv=((osdata[5] & 0x0F) * 10)  + (osdata[4] >> 4);
        Serial.print(uv); 
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        Serial.print(F(";Oregon UVN128/138;ID="));           // Label
        PrintHexByte(osdata[0]);
        PrintHexByte(osdata[2]);
        // ----------------------------------
        sprintf(pbuffer, ";UV=%04x;", uv);     
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print(F("BAT=LOW;")); 
        } else {        
           Serial.print(F("BAT=OK;")); 
        }        
        Serial.print(F(";DEBUG="));                 // Label
        PrintHexByte(datLen);
        PrintHexByte(found);
        PrintHex8( osdata, datLen+1);
        Serial.println(";");  
        //Serial.println();   
      } else
      // ==================================================================================
      // da78  UV Sensor: UVN800
      // UV + BAT + CRC
      // transmits every 73 seconds
      // measures: UV and risk message: low/moderate/high/very high/extremely high
      // also??? outdoor temp -20 - 60 degrees,
      // 20;0D;Oregon UVN800;ID=DA38;UV=00f0;BAT=LOW;
      // 20;0E;Oregon UVN800;DEBUG=da78143820f008454e00000000;
      // OSV3 DA78146500F00641B5 donker 
      //      DA7814651050083AF2 zon
      //      da78143820f008454e
      //      da78144608d0064611
      //      012345678901234567
      //      0 1 2 3 4 5 6 7 8 9 
      //              lh = uv       (inital value = 0x6f?)
      //            aa = address
      // CRC: d+a+7+8+1+4+3+8+2+0+f+0+0+8+4+5=58-a=4e  > 4e? OK
      // ==================================================================================
      if( id == 0xda78) { 
        if ( checksum(1,8,osdata[8]) !=0) return false;
        uv=((osdata[4] & 0xf0) >>4) + ((osdata[4] &0x0f)<<4); // Nibble swapped UV value 
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        Serial.print(F(";Oregon UVN800;ID="));      // Label
        PrintHexByte(osdata[0]);
        PrintHexByte(osdata[3]);
        // ----------------------------------
        sprintf(pbuffer, ";UV=%04x;", uv);     
        Serial.print( pbuffer );
        if ((osdata[2] & 0x08) == 8) {
           Serial.print(F("BAT=LOW;")); 
        } else {        
           Serial.print(F("BAT=OK;")); 
        }    
        Serial.print(F(";DEBUG="));                 // Label
        PrintHexByte(datLen);
        PrintHexByte(found);
        PrintHex8( osdata, datLen+1);
        Serial.println(";");  
        //Serial.println();     
      } else
      // ==================================================================================
      // *aec  Date&Time: RTGR328N
      // 8A EC 13 FC 60 81 43 91 11 30 0 0 0 ;
      // 8+A+E+C+1+3+F+C+6+0+8+1+4+3+9+1=6B -A=61  != 30
      // 8+A+E+C+1+3+F+C+6+0+8+1+4+3+9=6a-A=60 0=0
      // NO CRC YET
      //20;06;Oregon Unknown;DEBUG=8A EC 13 FC 60 81 43 91 11 30 0 0 0 ;
      //20;20;Oregon Unknown;DEBUG=8A EC 13 FC 40 33 44 91 11 30 0 0 0 ;
      // ==================================================================================
      // 8AEA1378077214924242C16CBD  21:49 29/04/2014 
      // 0 1 2 3 4 5 6 7 8 9 0 1 2
      // 8+A+E+A+1+3+7+8+0+7+7+2+1+4+9+2+4+2+4+2+C+1+6+C=88 !=   BD
      // Date & Time
      // ==================================================================================
      // eac0  Ampere meter: cent-a-meter, OWL CM113, Electrisave
      // ==================================================================================
      if( (id&0xff00)==0xea00 ) { // Any Oregon sensor with id 2axx 
        if ((osdata[9] == 0xff) && (osdata[10] == 0x5f)) { 
           int phase1=(((osdata[3]))+((osdata[4]&0x3)<<8))/10;          // courant en ampères
           int phase2=(((osdata[4]&0xFC)>>2)+((osdata[5]&0xF)<<6))/10;  // courant en ampères
           int phase3=(((osdata[5]&0xF0)>>4)+((osdata[6]&0x3F)<<4))/10; // courant en ampères
            // ----------------------------------
            // Output
            // ----------------------------------
            Serial.print("20;");
            PrintHexByte(PKSequenceNumber++);
            // ----------------------------------
            Serial.print(F(";Oregon CM113;ID="));
            PrintHexByte(osdata[1]); 
            PrintHexByte(osdata[2]);             
            sprintf(pbuffer, ";WATT=%04x", phase1&0xfffff); 
            Serial.print(pbuffer);
            Serial.println(";");
        } else {
            // ----------------------------------
            // Output
            // ----------------------------------
            Serial.print("20;");
            PrintHexByte(PKSequenceNumber++);
            // ----------------------------------
            Serial.print(F(";Oregon CM113;DEBUG="));                 // Label
            PrintHexByte(datLen);
            PrintHexByte(found);
            PrintHex8( osdata, datLen+1);
            Serial.println(";");  
            return false;
        }
      } else  
      // ==================================================================================
      // 0x1a* / 0x2a* 0x3a** Power meter: OWL CM119 / OWL CM160
      // 20;74;Oregon Unknown; DEBUG = 2a00ea2401e0b2390000000000;
      // 2a00ea2401e0b23900
      // 0 1 2 3 4 5 6 7 8
      // 2+a+0+0+e+a+2+4+0+1+e+0+b+2=46-d=39    39
      // 2+a+6+f+b+9+1+0+0+1+e+0=45   a3?
      // 2+a+6+e+b+9+1+0+0+1+0+0=36   a6?
      
      // ==================================================================================
      // OWL CM160
      // CM160
      // 2A806D3703C079B17300
      // 0 1 2 3 4 5 6 7 8
      // 2+A+8+0+6+D+3+7+0+3+C+0+7+9+B+1=5c   73
      /*
      So for this example that gives me 0x0330 (masking out the 7 didn't make sense) which is 816 in decimal. 
      This is very close to the wattage displayed on my energy monitor as it turned out I had to multiply the 
      decoded number by 1.006196884 to get the value displayed on the energy monitor (this works for all ranges 
      of wattages). C-code to do this:
      watts = ((data[4] * 256) + (data[3] & 0xF0)) * 1.006196884; 
      
      As to the rest of the OSV packet, byte 2 is always 6D, byte 0 is always 2A (manufacturer code), 
      lower nibble of byte 3 is always 7, couldn't work out bytes 5 or 6, byte 7 seems to be some kind of counter 
      and the '~7300'_T at the end is always there.
      */
      // ==================================================================================
      if( (id&0xff00)==0x2a00 ) { // Any Oregon sensor with id 2axx 
		unsigned int watts = 0;
        watts = ((osdata[4] * 256) + (osdata[3] & 0xF0)) * 1.006196884; 
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        // ----------------------------------
        if (osdata[2] == 0x6d) {
           Serial.print(F(";Oregon CM160;ID="));                
        } else {
           Serial.print(F(";Oregon CM119;ID="));                
        }
        PrintHexByte(osdata[0]);
        PrintHexByte(osdata[1]&0xf0);
        sprintf(pbuffer, ";KWATT=%04x", watts&0xffff); 
        Serial.print(pbuffer);
        if (datLen > 10) {        
           /*
           byte chksum_CM119=0; 
           for(byte i=1; i<7; i++){
              chksum_CM119 += (byte)(osdata[i]&0x0F) + (byte)(osdata[i]>>4) ;
           }
           byte tempval=((osdata[11])>>4)&0x0f;
           byte tempval2=((osdata[12])<<4)&0xf0;
           tempval=tempval+tempval2;
           if (chksum_CM119 != tempval) Serial.print(";BAD_CRC?"); 

           unsigned long long total=0LL;   // type long long is necessary for the big values permitted
           total+=(unsigned long long)osdata[10]<<36;	//UV000000000
           total+=(unsigned long long)osdata[9]<<28; 	//  ST0000000
           total+=(unsigned long long)osdata[8]<<20; 	//    QR00000
           total+=(unsigned long long)osdata[7]<<12; 	//      OP000
           total+=(unsigned long long)osdata[6]<<4;        //        MN0   
           total+=(unsigned long long)(osdata[5]>>4)&0X0F; //          K
           total= (unsigned long long)((total*1000LL)/223666LL);
           sprintf(pbuffer, "T=%08lx;", total); 
           Serial.print(pbuffer); 
           */
           Serial.print(F(";DEBUG="));                // Label
           PrintHexByte(datLen);
           PrintHexByte(found);
           PrintHex8( osdata, datLen+1);
        }
        Serial.println(";");  
      } else
      // ==================================================================================
      // 0x62b* Power meter: OWL CM180
      // ==================================================================================
      // 6284 3C 7801 D0 
      // 6280 3C 2801 A0A8BA05 00 00 ?? ?? ?? 
      // 62b0 2c 2801 808bd5270300 0000 
      //
      // 13 bytes(long packet)     :   AB CD EF GH IJ KL MN OP QR ST UV WX YZ
      // osdata 		           :    0  1  2  3  4  5  6  7  8  9 10 11 12
      // 6 bytes(short packet)     :   AB CD EF GH IJ KL
      // ABC = 628;  fixed : type OWL CM180  (Note : the RFXTRX-usb don't use C=8 )   
      // D = ??;    it seems D = 0, long packet (13 bytes)
      //		     D > 0 (1,2..4), short packet (6 bytes), no counter's total, just instant power
      // HEF = Device's home-id; 
      // L IJ G : instant power = (L*16*16*16 + I*16*16 +J*16+G) * 16 * (500/497) Watt
      // UV ST QR OP MN K: counter's total=(U*16^10+V*16^9+S*16^8+T*16^7+Q*16^6+R*16^5+O*16^4+P*16^3+M*16^2+N*16+K)*1000/223666
      // X Z W  :checksum long packet = A+B+C+D+E+F+G+H+I+J+K+L+M+N+O+P+Q+R+S+T+U+V -2 = X*16^2+Z*16 + W
      // Y : ???? 
      // ==================================================================================
      if( (id&0xff00)==0x6200 ) { // Any Oregon sensor with id 62xx 
        if ((datLen!=13) && (datLen!=6)) return false;       //incomplete (corrupted) packets are eliminated
        if ((datLen==6) && (((osdata[1])&0X0F)==0)) return false; //if a long packet (osdata[1]=.0) is corrupted (and become a short packet),it is eliminated
        unsigned long long total=0LL;   // type long long is necessary for the big values permitted
        unsigned long val = 0L;

        val += (osdata[5]&0X0F)<<12;    //L000
        val += osdata[4]<<4;            // IJ0
        val += (osdata[3]&0XF0)>>4;     //   G
        val =  (val*500L*16L)/497L;     // type long is necessary for the big values permitted and precision in the calculation
        	
        if (datLen==13) { //calculate checksum for long packet and exit if false 
           unsigned int chksum_CM180=0; 
           for(byte i=0; i<11; i++){
              chksum_CM180 += (unsigned int)(osdata[i]&0x0F) + (unsigned int)(osdata[i]>>4) ;
           }
           chksum_CM180 -=2;
           if (osdata[11]!=(((chksum_CM180&0x0F)<<4) + ((chksum_CM180>>8)&0x0F))||((osdata[12]&0x0F)!=(chksum_CM180>>4)&0x0F)) return false;
	                     
           total+=(unsigned long long)osdata[10]<<36;	//UV000000000
           total+=(unsigned long long)osdata[9]<<28; 	//  ST0000000
           total+=(unsigned long long)osdata[8]<<20; 	//    QR00000
           total+=(unsigned long long)osdata[7]<<12; 	//      OP000
           total+=(unsigned long long)osdata[6]<<4;        //        MN0   
           total+=(unsigned long long)(osdata[5]>>4)&0X0F; //          K
           total= (unsigned long long)((total*1000LL)/223666LL);
           previous_total=total; //see below
        }
        // ----------------------------------
        if (datLen==6) { //use the last total stored in global variable because there's no total data in the packet
           total=previous_total;
           // if it's a short packet (no information about counter'stotal) domoticz considers in this case
           // the counter's total as null (and difference between counter's totals = energy consumption for domotiz) 
           // so we have to use/return the previous total (stored in previous_total) to avoid an error of daily consumption 
        }
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        // ----------------------------------
        Serial.print(F(";Oregon CM180;ID="));
        PrintHexByte(((osdata[3]&0x0f)<<8)); //H  (HEF : Device's ID)    
        PrintHexByte(osdata[2]);             //EF (HEF : Device's ID)            
        sprintf(pbuffer, ";WATT=%04x", val&0xfffff); 
        Serial.print(pbuffer);
        sprintf(pbuffer, ";KWATT=%08lx", total); 
        Serial.print(pbuffer); 
        Serial.println(";");
      } else
      // ==================================================================================
      // 1a99  Anemometer: WGTR800 / WGR800 original - Wind speed sensor with temp/hum
      // WIND + TEMP + HUM + CRC
      // ==================================================================================
      if(id == 0x1a99) { // Wind sensor
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        // ----------------------------------
        Serial.print(F(";Oregon WGR800;DEBUG="));                // Label
        PrintHexByte(datLen);
        PrintHexByte(found);
        PrintHex8( osdata, datLen+1);
        Serial.println(";");  
      } else    
      // ==================================================================================
      if( (id&0xf00)==0xA00 ) { // Any Oregon sensor with id xAxx 
        // ----------------------------------
        // Output
        // ----------------------------------
        Serial.print("20;");
        PrintHexByte(PKSequenceNumber++);
        // ----------------------------------
        Serial.print(F(";Oregon Unknown;DEBUG="));                // Label
        PrintHexByte(datLen);
        PrintHexByte(found);
        PrintHex8( osdata, datLen+1);
        Serial.println(";");  
        return false;
      }
      // ==================================================================================
      RawSignal.Repeats=true;                    // suppress repeats of the same RF packet 
      RawSignal.Number=0;
      return true;
}
#endif // PLUGIN_048
