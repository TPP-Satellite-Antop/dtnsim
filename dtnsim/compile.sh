#!/bin/bash

rm -rf out

# Build antop
(
  cd ../external/lib
  git pull
  ./lib-build.sh
)

# Build dtnsim
make