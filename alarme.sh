#!/bin/sh

if [ "$1" = "stop" ]
then
	echo "Stopping running alarm ..."
	sudo killall aplay alarme.sh
else
	echo "Starting alarm ..."
	sudo /home/osmc/tools/speakCS.sh "Attention\! Intrusion en cours!"
	while [ true ]
	do
  		sudo aplay /home/osmc/tools/siren.wav > /dev/zero 2>&1
	done
fi
