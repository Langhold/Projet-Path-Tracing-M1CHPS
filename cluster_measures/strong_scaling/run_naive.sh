#!/bin/bash
#SBATCH --job-name=strong_scaling_ppn
#SBATCH --output=cluster_measures/strong_scaling/output/naive/naive_slurm-%j.out
#SBATCH --error=cluster_measures/strong_scaling/output/naive/naive_slurm-%j.err
#SBATCH --time=00:10:00
#SBATCH --cpus-per-nodes=1

echo "Running with $npc CPUs..."
srun ./build/ppm "cluster_measures/strong_scaling/huge/config/naive.txt"
