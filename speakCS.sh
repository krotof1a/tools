#!/bin/sh

espeak -v'fr-fr' -k5 -s150 "$1" > /dev/zero 2>&1

