#!/bin/bash
docker run -v ${PWD}:/build -i node:${NODE_NVM_VERSION}-alpine /bin/sh <<-EOF
apk add --no-cache make gcc jq g++ python
cd /build
EXTRA_NODE_PRE_GYP_FLAGS="--target_libc=musl" npm install --build-from-source --unsafe-perm
EOF
sudo chown -R travis ${PWD}
