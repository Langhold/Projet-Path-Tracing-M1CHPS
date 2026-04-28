#!/bin/bash

# Create output directory
mkdir -p performance/measures
output_file="performance/measures/run_mpi_samples.json"
rm -f "$output_file"
echo "[" > "$output_file"

first=1

for ((s=50; s<=2000; s+=2000))
do
    method="samples_800x600"
    echo "Running sample size: $s ..."

    # Run MPI program and capture perf metrics
    perf_output=$(BOUNCES=2 perf stat -x, -e instructions,cycles,cache-misses,cache-references \
                  mpirun -np 8 ./ppm 800 600 "$s" 1>/dev/null 2>&1)

    # Parse metrics, remove commas
    instructions=$(echo "$perf_output" | awk -F, '/instructions/ {gsub(/,/,""); print $1}')
    cycles=$(echo "$perf_output" | awk -F, '/cycles/ {gsub(/,/,""); print $1}')
    echo "NUEIOABGEOIAUBGEIOAHBNGEIOGHNEAIOGHBNEAIOUKJBGNEIOAUGBEIOUAGBHN$cycles"
    cache_misses=$(echo "$perf_output" | awk -F, '/cache-misses/ {gsub(/,/,""); print $1}')
    cache_references=$(echo "$perf_output" | awk -F, '/cache-references/ {gsub(/,/,""); print $1}')

    # Compute IPC and cache miss rate
    IPC=$(echo "scale=6; $instructions / $cycles" | bc -l)
    cache_miss_rate=$(echo "scale=6; $cache_misses / $cache_references * 100" | bc -l)

    # JSON object
    json="{\"method\":\"$method\", \"samples\":$s, \"IPC\":$IPC, \"cache_miss_rate_percent\":$cache_miss_rate}"

    # Append JSON
    if [ $first -eq 1 ]; then
        first=0
    else
        echo "," >> "$output_file"
    fi
    echo "$json" >> "$output_file"

    echo "s=$s Done"
done

echo "]" >> "$output_file"

# Beep when finished
powershell.exe -Command "[console]::beep(250,300)"
