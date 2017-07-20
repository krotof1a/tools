#!/bin/bash

ip="192.168.1.20" # ip locale
port="5665" # port Domoticz
idx_planning="284" # idx de l'interrupteur Planning
jour_sem=0
emiter=2063597568

if [ $(date +%u) -gt 5 ]
   then
   jour_sem=512
else
   jour_sem=256
fi

date_serveur=$(curl -s "http://$ip:$port/json.htm?type=command&param=getSunRiseSet" | jq -r '.ServerTime | strptime("%Y-%m-%d %H:%M:%S") | mktime')
/home/pi/build/433Utils/RPi_utils/codesend $date_serveur 1
exit 0
sleep 1

mycmd=0
timer=$(curl -s "http://$ip:$port/json.htm?type=schedules" | jq '[.result[] | select((.DeviceRowID | contains ('$idx_planning')) and ((.Days | contains('$jour_sem')) or (.Days | contains(128))) and (.TimerCmd | contains(0)))]' | jq '[.[].ScheduleDate | strptime("%Y-%m-%d %H:%M:%S") | mktime]' | jq 'min')
timer=$(expr $timer - $date_serveur)
fullCmd=$(expr $emiter + $timer)
/home/pi/build/433Utils/RPi_utils/codesend $fullCmd 1
sleep 1

mycmd=1048576
timer=$(curl -s "http://$ip:$port/json.htm?type=schedules" | jq '[.result[] | select((.DeviceRowID | contains ('$idx_planning')) and ((.Days | contains('$jour_sem')) or (.Days | contains(128))) and (.TimerCmd | contains(1)))]' | jq '[.[].ScheduleDate | strptime("%Y-%m-%d %H:%M:%S") | mktime]' | jq 'min')
timer=$(expr $timer - $date_serveur)
fullCmd=$(expr $emiter + $mycmd + $timer)
/home/pi/build/433Utils/RPi_utils/codesend $fullCmd 1
