#include "kmeanproblem.h"

std::vector<KMeansClusterDefinition> KMeansClusterDefinition::load(const std::string& fname) {
  std::ifstream ifs(fname);
  return load(ifs);
}

std::vector<KMeansClusterDefinition> KMeansClusterDefinition::load(std::ifstream& ifs) {
  std::vector<KMeansClusterDefinition> definitions;

  if (!ifs.is_open()) {
    return definitions;
  }

  std::string line;
  while (std::getline(ifs, line)) {
    KMeansClusterDefinition def;
    std::stringstream ss(line);
    ss >> def.n_points >> def.center.x >> def.center.y >> def.stdev.x >> def.stdev.y;
    std::cout << def.n_points << " " << def.center.x << " " << def.center.y << " " << def.stdev.x
              << " " << def.stdev.y << std::endl;
    definitions.push_back(def);
  }
  return definitions;
}

void DataSet::generate_random(const std::vector<KMeansClusterDefinition>& def) {
  std::random_device random_device;
  std::mt19937 random_engine(random_device());

  // Combien de points au total?
  size_t total_points = 0;
  for (size_t i = 0; i < def.size(); i++) {
    total_points += def[i].n_points;
  }

  // Allouer la memoire
  m_points.resize(total_points);
  m_cluster.resize(total_points);

  // Créer les points pour chaque cluster
  size_t offset = 0;
  for (size_t cluster_id = 0; cluster_id < def.size(); cluster_id++) {
    const KMeansClusterDefinition& cluster = def.at(cluster_id);
    std::normal_distribution<double> dist_x(cluster.center.x, cluster.stdev.x);
    std::normal_distribution<double> dist_y(cluster.center.y, cluster.stdev.y);

    for (size_t i = 0; i < cluster.n_points; i++) {
      m_points[i + offset] = Point(dist_x(random_engine), dist_y(random_engine));
      m_cluster[i + offset] = cluster_id;
    }
    offset += cluster.n_points;
  }
}

void DataSet::generate_deterministic(const std::vector<KMeansClusterDefinition>& def) {
  // Combien de points au total?
  size_t total_points = 0;
  for (size_t i = 0; i < def.size(); i++) {
    total_points += def[i].n_points;
  }

  // Allouer la memoire
  m_points.resize(total_points);
  m_cluster.resize(total_points);

  // Créer les points pour chaque cluster
  size_t offset = 0;
  for (size_t cluster_id = 0; cluster_id < def.size(); cluster_id++) {
    const KMeansClusterDefinition& cluster = def.at(cluster_id);

    for (size_t i = 0; i < cluster.n_points; i++) {
      double angle = i * (2 * M_PI) / cluster.n_points;
      double x = 0.5 * cluster.stdev.x * std::cos(angle) + cluster.center.x;
      double y = 0.5 * cluster.stdev.y * std::sin(angle) + cluster.center.y;

      m_points[i + offset] = Point(x, y);
      m_cluster[i + offset] = cluster_id;
    }
    offset += cluster.n_points;
  }
}

void DataSet::write_points(const std::vector<std::vector<uint8_t>>& clusters,
                           const std::string& out) {
  std::ofstream ofs(out);

  // write header
  ofs << "x,y";
  for (size_t j = 0; j < clusters.size(); j++) {
    ofs << ",cluster_" << std::to_string(j);
  }
  ofs << "\n";

  // write data
  for (size_t i = 0; i < m_points.size(); i++) {
    const Point& point = m_points.at(i);
    ofs << point.x << ", " << point.y;
    for (size_t j = 0; j < clusters.size(); j++) {
      ofs << "," << (int)clusters[j][i];
    }
    ofs << "\n";
  }
}

std::vector<Point> start_points_random(const std::vector<Point>& points, int k, int generation) {
  std::vector<Point> centers;
  std::random_device random_device;
  std::mt19937 random_engine(random_device());
  std::uniform_int_distribution<size_t> distribution(0, points.size() - 1);
  for (uint64_t i = 0; i < k; i++) {
    size_t point_id = distribution(random_engine);
    centers.push_back(points[point_id]);
  }
  return centers;
}

std::vector<Point> start_points_deterministic(const std::vector<Point>& points, int k,
                                              int generation) {
  std::vector<Point> centers;
  for (uint64_t i = 0; i < k; i++) {
    size_t point_id = (generation * k + i) % points.size();
    centers.push_back(points[point_id]);
  }
  return centers;
}

int make_cluster_mapping(const std::vector<Point>& centers,
                         const std::vector<KMeansClusterDefinition>& definitions,
                         std::unordered_map<size_t, size_t>& matches) {
  // We can do better than O(n^2) using kdtree, but it's outside the scope of the course
  for (size_t i = 0; i < centers.size(); i++) {
    // find the closest cluster
    const Point& actual_center = centers.at(i);
    double min_dist = std::numeric_limits<double>::max();
    size_t match = 0;
    bool has_match = false;
    for (size_t j = 0; j < definitions.size(); j++) {
      // skip if already matched;
      if (matches.find(j) != matches.end()) {
        continue;
      }

      const Point& center = definitions[j].center;
      double distance = center.dist(actual_center);
      if (distance < min_dist) {
        min_dist = distance;
        match = j;
        has_match = true;
      }
    }

    if (!has_match) {
      std::cout << "FAIL: no match for center " << actual_center << std::endl;
      return -1;
    }
    matches[match] = i;
  }
  return 0;
}
