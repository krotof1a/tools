#!/bin/sh

# Create FIFO if not exists
FIFO=/tmp/radioReceptionFIFO
if [ ! -p $FIFO ]
then
	mkfifo $FIFO
fi

while [ true ]
do
	/home/osmc/tools/radioReception \
		http://localhost:5665/json.htm \
		2 \
		10 16 \
		736 14 \
		737 52 \
		41 27 \
		809 34 \
		489 54 \
		2>&1 | /home/osmc/tools/ftee $FIFO > /dev/zero
	sleep 3 # Wait in case reception has been killed for emission
done

\rm -f $FIFO
return 0;
