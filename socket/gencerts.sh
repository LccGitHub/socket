#!/bin/sh


rm *.pem *.csr *.srl


echo "Gen CA Cert"
openssl genrsa -out private.pem 4096
openssl req -new -x509 -days 7200 -key private.pem -out rootca_cert.pem
echo "Done."
echo "\n\n"

echo "Gen Server Cert..."
openssl req -out temp.csr -new -newkey rsa:4096 -nodes -outform PEM -keyout server_key.pem
openssl x509 -req -in temp.csr -CA rootca_cert.pem -CAkey private.pem -CAcreateserial -outform PEM -out server_cert.pem -days 365 -sha256

echo "\n\n"
openssl verify -CAfile rootca_cert.pem server_cert.pem
echo "Done."
echo "\n\n"

echo "Gen Client Cert..."
openssl req -out temp.csr -new -newkey rsa:4096 -nodes -outform PEM -keyout client_key.pem
openssl x509 -req -in temp.csr -CA rootca_cert.pem -CAkey private.pem -CAcreateserial -outform PEM -out client_cert.pem -days 365 -sha256
echo "\n\n"
openssl verify -CAfile rootca_cert.pem client_cert.pem
echo "Done."
