#!/bin/bash
#SBATCH --job-name=prueba_mpi
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=4
#SBATCH --time=00:05:00
#SBATCH --output=salida_mpi.out
#SBATCH --error=error_mpi.err

mpirun ./dij_mpi.c
