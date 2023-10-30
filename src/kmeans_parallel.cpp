#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>

#include "kmeans_parallel.h"

#include <cassert>

KMeansParallel::KMeansParallel() : KMeans() {}

void KMeansParallel::fit(const std::vector<Point>& points, const std::vector<Point>& start_points) {
  m_cluster.resize(points.size());
  std::fill(m_cluster.begin(), m_cluster.end(), 0);
  std::vector<double> distances(points.size(), 0.0);
  size_t k = start_points.size();

  /*
   * Copier les points de départ, car il seront modifiés à chaque itération.
   */
  m_cluster_centers = start_points;

  /*
   * Boucle de rebalancement. Répéter tant qu'il y a des points qui changent de clusters.
   */
  size_t iter = 0;
  while (true) {
    std::cout << "iter " << iter++ << std::endl;
    /*
     * Pour chaque point, on trouve le centre le plus proche. On compte le nombre de points qui
     * changent de cluster.
     */
    size_t changes = 0;
//parallel_reduce
    tbb::task_arena limited_arena(std::thread::hardware_concurrency());
    limited_arena.execute([&]{
        tbb::parallel_for(tbb::blocked_range<size_t>(0, points.size()), [&](tbb::blocked_range<size_t>& range) {
          for (size_t point_id = range.begin(); point_id < range.end(); point_id++) {
            double min_dist = std::numeric_limits<double>::max();
            uint8_t cluster_id = 0;
            const Point& point = points.at(point_id);
            for (size_t i = 0; i < k; i++) {
              const Point& centroid = m_cluster_centers[i];
              double dist = point.dist(centroid);
              if (dist < min_dist) {
                min_dist = dist;
                cluster_id = i;
              }
            }
            if (m_cluster[point_id] != cluster_id) {
              m_cluster[point_id] = cluster_id;
              changes++;
            }
            distances[point_id] = min_dist;
          }
        });
    });

    /*
     * Mise à jour du centre de chaque cluster basé sur les points qui le compose. Le nouveau
     * centre est la moyenne des coordonnées des points.
     */
    std::vector<int> cluster_size(k, 0);
    std::vector<Point> cluster_average(k);

    for (size_t point_id = 0; point_id < points.size(); point_id++) {
      uint8_t point_group = m_cluster[point_id];
      const Point& point = points.at(point_id);
      cluster_size[point_group]++;
      cluster_average[point_group].x += point.x;
      cluster_average[point_group].y += point.y;
    }

    for (size_t i = 0; i < k; i++) {
      if (cluster_size[i] > 0) {
        m_cluster_centers[i].x = cluster_average[i].x / cluster_size[i];
        m_cluster_centers[i].y = cluster_average[i].y / cluster_size[i];
      } else {
        // Le cluster groupe est vide, pour éviter un NaN il faut éviter de faire la moyenne.
        // Normalement, il faudrait assigner un autre point au cluster.
      }
    }

    /*
     * Si aucun changements, alors on arrête l'itération, la solution a convergé
     */
    if (changes == 0) {
      break;
    }
  }

  /*
   * La génération est utile si on effectue plusieurs tentatives pour trouver une meilleure solution
   */
  m_generation++;
}

void KMeansParallel::classify(const std::vector<Point>& points, std::vector<uint8_t>& cluster) {
  cluster.resize(points.size());

  tbb::task_arena limited_arena(std::thread::hardware_concurrency());
  limited_arena.execute([&]{
      tbb::parallel_for(tbb::blocked_range<size_t>(0, points.size()), [&](tbb::blocked_range<size_t>& range) {
        for (size_t i = range.begin(); i < range.end(); i++) {
          double dist = std::numeric_limits<double>::max();
          for (size_t j = 0; j < m_cluster_centers.size(); j++) {
            double temp_dist = m_cluster_centers[j].dist(points[i]);
            if (temp_dist < dist) {
              cluster[i] = static_cast<uint8_t>(j);
              dist = temp_dist;
            }
          }
        }
      });
  });
}

size_t KMeansParallel::compare(const std::vector<uint8_t>& first,
                               const std::vector<uint8_t>& second,
                               const std::unordered_map<size_t, size_t>& mapping) {
  assert(first.size() == second.size());
  size_t good = 0;

  tbb::task_arena limited_arena(std::thread::hardware_concurrency());
  limited_arena.execute([&]{
      tbb::parallel_for(tbb::blocked_range<size_t>(0, first.size()), [&](tbb::blocked_range<size_t>& range) {
          for (size_t i = range.begin(); i < range.end(); i++) {
            size_t cluster_id = mapping.at(first[i]);
            if (second[i] == cluster_id) {
              good++;
            }
          }
      });
  });
  return good;
}
