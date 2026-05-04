#!/bin/bash
#SBATCH --job-name=my_first_ppn_job
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err
#SBATCH --time=00:05:00
echo "Job started on $(hostname) at $(date)"

module load gcc/13.2.0

cmake -B build -DCMAKE_BUILD_TYPE=Release

cmake --build build

sbatch --nodes 1 --ntasks-per-node 1 --cpus-per-task 1 ./build/ppm config.txt

 
