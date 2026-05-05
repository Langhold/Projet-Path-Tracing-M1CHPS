#!/bin/bash
#SBATCH --nodes=1

echo "Job started on $(hostname) at $(date)"

export npc=1

for J in {1..8}; do


sbatch --wait --tasks-per-node=$npc ./cluster_measures/strong_scaling/cluster_simd.sh
sbatch --wait --tasks-per-node=$npc ./cluster_measures/strong_scaling/run_simd.sh
sbatch --wait --tasks-per-node=$npc ./cluster_measures/strong_scaling/run_naive.sh

npc=$((npc * 2))
done

