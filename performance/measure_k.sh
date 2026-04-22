#!/bin/zsh

WIDTH=800
HEIGHT=600
SAMPLES=2000

export BOUNCES=26
export OMP_PROC_BIND=false
export OMP_NUM_THREADS=1

for J in run_cluster_2000_huge run_bhv_2000_huge run_standard_2000_huge; do

make -C build

PT_MEASURES_PATH=$J mpirun -n 10 ./build/ppm $WIDTH $HEIGHT $SAMPLES 20 no_image measures

done
