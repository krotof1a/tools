#!/bin/bash

ip="192.168.1.20" # ip locale
port="5665" # port Domoticz
idx_planning="284" # idx de l'interrupteur Planning

date_serveur=$(curl -s "http://$ip:$port/json.htm?type=command&param=getSunRiseSet" | jq -r '.ServerTime | strptime("%Y-%m-%d %H:%M:%S") | mktime')
/home/pi/build/433Utils/RPi_utils/codesend $date_serveur 1
sleep 1

/home/pi/tools-domo/sendEvent.sh 1
sleep 1

/home/pi/tools-domo/sendEvent.sh 0
