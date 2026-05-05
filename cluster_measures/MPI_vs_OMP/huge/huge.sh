#!/bin/bash
#SBATCH --job-name=MPI_vs_OMP_ppn
#SBATCH --output=cluster_measures/MPI_vs_OMP/output/cluster/cluster_slurm-%j.out
#SBATCH --error=cluster_measures/MPI_vs_OMP/output/cluster/cluster_slurm-%j.err
#SBATCH --time=00:10:00
#SBATCH --nodes=1

npc=$SLURM_NTASKS
ntc=${OMP_NUM_THREADS}
echo "MPI processes: $npc"
echo "OMP threads per process: $ntc"

echo "Running with $npc Process and $ntc CPUs..."

srun --cpu-bind=cores ./build/ppm $1


