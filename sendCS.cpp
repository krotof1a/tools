/*
 Usage: ./send <systemCode> <unitCode> <command>
 Command is 0 for OFF and 1 for ON
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
    char* code = argv[2];

    scheduler_realtime();
    
    if (wiringPiSetup () == -1) return 1;
	//printf("sending systemCode[%s] unitCode[%i] command[%i]\n", systemCode, unitCode, command);
	RCSwitch mySwitch = RCSwitch();
	mySwitch.enableTransmit(PIN);
	mySwitch.setProtocol(5);
	mySwitch.setRepeatTransmit(10);
	mySwitch.send(code);

scheduler_standard();

	return 0;


}
