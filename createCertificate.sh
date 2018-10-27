#!/bin/bash

if [ "$1" = "" ]
then
	echo "Script takes external IP adress as parameter"
	exit 1
fi

cd /home/pi
\rm -f demande.csr certificat.crt

if [ ! -r private.key ]
then
	echo "Private key is missing. Generate one with command:"
	echo "openssl genrsa 1024 > private.key"
	exit 1
fi

if [ ! -r ca.key ]
then
	echo "CA key is missing. Generate one with command:"
	echo "openssl genrsa -des3 1024 > ca.key"
	exit 1
fi

if [ ! -r ca.crt ]
then
	echo "CA certificate is missing. Generate one with command:"
	echo "openssl req -new -x509 -days 3650 -key ca.key > ca.crt"
	exit 1
fi

openssl req -new -key private.key -subj "/C=FR/ST=OCCITANIE/O=KROTOF1A/CN=$1" -reqexts SAN -config <(cat /etc/ssl/openssl.cnf <(printf "[SAN]\nsubjectAltName=IP:$1")) > demande.csr
openssl x509 -req -in demande.csr -out certificat.crt -CA ca.crt -CAkey ca.key -CAcreateserial -CAserial ca.srl -extensions SAN -extfile <(cat /etc/ssl/openssl.cnf <(printf "\n[SAN]\nsubjectAltName=IP:$1")) -days 3650
cat private.key certificat.crt > domoticz/server_cert.pem
\rm -f demande.csr certificat.crt ca.srl

exit 0

