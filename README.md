# Monte Carlo Path Tracing
A path tracer using Monte Carlo for image rendering written in C.



## Installation

### Build 
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
make -C build
```

### Run 
```bash
mkdir -p image
mkdir -p performance
mkdir -p performance/measures

./build/ppm <CONFIG>.txt 
```



## Experiments
We focus our study on runtime optimization.
Two experimental protocols are used:
   - runtime measurement
   - convergence speed measurement

Some optimizations that reduce runtime can negatively impact convergence speed.

### Protocol 1: The runtime

#### Objective
The goal of this experiment is to measure the performances of the Path Tracer by focusing on the runtime. It will be measured only on the rendering phase, not the scene initialization and the time computation neather. 

#### Requierment
   - Compiler: gcc / clang
   - CMake
   - mpich 4.3.0
   - Open MP

#### Setup
Build the project in Release mode:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
make -C build
```
   - Image resolution: WxH
   - Number of samples: N
   - Number of bounces: B
   - Number of processes MPI: M
   - Number of threads Open MP: O
   - Benchmark scene: either homemade or predefined

#### Time Measurement
Runtime measurement is implemented directly in the path tracer using:
```c
clock_gettime();
```
The function is called twice:
```c
struct timespec start, end;

clock_gettime(CLOCK_MONOTONIC, &start);

/* rendering computation */

clock_gettime(CLOCK_MONOTONIC, &end);
```

Runtime measurements are exported to:
```code
performance/measures/measures.csv
```
All runtimes are stored in seconds.

Run the experiment:
```bash
export OMP_NUM_THREADS=O
mpirun -n M ./build/ppm config.txt
```
With config.txt:
```bash
width = W
height = H
samples = N
bounces = B
output_filename = performance/measures/measures.csv
benchmark = medium
```

By default, the executable generates:
   - one rendered image
   - one runtime measurement
To perform multiple measurements during a single execution, use:
 ```bash
 print rate = <NUMBER OF IMAGE AND RUNTIME MEASURES RATE>
 ```
where print rate specifies how many measurements are taken between 1 and <print rate> samples.

To generate only the final image instead of all intermediate images, use "only last image" option.

Example:
```bash
#Only the final image will be print
only last image = 1
#You can create the same images multiple times using n_measures to refine your measurements
n_measures = 5
```
#### 



### Protocol 2: The convergence speed and the mean error



