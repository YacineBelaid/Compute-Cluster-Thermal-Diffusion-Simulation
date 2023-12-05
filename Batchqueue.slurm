#!/bin/bash
#SBATCH --job-name=work
#SBATCH --output=stdout_%j.out
#SBATCH --time=02:00:00

# Nombre de processeurs à tester
processors="1 2 4 8 16 32 64"

# Niveau de raffinement parallel
rpl="1 2 3 4"

# Chemin vers l'exécutable heatsim
heatsim_exec="./build/heatsim"

# Boucle pour lancer le programme avec différents nombres de processeurs et de raffinement parallel
for p in $processors; do
    mem_per_cpu=$((4096 / p))M
    for rp in $rpl; do
        job_name="Heatsim_${p}_${rp}"
        script_file="${job_name}_script.slurm"

        echo "#!/bin/bash" > $script_file
        echo "#SBATCH --job-name=$job_name" >> $script_file
        echo "#SBATCH --output=$job_name.out" >> $script_file
        echo "#SBATCH --time=02:00:00" >> $script_file
        echo "" >> $script_file
        echo "srun --ntasks=$p --mem-per-cpu=$mem_per_cpu $heatsim_exec -rp $rp -o 2 -m ../data/part.msh -n $job_name -p $p" >> $script_file

        sbatch $script_file
        rm $script_file
    done
done
