#!/bin/sh
#SBATCH --job-name=my_first_ppn_job
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err
#SBATCH --time=00:05:00
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --nodes=1

echo "Job started on $(hostname) at $(date)"

module load gcc/13.2.0
module load openmpi/4.1.6

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

srun ./build/ppm config.txt

echo "Job completed at $(date)" 
