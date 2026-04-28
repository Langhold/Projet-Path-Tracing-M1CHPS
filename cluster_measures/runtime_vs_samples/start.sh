#!/bin/zsh


for J in big huge medium mickey; do
    "./cluster_measures/runtime_vs_samples/${J}/${J}.sh"
done
