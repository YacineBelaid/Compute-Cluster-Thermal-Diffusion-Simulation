#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>

#include "kmeanproblem.h"
#include "kmeans.h"
#include "kmeans_parallel.h"
#include "point.h"

int main(int argc, char** argv) {
  // Generate group of points given average and variance

  if (argc != 2) {
    std::cout << "missing argument: problem file" << std::endl;
  }

  std::vector<KMeansClusterDefinition> definitions(KMeansClusterDefinition::load(argv[1]));

  if (definitions.size() == 0) {
    std::printf("failed to open problem file %s\n", argv[1]);
    return -1;
  }

  DataSet training_set;
  training_set.generate_random(definitions);

  // Points de départ pour initialiser l'algorithme
  std::vector<Point> start_points
      = start_points_random(training_set.m_points, definitions.size(), 0);

  // Trouver les groupes dans le jeu de données
  KMeans kmeans;
  kmeans.fit(training_set.m_points, start_points);

  // Comparer les groupes déterminés par l'algorithmes et ceux d'origine
  training_set.write_points({training_set.m_cluster, kmeans.get_cluster()}, "kmeans_fit.csv");
  std::ofstream cluster_out("clusters.csv");
  kmeans.write_clusters(cluster_out);

  // Classifier un autre jeu de données pour évaluer la précision
  DataSet eval_set;
  eval_set.generate_random(definitions);

  std::vector<uint8_t> eval_cluster;
  kmeans.classify(eval_set.m_points, eval_cluster);

  eval_set.write_points({eval_set.m_cluster, eval_cluster}, "kmeans_classify.csv");

  std::unordered_map<size_t, size_t> mapping;
  make_cluster_mapping(kmeans.get_centers(), definitions, mapping);

  size_t good = kmeans.compare(eval_set.m_cluster, eval_cluster, mapping);
  size_t bad = eval_set.m_cluster.size() - good;
  std::printf("Report\n");
  std::printf("Good            : %10lu\n", good);
  std::printf("Bad             : %10lu\n", bad);
  std::printf("Precision rate  : %10.1f %%\n",
              100 * static_cast<double>(good) / eval_set.m_cluster.size());

  return 0;
}
