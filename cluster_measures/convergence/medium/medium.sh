#!/bin/zsh


for J in cluster.txt bdpt.txt bdpt_no_rr.txt; do

mpirun -np 10 ./build/ppm "cluster_measures/convergence/medium/config/${J}"

done
