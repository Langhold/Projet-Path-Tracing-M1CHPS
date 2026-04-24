# Monte Carlo Path Tracing

A path tracer using Monte Carlo for image rendering written in C.

## Installation

### Build

```bash
cmake -B build
make -C build
```

### Run

```bash
mkdir -p image
mkdir -p performance
mkdir -p performance/measures

export BOUNCES=number_of_bounces #(optional; = 26 by default)
./build/ppm width height amount_of_sample
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
performance/measures/runtime_by_samplings.csv
```

All runtimes are stored in seconds.

Run the experiment:

```bash
export OMP_NUM_THREADS=M
export BOUNCES=b
mpirun -n O ./build/ppm W H N
```

By default, the executable generates:

- one rendered image
- one runtime measurement
  To perform multiple measurements during a single execution, use:

```bash
mpirun -n O ./build/ppm W H N "number_of_measures"
```

where number_of_measures specifies how many measurements are taken between 1 and N samples.

To generate only the final image instead of all intermediate images, use "no_image" option.

Example:

```bash
export OMP_NUM_THREADS=8
export BOUNCES=10
mpirun -n 4 ./build/ppm 800 600 1000 100 no_image
```

####

## SIMD VERSION

Make sure to have your cpu compatible with sse instruction minimum.
To check if your cpu is compatible

```bash
lscpu | grep "sse"
```

Also, make sure to install Sleef library, for compatibility with intrinsics function like powf and absf.

```bash
sudo apt-get install libmpfr-dev libssl-dev libfftw3-dev
cmake -S . -B build/ -DCMAKE_INSTALL_PREFIX=./install
cmake --build build/ --clean-first -j `nproc`
ctest --test-dir build/ -j `nproc`
cmake --install build/
```

### Build

```bash
./script/build.sh
```

### Run

```bash

./script/run.sh
```
