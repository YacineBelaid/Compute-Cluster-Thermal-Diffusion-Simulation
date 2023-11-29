# INF5171-233-tp3


## Mise en situation

Votre collègue vient vous demander de l'aide pour concevoir une pièce. Il s'agit d'un bloc dans lequel passe un fort courant, ce qui fait en sorte que la pièce chauffe. Votre collègue a tenté d'ajouter des tubes pour refroidir la pièce, mais aimerait savoir quelle est la température prévue dans la pièce. Étant donné que construire la pièce et tester différents design est coûteur, il vous demande votre aide pour la simuler.

Pour ce TP, vous aurez besoin des outils suivants:

* [GMSH](https://gmsh.info/) Générateur de maillage
* [MFEM](https://mfem.org/) Librairie de simulation à éléments finis
* [ParaView](https://www.paraview.org/) Visualisateur

La librairie MFEM est disponible sur la grappe de calcul. Si vous souhaitez l'installer sur votre ordinateurs, suivez les instructions de compilation. Vous aurez besoin de METIS et HYPRE.

## Design de la pièce

La pièce est définie dans le fichier `data/part.geo` au format GMSH. La première étape est de visualiser la pièce et générer le maillage. Exécutez la commande suivante pour démarrer GMSH:

```
cd data
gmsh part.geo
```

La vue sera celle de la pièce et de deux canaux de refroidissement cylindrique. Pour générer le maillage, sélectionner dans le panneau de gauche `Modules > Mesh > 3D`. Pour mieux visualiser les éléments, sélectionner dans le menu `Tools > Options > Mesh > Visibility > 3D Element faces` et désactiver `2D ement edges`.

Pour visualiser l'intérieur du volume, sélectionner `Tools > Clipping > Mesh`. Sélectionner `Keep whole elements`, puis définir un plan de coupe. Effectuer une capture d'écran pour votre rapport.

Pour générer le maillage de la pièce, utiliser la commande suivante, ce qui va générer le fichier `part.msh`. Le fichier MSH est nécessaire pour la simulation.

```
gmsh -3 part.geo
```

## Lancement de la simulation

Le programme principal est `heatsim`. Il s'agit d'un programme qui utilise MPI pour effectuer un calcul distribué. Il charge le maillage de la pièce, applique l'équation de diffusion pour assembler la matrice d'un système, fait la résolution à l'aide de la méthode du gradient conjugué, et sauvegarde le résultat.

La compilation se fait avec CMake sur la grappe de calcul de la manière suivante.

```
module load cmake
mkdir build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/project/def-sponsor00/mfem/
```

Voici les options du programme:

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

* On utiliser l'option `--mesh` pour utiliser une autre géométrie.
* L'option `--ordre` spécifie l'ordre des polynômes d'interpolation. Les valeurs sont généralement 1, 2, ou 3. Un ordre plus élevé est difficile à résoudre et pourrait ne pas converger.
* L'option `--refine-parallel` permet de diviser les éléments pour augmenter la précision de la simulation. En règle général, chaque raffinement multiplie le nombre d'éléments par un facteur 7.
* L'option `--name` permet de changer le nom du répertoire de sortie. Si on lance plusieurs simulations en même temps, chaque simulation doit avoir son propre répertoire, sinon les résultats seront écrasés.

La simulation est sauvegardée dans le répertoire `ParaView/Heatsim`. Vous pouvez compresser et copier ce répertoire sur votre ordinateur. Ouvez le fichier `Heatsim.pvd` pour visualiser le résultat. Techniquement, visualiser le résultat avec un faible raffinement, sinon le fichier sera très volumineux.

Le lancement de la simulation se fait avec `mpirun` sur votre propre ordinateur, ou par l'entremise de SLURM sur la grappe de calcul. Voici des exemples:

```
# Lancement avec deux processeurs
mpirun -np 2 ./heatsim

# Lancement avec srun sur 64 processeurs
# Effectue 4 raffinements
# Calcul avec des polynômes de degré 2
srun --ntasks=64 --mem-per-cpu=8000M ./heatsim -rp 4 -o 2 -m ../data/part.msh
```

Attention: vous allez faire planter votre machine si vous tentez d'exécuter la commande destinée à la grappe (4 étapes de raffinement), car le maillage risque de ne pas entrer en mémoire d'un ordinateur de bureau.

Attention: le lancement de grandes simulations va produire d'énormes fichiers. Sur la grappe, lancer la simulation dans le répertoire `$HOME/scratch`, qui contient beaucoup d'espace libre.

## Étude de la mise à l'échelle

Votre tâche consiste à mesurer le temps d'exécution des 4 étapes de la simulation.

Instrumentez le programme `heatsim.cpp` pour mesurer le temps de chaque étape et l'enregistrer dans un fichier.

* Chargement et raffinement du maillage
* Assemblage
* Résolution
* Sauvegarde des résultats

Finalement, un métrique importante à calculer est la suivante: degrés de libertés (Degree of Freedom) calculé par unité de temps (DoF/s). Un degré de liberté correspond à un point de température de la simulation. Ceci permet de comparer des calculs en terme de débit en tenant en compte la taille du problème à résoudre.

Calculer la proportion du temps passé pour chaque étape et le temps écoulé total pour 1, 2, 4, 8, 16, 32, et 64 processeurs.

Faire varier le paramètre raffinement de 0 à 4. Attention: une très grande simulation va échouer ou prendre beaucoup de temps à 1 processeur. Si cela survient, il faut augmenter la limite de temps par défaut de 1h. Si la simulation échoue, ne pas en tenir compte dans l'analyse.

Le lancement de tous les calculs doit se faire en exécutant un seul script. Les tâches doivent êtres ajoutées dans la file d'attente et réserver uniquement le nombre de processeurs requis pour l'expérience en cours.

Vous devez remettre un rapport d'au maximum 3 pages avec vos résultats, incluant une capture d'écran de votre maillage (GMSH) et un exemple de résultat avec une vue en coupe (ParaView), les graphiques d'accélération en fonction du nombre de processeurs et la proportion de chaque étape en fonction du nombre de processeurs et une courte analyse sur la mise à l'échelle de la simulation.

Remettez votre rapport ainsi qu'une archive ZIP, qui comprend votre code et vos scripts SLURM pour lancer votre simulation.

Barèmes:
* Instrumentation du programme heatsim: 25
* Scripts de lancement SLURM: 25
* Rapport (captures, graphiques, analyse): 50
* Total: 100

Bon travail!
