#!/bin/bash
#SBATCH --job-name=strong_scaling_ppn
#SBATCH --output=cluster_measures/strong_scaling/output/naive/simd_slurm-%j.out
#SBATCH --error=cluster_measures/strong_scaling/output/naive/simd_slurm-%j.err
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1

echo "Running with $npc CPUs..."
srun ./build/ppm "cluster_measures/strong_scaling/huge/config/simd.txt"
