#!/bin/bash

#remove old results
rm -rf results

#create results folders
mkdir results

# python scripts to read results databases and generate .txt results

python3 NCurvas_delivered.py
python3 NCurvas_transmitted.py
python3 NCurvas_deliveryRatio.py
python3 NCurvas_meanDelay.py
python3 NCurvas_meanHops.py
python3 NCurvas_meanBundlesInSDR.py
python3 NCurvas_energyEfficiency.py

: <<'END'
END
