#!/bin/zsh

NPC=10

for J in {1..10}; do

if [ $(($NPC % $J)) -eq 0 ]; then
export OMP_NUM_THREADS=$(($NPC/$J))


mpirun -np $J ./build/ppm "cluster_measures/MPI_vs_OMP/huge/config/cluster.txt"

fi
done
