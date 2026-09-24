#!/usr/bin/bash
# sign $1 using the SafeNet token
java -jar $HOME/usr/bin/jsign-6.0.jar --storetype ETOKEN --keystore ~/usr/etc/SafeNet.cfg -t http://timestamp.sectigo.com -r 10 -w 15 --storepass "$WINDOWS_SIGN_TOKEN_PASSWORD" $1
