#!/bin/bash

export OMP_PROC_BIND=true
export OMP_PLACES=cores
export OMP_NUM_THREADS=1

mpirun -n 1 valgrind --leak-check=full ./build/ppm $1