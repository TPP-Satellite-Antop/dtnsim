#!/bin/bash

rm -rf out

# Build antop
(
  cd ../external/lib
  ./lib-build.sh
)

# Build dtnsim
make