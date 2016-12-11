#!/bin/sh

curl -s -H "Content-Type: application/json" -H "X-Api-Key: A28235311C5C44459F7ADF1FBAA15DF5" -X POST -d '{ "command":"connect" }' http://192.168.1.33/api/system/commands/core/shutdown
