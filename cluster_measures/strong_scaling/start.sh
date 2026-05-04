#!/bin/bash

echo "Job started on $(hostname) at $(date)"

npc=1

for J in {1..8}; do

echo "Running with $npc CPUs..."
sbatch --cpus-per-task=$npc ./build/ppm "cluster_measures/strong_scaling/run_cluster.sh"
#sbatch --cpus-per-task=$npc ./build/ppm "cluster_measures/strong_scaling/huge/config/simd.txt"
#sbatch --cpus-per-task=$npc ./build/ppm "cluster_measures/strong_scaling/huge/config/naive.txt"

npc=$((npc * 2))
done

