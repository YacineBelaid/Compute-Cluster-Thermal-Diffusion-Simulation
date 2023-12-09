#!/bin/bash

for i in 1 2 4 16 32 64; do
	
	for j in 0 1 2 3 4; do
	sbatch --ntasks=${i} work.slurm ${i} ${j}
	done
done
