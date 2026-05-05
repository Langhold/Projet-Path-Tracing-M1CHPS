#!/bin/bash
#SBATCH --job-name=convergence_ppn
#SBATCH --output=cluster_measures/convergence/mickey/mickey_slurm-%j.out
#SBATCH --error=cluster_measures/convergence/mickey/mickey_slurm-%j.err
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1

npc=$SLURM_NTASKS
ntc=${OMP_NUM_THREADS}

echo "Running $npc process..."
echo "Running $ntc threads..."

for J in cluster.txt bdpt.txt bdpt_no_rr.txt; do

srun --cpu-bind=cores ./build/ppm "cluster_measures/convergence/mickey/config/${J}"

done
