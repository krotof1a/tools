#!/bin/sh

/home/pi/tools/speakCS.sh "Attention! Intrusion en cours!"
while [ true ]
do
  aplay /home/pi/tools/siren.wav > /dev/zero 2>&1
  if [ $? -ne 0 ]
  then
	break
  fi
done
