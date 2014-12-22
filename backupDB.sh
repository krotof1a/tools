#!/bin/sh

wget -O /media/sda1/backups/domoticz.db "http://88.124.222.120:5665/backupdatabase.php"
chmod 666 /media/sda1/backups/domoticz.db

