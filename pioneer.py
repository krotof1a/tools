#!/usr/bin/env python

import getpass
import sys
import telnetlib

tn = telnetlib.Telnet("192.168.1.3",8102)

if (sys.argv[1]=='off'):
     print 'Turn AVR off'
     tn.write("PF\n\r")
if (sys.argv[1]=='on'):
     print 'Turn AVR on'
     tn.write("PO\n\r")

tn.close()

