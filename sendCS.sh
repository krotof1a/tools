#!/bin/sh

# First step : kill reception daemon
/home/osmc/tools/killRadioReception.sh

# Second step : Start the binary to emit
/home/osmc/tools/sendCS $*

# No need to start reception daemon again as it will
# be done automaticaly after 5 seconds
