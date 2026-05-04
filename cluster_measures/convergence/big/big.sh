#!/bin/zsh


for J in bdpt.txt cluster.txt bdpt_no_rr.txt; do

mpirun -np 10 ./build/ppm "cluster_measures/convergence/big/config/${J}"

done
