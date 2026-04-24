cat > README.md <<'EOF'
# Parallel N-Body Simulation

## Compilation

Compile the program with:

make

## Execution

Run the program with:

./nbody <input> <dt> <nbstep> <printevery> <nbthreads>

Where:

- `input` can be:
  - a number for random initialization
  - `planet` for the solar system model
  - a filename to load a saved simulation state
- `dt` is the time step
- `nbstep` is the number of simulation steps
- `printevery` is how often to print the state
- `nbthreads` is the number of threads to use

## Examples

Solar system:

./nbody planet 200 500000 10000 1 > solar_seq.out
./nbody planet 200 500000 10000 8 > solar_par.out

Random 100 particles:

./nbody 100 1 10000 100 1 > random100_seq.out
./nbody 100 1 10000 100 8 > random100_par.out

Random 1000 particles:

./nbody 1000 1 10000 100 1 > random1000_seq.out
./nbody 1000 1 10000 100 8 > random1000_par.out

## Parallelization

This program uses the provided `OmpLoop` abstraction for all parallelism.

The following parts were parallelized:

- resetting forces
- computing forces
- updating velocities and positions

The outer loop over particles is parallelized during force computation so that each thread computes the total force for its assigned particle, avoiding race conditions.

## Timing Summary

Results collected on Centaurus:

### Solar system (planet, dt=200, nbstep=500000, printevery=10000)
- 1 thread: 1.493s
- 8 threads: 12.078s

### Random 100 particles (dt=1, nbstep=10000, printevery=100)
- 1 thread: 0.762s
- 8 threads: 1.189s

### Random 1000 particles (dt=1, nbstep=10000, printevery=100)
- 1 thread: 69.936s
- 8 threads: 40.161s

## Notes

For small workloads, the parallel version can be slower because thread scheduling overhead is larger than the amount of useful work.

For larger workloads such as 1000 particles, the parallel version is faster and shows a clear speedup.
EOF

## Comparison of Sequential vs Parallel Performance

The parallel implementation does not always outperform the sequential version.

For small workloads (e.g., 10 or 100 particles), the parallel version is slower due to thread creation overhead and scheduling costs.

For larger workloads (e.g., 1000 particles), the parallel version shows clear performance improvements.

In our experiments:
- Small inputs: parallel is slower
- Large inputs: parallel is faster

The achieved speedup for 1000 particles is approximately:

Speedup ≈ 69.936 / 40.161 ≈ 1.7x

This is lower than the ideal speedup due to:
- memory contention
- cache effects
- scheduling overhead
- shared CPU resources on Centaurus