#!/bin/bash
#SBATCH --job-name=work
#SBATCH --output=sortie_slurm_Heatsim/sortie_Heatsim_%j.out
#SBATCH --time=01:00:00
#SBATCH --account=def-sponsor00
#SBATCH --mem=4096M

srun --ntasks=${1} ./build/heatsim -rp ${2} -o 2 -m ./data/part.msh -n Rapport/Heatsim_${1}_${2} -p ${1} 

