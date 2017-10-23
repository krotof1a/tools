import getpass
import sys
import telnetlib

tn = telnetlib.Telnet("192.168.1.3",8102)

if sys.argv[1] == 'on':
	tn.write("PO\r\n")
elif sys.argv[1] == 'off':
	tn.write("PF\r\n")
else:
	sys.stdout.write('Command not recognized\n')
tn.close()

