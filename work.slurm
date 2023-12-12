#!/bin/bash
#SBATCH --job-name=work
#SBATCH --output=sortie_slurm_Heatsim/sortie_Heatsim_%j.out
#SBATCH --time=01:00:00
#SBATCH --account=def-sponsor00
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=4

# Set the number of tasks
ntasks=${1}

# Calculate memory per CPU dynamically
mem_per_cpu=$((1024*${2}/${1}))

# Build the srun command with proper memory specification
srun --ntasks=${ntasks} --mem-per-cpu=${mem_per_cpu}M ./build/heatsim -rp ${2} -o 2 -m ./data/part.msh -n "Rapport/Heatsim_${1}_${2}" -p ${ntasks} > "sortie_slurm_Heatsim/sortie_Heatsim_${1}_${2}.out" 

