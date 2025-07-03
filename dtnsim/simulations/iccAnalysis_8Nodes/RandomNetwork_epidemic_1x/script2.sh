#!/bin/bash

#remove old results
rm -rf results

#create results folders
mkdir results

cd results

mkdir results_random

cd results_random
mkdir 0.2
cd ..

cd ..

# python scripts to read results databases and generate .txt results

python3 deletedContactsAnalysis_random_0.2.py

: <<'END'
END
