#!/bin/bash
set -xe

npm install -g electron@$ELECTRON_VERSION
if [ "$MOCHA_RUNTIME_VERSION" = "" ]; then 
    npm install -g electron-mocha
else
    npm install -g electron-mocha@$MOCHA_RUNTIME_VERSION
fi

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then 
    export DISPLAY=:99.0
    sh -e /etc/init.d/xvfb start
    sleep 3
fi

electron-mocha -R tap ./tests/*.js --timeout 15000
