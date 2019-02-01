#!/bin/bash
docker run -v ${PWD}:/build -i node:${NODE_NVM_VERSION}-alpine /bin/sh <<-EOF
cd /build
npm test
EOF
sudo chown -R travis ${PWD}
