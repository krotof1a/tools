#!/bin/sh

curl -s -H "Content-Type: application/json" -H "X-Api-Key: 123CEDB65F504A7997870642936020B6" -X POST -d '{ "command":"connect" }' http://192.168.1.33/api/system/commands/core/shutdown
