#!/usr/bin/bash
# sign $1 using the SafeNet token
java -jar $HOME/usr/bin/jsign-7.6.jar --storetype PKCS11 --keystore $HOME/usr/etc/SafeNet.cfg -t http://timestamp.sectigo.com -r 10 -w 15 --storepass "$WINDOWS_SIGN_TOKEN_PASSWORD" $1

