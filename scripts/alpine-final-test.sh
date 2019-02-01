#!/bin/bash
docker run -v ${PWD}:/build -i node:${NODE_NVM_VERSION}-alpine /bin/sh <<-EOF
cd /build
./node_modules/.bin/node-pre-gyp install $EXTRA_NODE_PRE_GYP_FLAGS
npm test
EOF
sudo chown -R travis ${PWD}
