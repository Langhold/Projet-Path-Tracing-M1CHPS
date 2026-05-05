#!/bin/bash

echo "Job started on $(hostname) at $(date)"

export npc=16
export ntc=8

for J in big huge medium mickey; do
    echo "Running convergence on ${J} settings ..."
    sbatch --nodes 1 --export=ALL,OMP_NUM_THREADS=$ntc --ntasks-per-node=$npc --cpus-per-task=$ntc "./cluster_measures/convergence/${J}/${J}.sh"
done

