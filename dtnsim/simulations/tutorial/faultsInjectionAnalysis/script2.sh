#!/bin/bash

#remove old results
rm -rf results_Graphics

#create results folders
mkdir results_Graphics

cd results_Graphics

mkdir centrality
mkdir random

cd centrality
mkdir 0.5
cd ..

cd random
mkdir 0.5
cd ..

cd ..

# python scripts to read results databases and generate .txt results
python3 deletedContactsAnalysis_centrality_0.5.py

python3 deletedContactsAnalysis_random_0.5.py

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
