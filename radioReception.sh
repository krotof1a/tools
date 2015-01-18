#!/bin/sh

while [ true ]
do
	/home/pi/tools/radioReception \
		http://localhost:5665/json.htm \
		2 \
		10 16 \
		736 14 \
		41 27 \
		809 34 2> /dev/zero
	sleep 3 # Wait in case reception has been killed for emission
done

return 0;
