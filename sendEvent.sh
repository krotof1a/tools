#!/bin/bash

ip="192.168.1.86" # ip locale
port="5665" # port Domoticz
idx_planning="284" # idx de l'interrupteur Planning
jour_sem=0

mycmd=0
if [ $1 = "1" ]
then
	echo "On"
	mycmd=1048576
fi

if [ $(date +%u) -gt 5 ]
   then
   jour_sem=512
else
   jour_sem=256
fi

date_serveur=$(curl -s "http://$ip:$port/json.htm?type=command&param=getSunRiseSet" | jq -r '.ServerTime | strptime("%Y-%m-%d %H:%M:%S") | mktime')
# On recupere le prochain timer sur le planning
timer=$(curl -s "http://$ip:$port/json.htm?type=schedules" | jq '[.result[] | select((.DeviceRowID | contains ('$idx_planning')) and ((.Days | contains('$jour_sem')) or (.Days | contains(128))) and (.TimerCmd | contains('$1')))]' | jq '[.[].ScheduleDate | strptime("%Y-%m-%d %H:%M:%S") | mktime]' | jq 'min')
timer=$(expr $timer - $date_serveur)
echo "Timestamp prochain timer : $timer"
fullCmd=$(expr $mycmd + $timer)
/home/chip/build/433Utils/CHIP_utils/codesend $fullCmd 1
