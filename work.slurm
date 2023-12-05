#!/bin/bash
#SBATCH --job-name=work
#SBATCH --output=stdout_%j.out
#SBATCH --time=02:00:00

# Nombre de processeurs à tester
processors="1 2 4 8 16 32 64"

# Niveau de raffinement parallel
rpl=" 1 2 3 4"

# Boucle pour lancer le programme avec différents nombres de processeurs et de raffinement parallel
for p in $processors; do
 mem_per_cpu=$((1024/p))M
    for rp in $rpl; do
        srun --ntasks=$p --mem-per-cpu=$mem_per_cpu ./build/heatsim -rp $rp -o 2 -m ../data/part.msh -n "Heatsim_${p}_${rp}" -p $p
    done
done
