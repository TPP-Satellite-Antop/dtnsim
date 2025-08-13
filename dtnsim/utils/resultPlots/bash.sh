#!/bin/bash

directory="$1"
if [ -z "$directory" ]; then
  echo "Usage: $0 <path_to_dtnsim_results_directory>"
  exit 1
fi

rm -rf "$directory"/dtnsim.*.pdf

python3 mainVec.py "$directory" "$directory"
python3 mainSca.py "$directory" "$directory"
