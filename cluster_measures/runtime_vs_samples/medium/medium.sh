#!/bin/bash
#SBATCH --job-name=runt_vs_smpl
#SBATCH --output=cluster_measures/runtime_vs_samples/medium/cluster_slurm-%j_cluster.out
#SBATCH --error=cluster_measures/runtime_vs_samples/medium/cluster_slurm-%j.err
#SBATCH --time=02:00:00
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1


for J in naive.txt tree.txt cluster.txt simd.txt bdpt.txt russian.txt bdpt_no_rr.txt; do

echo "Running ${J}"
srun ./build/ppm "cluster_measures/runtime_vs_samples/medium/config/${J}"

done
