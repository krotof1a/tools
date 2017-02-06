#!/bin/sh

amixer sset 'Master' 60% > /dev/zero

mpg321 $1 > /dev/zero 2>&1
if [ $? -ne 0 ] 
then
	url=`curl $1 -s -L -I -o /dev/null -w '%{url_effective}'`
	mpg321 $url > /dev/zero 2>&1
fi

amixer sset 'Master' 100% > /dev/zero

