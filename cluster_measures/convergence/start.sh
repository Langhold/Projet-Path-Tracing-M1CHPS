#!/bin/zsh


for J in big huge medium mickey; do
    "./cluster_measures/convergence/${J}/${J}.sh"
done
