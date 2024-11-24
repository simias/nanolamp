#!/bin/sh

python model.py

openscad model.scad &

inotifywait -m -e close_write --format '%w%f' ./ | while read f
do
    if [[ "$f" == *.py ]]
    then
        echo "[$(date)] $f"
        python model.py
    fi
done
