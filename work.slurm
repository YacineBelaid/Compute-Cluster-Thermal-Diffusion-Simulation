#!/bin/bash
#SBATCH --job-name=work
#SBATCH --output=stdout_%j.out
#SBATCH --time=02:00:00
#SBATCH --account=def-sponsor00


srun --ntasks=${1} --mem-per-cpu=(4096/${1})M ./build/heatsim -rp ${2} -o 2 -m ./data/part.msh -n "Heatsim_${1}_${2}" -p ${1}

