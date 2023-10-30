#pragma once

#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>

#include "kmeanproblem.h"
#include "point.h"

class KMeans {
public:
  KMeans();
  virtual ~KMeans() = default;

  virtual void fit(const std::vector<Point>& points, const std::vector<Point>& initial_centers);
  virtual void classify(const std::vector<Point>& points, std::vector<uint8_t>& cluster);
  virtual size_t compare(const std::vector<uint8_t>& first, const std::vector<uint8_t>& second,
                         const std::unordered_map<size_t, size_t>& mapping);

  void load_clusters(std::ifstream& ifs);
  void write_clusters(std::ofstream& ofs);

  std::vector<uint8_t>& get_cluster() { return m_cluster; }
  std::vector<Point>& get_centers() { return m_cluster_centers; }
  int get_generation() { return m_generation; }

protected:
  int m_generation;
  std::vector<uint8_t> m_cluster;
  std::vector<Point> m_cluster_centers;
};
