#!/bin/zsh


for J in cluster.txt bdpt.txt; do

mpirun -np 10 ./build/ppm "cluster_measures/convergence/huge/config/${J}"

done
