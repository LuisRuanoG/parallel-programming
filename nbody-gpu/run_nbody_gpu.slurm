#!/bin/bash
#SBATCH --job-name=nbody_gpu
#SBATCH --partition=GPU
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --gres=gpu:1
#SBATCH --output=nbody_gpu_%j.out
#SBATCH --error=nbody_gpu_%j.err

module load cuda/12.4

make clean
make

./nbody 1000 1 10 1 128 > random1000.out
./nbody 10000 1 10 1 128 > random10000.out
./nbody planet 200 1000 100 128 > solar.out