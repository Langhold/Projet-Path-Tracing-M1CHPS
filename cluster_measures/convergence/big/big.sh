#!/bin/zsh


for J in bdpt.txt cluster.txt; do

mpirun -np 10 ./build/ppm "cluster_measures/convergence/big/config/${J}"

done
