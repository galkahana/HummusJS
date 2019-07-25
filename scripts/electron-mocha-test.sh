#!/bin/bash
set -xe

npm install -g electron@$ELECTRON_VERSION
if [ "$MOCHA_RUNTIME_VERSION" = "" ]; then 
    npm install -g electron-mocha
else
    npm install -g electron-mocha@$MOCHA_RUNTIME_VERSION
fi

electron-mocha -R tap ./tests/*.js --timeout 15000
$*
