#include <functional>
#include <memory>
#include <unordered_map>

#include "kmeanproblem.h"
#include "kmeans.h"
#include "kmeans_parallel.h"
#include "modules.h"

int check_clusters(const std::vector<Point>& centers,
                   const std::vector<KMeansClusterDefinition>& definitions) {
  std::unordered_map<size_t, size_t> matches;
  int res = make_cluster_mapping(centers, definitions, matches);
  if (res < 0) {
    return -1;
  }

  for (const auto& item : matches) {
    const Point& ref_center = definitions[item.first].center;
    const Point& act_center = centers[item.second];
    double dist = act_center.dist(ref_center);
    if (dist < 0.1) {
      std::cout << "PASS ";
    } else {
      std::cout << "FAIL ";
      res = -1;
    }

    std::cout << item.first << " " << item.second << " " << ref_center << " " << act_center << " "
              << dist << "\n";
  }

  return res;
}

int main() {
  std::vector<KMeansClusterDefinition> definitions = {
      KMeansClusterDefinition(1E6, Point(5, 5), Point(0.5, 0.5)),
      KMeansClusterDefinition(1E6, Point(10, 10), Point(0.5, 0.5)),
      KMeansClusterDefinition(1E6, Point(0, 0), Point(2, 2)),
  };

  DataSet problem;
  problem.generate_deterministic(definitions);
  std::vector<Point> start_points
      = start_points_deterministic(problem.m_points, definitions.size(), 0);

  bool fail = false;

  for (const struct test_klass& item : algo) {
    std::cout << "TEST " << item.klass << "\n";
    std::shared_ptr<KMeans> kmeans = item.factory();
    kmeans->fit(problem.m_points, start_points);

    std::string points_output = "test_" + item.klass + "_points.csv";
    std::string cluster_output = "test_" + item.klass + "_cluster.csv";

    problem.write_points({kmeans->get_cluster(), problem.m_cluster}, points_output);

    std::ofstream ofs(cluster_output);
    kmeans->write_clusters(ofs);
    if (check_clusters(kmeans->get_centers(), definitions) < 0) {
      fail = true;
    }
  }

  if (fail > 0) {
    return -1;
  }
  return 0;
}
