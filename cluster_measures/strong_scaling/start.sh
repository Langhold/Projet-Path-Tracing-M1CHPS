#!/bin/bash
#SBATCH --job-name=strong_scaling_ppn
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err
#SBATCH --time=00:05:00
echo "Job started on $(hostname) at $(date)"

module load gcc/13.2.0

cmake -B build -DCMAKE_BUILD_TYPE=Release

cmake --build build

npc=1

for J in {1..8}; do

echo "Running with $npc CPUs..."
sbatch --cpus-per-task=$npc ./build/ppm "cluster_measures/strong_scaling/huge/config/cluster.txt"
sbatch --cpus-per-task=$npc ./build/ppm "cluster_measures/strong_scaling/huge/config/simd.txt"
sbatch --cpus-per-task=$npc ./build/ppm "cluster_measures/strong_scaling/huge/config/naive.txt"

npc=$((npc * 2))
done

