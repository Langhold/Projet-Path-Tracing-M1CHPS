#!/bin/zsh


echo "Job started on $(hostname) at $(date)"

export npc=1
export ntc=1

#Processus scaling
echo "MPI scaling ..."
for P in 1 2 3 4 8 10 16 20 24 32; do

    npc=$P

    sbatch --nodes 1 --export=ALL,OMP_NUM_THREADS=$ntc --ntasks-per-node=$npc --cpus-per-task=$ntc ./cluster_measures/MPI_vs_OMP/huge/huge.sh "cluster_measures/MPI_vs_OMP/huge/config/cluster_mpi.txt"

done

npc=1

#Thread scaling
echo "OMP scaling ..."
for T in 1 2 3 4 8 10 16 20 24 32; do

    ntc=$T

    sbatch --nodes 1 --export=ALL,OMP_NUM_THREADS=$ntc --ntasks-per-node=$npc --cpus-per-task=$ntc ./cluster_measures/MPI_vs_OMP/huge/huge.sh "cluster_measures/MPI_vs_OMP/huge/config/cluster_omp.txt"

done