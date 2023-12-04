#!/bin/bash
#SBATCH --job-name=work
#SBATCH --output=stdout_%j.out
#SBATCH --time=02:00:00

# Nombre de processeurs à tester
processors="1 2 4 8 16 32 64"

# Commande à exécuter
command="./heatsim"

# Options communes
common_options="-o 2 -m ../data/part.msh"

# Boucle pour lancer le programme avec différents nombres de processeurs
for p in $processors; do
    srun --ntasks=$p --mem-per-cpu=8000M $command $common_options $p
done
