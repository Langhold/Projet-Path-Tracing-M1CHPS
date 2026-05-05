#!/bin/bash

echo "Job started on $(hostname) at $(date)"

export npc=1

for J in {1 2 3 4 8 10 16 20 24 32}; do

npc=$J

sbatch --nodes 1 --ntasks-per-node $npc ./cluster_measures/strong_scaling/cluster_simd.sh
sbatch --nodes 1 --ntasks-per-node $npc ./cluster_measures/strong_scaling/run_simd.sh
sbatch --nodes 1 --ntasks-per-node $npc ./cluster_measures/strong_scaling/run_naive.sh

done

