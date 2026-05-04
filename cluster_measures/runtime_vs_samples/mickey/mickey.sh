#!/bin/zsh



for J in naive.txt tree.txt cluster.txt simd.txt bdpt.txt russian.txt; do

mpirun -np 10 ./build/ppm "cluster_measures/runtime_vs_samples/mickey/config/${J}"

done
