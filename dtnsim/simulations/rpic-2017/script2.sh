#!/bin/bash

#remove old results
rm -rf results

#create results folders
mkdir results

cd results

mkdir results_centrality
mkdir results_random

cd results_centrality
mkdir 0.1
mkdir 0.5
mkdir 1.0
cd ..

cd results_random
mkdir 0.1
mkdir 0.5
mkdir 1.0
cd ..

cd ..

# python scripts to read results databases and generate .txt results
python3 deletedContactsAnalysis_centrality_0.1.py
python3 deletedContactsAnalysis_centrality_0.5.py
python3 deletedContactsAnalysis_centrality_1.0.py

python3 deletedContactsAnalysis_random_0.1.py
python3 deletedContactsAnalysis_random_0.5.py
python3 deletedContactsAnalysis_random_1.0.py

# python scripts to read .txt result files and generate plots
python3 NCurvas_recibidos_centrality.py
python3 NCurvas_reruteados_centrality.py
python3 NCurvas_rutas_disponibles_centrality.py
python3 NCurvas_nodos_conectados_centrality.py

python3 NCurvas_recibidos_random.py
python3 NCurvas_reruteados_random.py
python3 NCurvas_rutas_disponibles_random.py
python3 NCurvas_nodos_conectados_random.py

: <<'END'
END
