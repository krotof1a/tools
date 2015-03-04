#!/bin/sh

if [ "$1" = "stop" ]
then
	echo "Stopping running alarm ..."
	sudo killall aplay alarme.sh
else if [ "$1" = "start" ]
then
	echo "Starting alarm ..."
	sleep 10
	sudo tts "Attention\! Intrusion en cours\!"
	while [ true ]
	do
  		sudo aplay /home/osmc/tools/siren.wav > /dev/zero 2>&1
	done
else if [ "$1" = "init" ]
then
	echo "Arming alarm ..."
	sudo tts "Attention\! Alarme en fonction\! Plus que dix secondes pour fermer les issues ..."
	sleep 10
	wget -o /dev/zero -O /dev/zero "http://localhost:5665/json.htm?type=command&param=setsecstatus&secstatus=2&seccode=$2"
else if [ "$1" = "finish" ]
then
	echo "Disarming alarm ..."
	wget -o /dev/zero -O /dev/zero "http://localhost:5665/json.htm?type=command&param=setsecstatus&secstatus=0&seccode=$2"
	sudo killall aplay alarme.sh
fi
fi
fi
fi
