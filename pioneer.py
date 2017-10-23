import sys
import telnetlib

tn = telnetlib.Telnet("192.168.1.3",8102)

if sys.argv[1] == 'on':
	tn.write("PO\r\n")
elif sys.argv[1] == 'off':
	tn.write("PF\r\n")
elif sys.argv[1] == 'vol':
	goal = int(sys.argv[2]) * 1.5
	tn.write("?V\r\n")
	output = tn.read_until("\r\n")
	currentlevel = int(output[3:])
	if goal < 150: # my set upper limit to prevent damage to speakers
  		if currentlevel > goal:
    			for x in range(currentlevel, goal, -1):
      				tn.write("VD\r\n")
  		else:
    			for x in range(currentlevel, goal, 1):
      				tn.write("VU\r\n")
	else:
  		sys.stdout.write('Given voulume is out of range\n')
else:
	sys.stdout.write('Command not recognized\n')
tn.close()
