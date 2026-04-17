# Parallel Merge Sort with Tasking

## Compilation

Run:

make

## Execution

Run:

./mergesort_seq <n> <nbthreads> <threshold>

Parameters:
- `n`: number of elements in the array
- `nbthreads`: number of threads
- `threshold`: minimum subarray size required to create parallel tasks

## Examples

Sequential-style run with one thread:

./mergesort_seq 1000000 1 1000

Parallel runs:

./mergesort_seq 1000000 2 1000
./mergesort_seq 1000000 4 1000
./mergesort_seq 1000000 8 1000

## Implementation Notes

This implementation uses the provided tasking abstraction in `omp_tasking.hpp`:
- `tasking::doinparallel`
- `tasking::taskstart`
- `tasking::taskwait`

For subarrays of size less than or equal to the threshold, the program switches to sequential merge sort to avoid excessive task overhead.

## Performance Summary

Threshold used: 1000

Array size: 10000
- 1 thread: 0.000641411
- 8 threads: 0.000936355

Array size: 100000
- 1 thread: 0.0077698
- 8 threads: 0.00293404

Array size: 1000000
- 1 thread: 0.0893196
- 2 threads: 0.0483943
- 4 threads: 0.0385669
- 8 threads: 0.0276161

## Short Comparison of Sequential vs Parallel Performance

For very small arrays, the parallel version is slower because the overhead of creating and synchronizing tasks is larger than the useful work.

For larger arrays, the parallel version becomes faster and shows clear speedup as the number of threads increases.

In these tests, the best performance was achieved with 8 threads on the 1000000-element input.