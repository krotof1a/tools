/*
 Usage: ./sendCS <gpioPin> <senderCode> <deviceCode/"portal"> <"on"/"off"/"pulse"/portalCode> <pulseDuration>
 */

#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

void scheduler_realtime() {
	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_RR);
	if( sched_setscheduler( 0, SCHED_RR, &p ) == -1 ) {
		perror("Failed to switch to realtime scheduler.");
	}
}

void scheduler_standard() {
	struct sched_param p;
	p.__sched_priority = 0;
	if( sched_setscheduler( 0, SCHED_OTHER, &p ) == -1 ) {
		perror("Failed to switch to normal scheduler.");
	}
}

int main(int argc, char *argv[]) {
    
    	int PIN = atoi(argv[1]);
    	int sender = atoi(argv[2]);
    	int device = 9999;
    	if (argv[3] == "portal") 
    		device=-1;
    	else
    		device = atoi(argv[3]);
    	string onoff = argv[4];
    	int pulse = 0;
    	if (argc == 6) pulse = atoi(argv[5]);
	if (wiringPiSetup () == -1) return 1;
    	
    	scheduler_realtime();
    	
    	RCSwitch mySwitch = RCSwitch();
    	mySwitch.enableTransmit(PIN);
	if (device == -1) {
		// Portal command
		mySwitch.setProtocol(5);
		mySwitch.setRepeatTransmit(10);
		mySwitch.send(onoff);
		if (pulse > 0) {
			delay(pulse);
			mySwitch.send(onoff);
		}
	} else {
		// DIO command
		mySwitch.setProtocol(6);
		mySwitch.setRepeatTransmit(1);
		if (onoff=="on" || onoff=="pulse") {
			mySwitch.send(sender,device,true);
		}
		if (onoff=="pulse" && pulse > 0) {
			delay(pulse);
		}
		if (onoff=="off" || onoff=="pulse") {
			mySwitch.send(sender,device,false);
		}
	}
	
	scheduler_standard();

	return 0;
}
