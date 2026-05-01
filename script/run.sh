#!/bin/bash

NUM_PROC=$2
export OMP_PROC_BIND=true
export OMP_PLACES=cores
export OMP_NUM_THREADS=$3

mpirun -n ${NUM_PROC} --map-by core --bind-to core ./build/ppm $1 