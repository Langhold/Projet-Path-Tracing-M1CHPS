#!/bin/bash

if [[ ${1} == "R" ]];then
    cmake -B build -DCMAKE_BUILD_TYPE=Release
else
    cmake -B build
fi
cmake --build build