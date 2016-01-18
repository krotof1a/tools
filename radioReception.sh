#!/bin/sh

# Create FIFO if not exists
FIFO=/tmp/radioReceptionFIFO
if [ ! -p $FIFO ]
then
	mkfifo $FIFO
fi

while [ true ]
do
	/home/pi/tools/radioReception \
		http://localhost:5665/json.htm \
		2 \
		10 16 \
		185780960 14 \
		185780961 52 \
		202954537 34 \
		202962985 27 \
		246798825 54 \
		293464352 69 \
		235067360 69 \
		293452576 70 \
		2>&1 | /home/pi/tools/ftee $FIFO > /dev/zero
	sleep 3 # Wait in case reception has been killed for emission
done

\rm -f $FIFO
return 0;
