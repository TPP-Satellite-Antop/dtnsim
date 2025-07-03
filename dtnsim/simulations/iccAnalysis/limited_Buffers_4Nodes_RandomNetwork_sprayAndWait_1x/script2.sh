#!/bin/bash

#remove old results
rm -rf results

#create results folders
mkdir results

cd results

mkdir results_random

cd results_random
mkdir 0.1
mkdir 0.5
mkdir 1.0
cd ..

cd ..

# python scripts to read results databases and generate .txt results

python3 deletedContactsAnalysis_random_0.1.py
python3 deletedContactsAnalysis_random_0.5.py
python3 deletedContactsAnalysis_random_1.0.py

python3 NCurvas_received.py
python3 NCurvas_transmitted.py
python3 NCurvas_deliveryRatio.py
python3 NCurvas_meanDelay.py
python3 NCurvas_meanHops.py
python3 NCurvas_meanBundlesInSDR.py

: <<'END'
END
