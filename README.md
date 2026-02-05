# Sequential Merge Sort Benchmark

## Overview

This project implements a sequential merge sort in C++ and assesses performance on the Centaurus (UNCC) educational cluster. 
The goal is to make execution time vs input size measurements and investigate the expected O(n log n) behavior.

## Files

- `src/mergesort.cpp` – Sequential merge sort implementation
- `Makefile` – Builds the project
- `run_mergesort.slurm` – SLURM script used to run benchmarks on Centaurus
- `mergesort_13232.out` – Benchmark output (array size vs time)
- `plot.py` – Python script used to generate the plot
- `plot.png` – Log–log plot of benchmark results

## Compilation

```bash
make
```

## Benchmarking

Benchmarks were executed on the Centaurus cluster using:

```bash
sbatch run_mergesort.slurm
```

The program outputs:
<array_size> <time_in_seconds>
