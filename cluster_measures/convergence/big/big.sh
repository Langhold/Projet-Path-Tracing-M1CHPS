#!/bin/bash
#SBATCH --job-name=strong_scaling_ppn
#SBATCH --output=cluster_measures/strong_scaling/output/naive/simd_slurm-%j.out
#SBATCH --error=cluster_measures/strong_scaling/output/naive/simd_slurm-%j.err
#SBATCH --time=00:10:00
#SBATCH --nodes=1

npc=$SLURM_NTASKS
ntc=${OMP_NUM_THREADS}

echo "Running $npc process..."
echo "Running $ntc threads..."

for J in cluster.txt bdpt.txt bdpt_no_rr.txt; do

srun --cpu-bind=cores ./build/ppm "cluster_measures/convergence/big/config/${J}"

done