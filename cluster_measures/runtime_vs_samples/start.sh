#!/bin/bash

echo "Job started on $(hostname) at $(date)"

export npc=32

for J in big huge medium; do

sbatch --nodes 1 --ntasks-per-node $npc ./cluster_measures/runtime_vs_samples/${J}/${J}.sh

done
