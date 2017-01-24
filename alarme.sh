#!/bin/sh
RUN=/var/run/alarm.run

if [ "$1" = "start" ]
then
	if [ -f $RUN ]
	then
		echo "Alarm already running ... Nothing to do."
	else
		touch $RUN
		echo "Starting alarm ..."
		sleep 10
		#amixer cset numid=3 1
		while [ true ]
		do
	  		aplay /home/chip/tools-domo/siren.wav > /dev/zero 2>&1
		done
	fi
else if [ "$1" = "init" ]
then
	echo "Arming alarm ..."
	sleep 10
	wget -o /dev/zero -O /dev/zero "http://127.0.0.1:5665/json.htm?type=command&param=setsecstatus&secstatus=2&seccode=$2"
else if [ "$1" = "finish" ]
then
	echo "Disarming alarm ..."
	\rm -f $RUN
	wget -o /dev/zero -O /dev/zero "http://127.0.0.1:5665/json.htm?type=command&param=setsecstatus&secstatus=0&seccode=$2"
	killall aplay alarme.sh
fi
fi
fi
