#!/bin/bash
#SBATCH --nodes=1

echo "Job started on $(hostname) at $(date)"

export npc=1

for J in {1..8}; do

echo "Running with $npc CPUs..."

sbatch --cpus-per-task=$npc ./cluster_measures/strong_scaling/cluster_simd.sh
sbatch --cpus-per-task=$npc ./cluster_measures/strong_scaling/run_simd.sh
sbatch --cpus-per-task=$npc ./cluster_measures/strong_scaling/run_naive.sh

npc=$((npc * 2))
done

