# 3D Parrallel Refinment on Compute CLusters

Using MPI and C++ 

## Heat simulator

Heat simulation for part production 

Here the tools used for the simulation :

* [GMSH](https://gmsh.info/) Mesh generator
* [MFEM](https://mfem.org/) Simulation library with limited elements
* [ParaView](https://www.paraview.org/) Viualiser

 MFEM library is available on the Compute Cluster (Calcul Quebec). If you wish to install it on your computer, follow those compiling instructions. You will need METIS and HYPRE.

## part Design

A part basemodel is defined in the file `data/part.geo` with GMSH format.To visualise it,execute this command (it will launch GMSH):

```
cd data
gmsh part.geo
```

You will visualise the part and two cylindrical cooling channels. To generate the mesh, select dans left tab `Modules > Mesh > 3D`. To get a better view of the elements, select  `Tools > Options > Mesh > Visibility > 3D Element faces` and deactivate `2D ement edges`.

To visualise inside the model, select `Tools > Clipping > Mesh`. Select `Keep whole elements`, then define a cutting plane. 

To generate a mesh of the part, use the following command, (it will generate the file) `part.msh`. MSH file is necessary for the simulation.

```
gmsh -3 part.geo
```

## Start of Simulation

The main program is `heatsim`. It's a program using  MPI to execute a distributed calculation. It loads the mesh of the part, applies the diffusion equation to assemble the matrix of a system, solves using the conjugate gradient method, and saves the result.

Compilation is done with CMake on the compute cluster in the following way.

```
module load cmake
mkdir build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/project/def-sponsor00/mfem/
```

Here are the program options:


```
$ ./heatsim -h

Usage: ./heatsim [options] ...
Options:
   -h, --help
	Print this help message and exit.
   -m <string>, --mesh <string>, current value: ../data/square-disc.mesh
	Mesh file to use.
   -o <int>, --order <int>, current value: 1
	Finite element order (polynomial degree) or -1 for isoparametric space.
   -rp <int>, --refine-parallel <int>, current value: 1
	Number of times to refine the mesh uniformly in parallel.
   -n <string>, --name <string>, current value: Heatsim
	Output data collection name
```
  
*  Use the '--mesh' option to use a different geometry. 
*  * The '--order' option specifies the order of the interpolation polynomials. Values are usually 1, 2, or 3. A higher order is difficult to resolve and might not converge. 
*  * The '--refine-parallel' option allows you to split the elements to increase the accuracy of the simulation. Typically, each refinement multiplies the number of elements by a factor of 7. * The '--name' option allows you to change the name of the output directory. If you run several simulations at the same time, each simulation must have its own directory, otherwise the results will be overwritten.

The simulation is saved in the 'ParaView/Heatsim' directory. You can compress and copy this directory to your computer. Open the 'Heatsim.pvd' file to view the result. Technically, view the result with low refinement, otherwise the file will be very large.
Le lancement de la simulation se fait avec `mpirun` sur votre propre ordinateur, ou par l'entremise de SLURM sur la grappe de calcul. Voici des exemples:

```
# Launch with two processors
mpirun -np 2 ./heatsim

#  Launch with 64 processors using srun
#  4 parrallel refinements
# Calculate with 2nd degree polynomial
srun --ntasks=64 --mem-per-cpu=8000M ./heatsim -rp 4 -o 2 -m ../data/part.msh
```

Warning: You will crash your machine if you try to run the command for the cluster (4 steps of refinement) because the mesh may not enter the memory of a desktop computer.

For Calcul Quebec :
Warning: running large simulations will produce huge files. On the cluster, run the simulation in the '$HOME/scratch' directory, which contains a lot of free space.

## Scaling Study

Now to study the scaling ability of the program, our task consists of measuring the execution time of the 4 stages of the simulation (creating a benchmark).

In`heatsim.cpp` we are measuring the time for each step and we save it in a file.

*  Mesh loading and refinement 
*  Assembly 
*  Resolution 
*  Saving results

Finally, an important metric to calculate is: Degree of Freedom calculated per unit of time (DoF/s). A degree of freedom corresponds to a temperature point in the simulation. This makes it possible to compare calculations in terms of throughput, taking into account the size of the problem to be solved.

We calculated the proportion of time spent for each step and the total elapsed time for 1, 2, 4, 8, 16, 32, and 64 processors.

By varying the refinement parameter from 0 to 4 Warning: a very large simulation will fail or take a long time at 1 processor. If this happens, the default time limit should be increased by 1 hour. If the simulation fails, disregard it in the analysis.

All calculations are started by running a single script. Tasks must be added to the queue and reserve only the number of processors required for the current experiment.

You will find in the project a report with all the results, including a screenshot of the mesh (GMSH) and an example result with a section view (ParaView), acceleration graphs as a function of the number of processors and the proportion of each step as a function of the number of processors, and a short analysis on the scaling of the simulation.

Feel free to edit the SLURM scripts to launch your own simulation on your own compute cluster

## How to execute SLURM 

Simply use/edit the shell script provided  'Boucle.sh': 
```
## launch from 1 - 64 processors for 1-4 parrallel refinment (you can make it start from 0 if you wish)

./boucle.sh
```
I provided two SLURM script. Work.slurm is a base script allowing you to launch a simple simulation with the possibility of setting the memory per cpu dynamically
The second one work2.slurm is a less flexible but more resilient script that will likely execute for all level parrallelism but it might be time consuming depending on your cluster hardware.

Author : Yacine Belaid
