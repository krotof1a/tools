#!/bin/sh

/home/pi/tools/speakCS.sh "Attention! Intrusion en cours!"
while [ true ]
do
  aplay /home/pi/tools/sirene.wav
done
