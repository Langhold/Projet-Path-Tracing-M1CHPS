#!/bin/bash
#SBATCH --job-name=strong_scaling_ppn
#SBATCH --output=cluster_measures/strong_scaling/cluster_slurm-%j_cluster.out
#SBATCH --error=cluster_measures/strong_scaling/cluster_slurm-%j.err
#SBATCH --time=00:05:00
#SBATCH --cpus-per-task=1

echo "Running with $npc CPUs..."
srun ./build/ppm "cluster_measures/strong_scaling/huge/config/cluster.txt"
