#!/bin/bash

echo "Job started on $(hostname) at $(date)"

export npc=1

for J in 1 2 4 8 16 32 64 128; do

npc=$J

sbatch --nodes 1 --ntasks-per-node $npc ./cluster_measures/strong_scaling/run_cluster.sh

done

