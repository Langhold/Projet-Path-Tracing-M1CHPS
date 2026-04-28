#!/bin/zsh


npc=1


for J in {1..10}; do


mpirun -np $npc ./build/ppm "cluster_measures/strong_scaling/huge/config/cluster.txt"

npc=$((npc ** 2))
done
