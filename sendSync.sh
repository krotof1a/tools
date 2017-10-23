#!/bin/bash

ip="192.168.1.20" # ip locale
port="5665" # port Domoticz
idx_planning="284" # idx de l'interrupteur Planning
idx_synccing="482" # idx de l'interrupteur Synccing
date_serveur=$(curl -s "http://$ip:$port/json.htm?type=command&param=getSunRiseSet" | jq -r '.ServerTime | strptime("%Y-%m-%d %H:%M:%S") | mktime')
/home/pi/build/433Utils/RPi_utils/codesend $date_serveur 1
curl -s "http://$ip:$port/json.htm?type=command&param=switchlight&idx=$idx_synccing&switchcmd=Off"
#sleep 5

curl -s "http://$ip:$port/json.htm?type=command&param=switchlight&idx=$idx_planning&switchcmd=Toggle"
sleep 10

curl -s "http://$ip:$port/json.htm?type=command&param=switchlight&idx=$idx_planning&switchcmd=Toggle"
