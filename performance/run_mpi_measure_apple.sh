#!/bin/zsh

WIDTH=800
HEIGHT=600
SAMPLES=500

export OMP_NUM_THREADS=5
export BOUNCES=26

for J in true close spread; do

export OMP_PROC_BIND=$J

for I in {1..5}; do
mpirun -n 2 ./build/ppm $WIDTH $HEIGHT $SAMPLES
done

echo >> performance/measures/runtime_by_samplings.csv

sleep 10

done
