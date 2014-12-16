#!/bin/sh

while [ true ]
do
	/home/pi/tools/radioReception \
		http://88.124.222.120:5665/json.htm \
		2 \
		10 16 \
		85 14
	sleep 5 # Wait in case reception has been killed for emission
done

return 0;
