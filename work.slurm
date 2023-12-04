#!/bin/bash
#SBATCH --job-name=work
#SBATCH --output=stdout_%j.out
#SBATCH --time=02:00:00

# Nombre de processeurs à tester
processors="1 2 4 8 16 32 64"

# Niveau de raffinement parallel
rpr=" 1 2 3 4"

# Boucle pour lancer le programme avec différents nombres de processeurs et de raffinement parallel
for p in $processors; do
    for rpara in $rpr; do
        srun --ntasks=$p --mem-per-cpu=125M ./build/heatsim -rp=$rpara -o 2 -m ../data/part.msh -n Heatsim_${p}_${rpara} $p $rpara
    done
done
