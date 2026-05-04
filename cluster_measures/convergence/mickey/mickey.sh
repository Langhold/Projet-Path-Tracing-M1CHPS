#!/bin/zsh



for J in cluster.txt bdpt.txt bdpt_no_rr; do

mpirun -np 10 ./build/ppm "cluster_measures/convergence/mickey/config/${J}"

done
