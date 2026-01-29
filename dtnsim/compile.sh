#!/bin/bash

rm -rf out

# Build antop
(
  cd ../external/lib
  git fetch
  git checkout fix/loop-detection
  git pull
  ./lib-build.sh
)

# Build dtnsim
make