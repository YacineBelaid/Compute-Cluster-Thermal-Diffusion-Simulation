#pragma once

#include <inttypes.h>

#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "point.h"

class KMeansClusterDefinition {
public:
  KMeansClusterDefinition() : n_points(0), center(0, 0), stdev(0, 0) {}
  KMeansClusterDefinition(size_t p, Point c, Point s) : n_points(p), center(c), stdev(s) {}
  size_t n_points;
  Point center;
  Point stdev;
  static std::vector<KMeansClusterDefinition> load(const std::string& fname);
  static std::vector<KMeansClusterDefinition> load(std::ifstream& ifs);
};

class DataSet {
public:
  std::vector<Point> m_points;
  std::vector<uint8_t> m_cluster;

  void generate_random(const std::vector<KMeansClusterDefinition>& def);
  void generate_deterministic(const std::vector<KMeansClusterDefinition>& def);

  void write_points(const std::vector<std::vector<uint8_t> >& clusters, const std::string& out);
};

std::vector<Point> start_points_random(const std::vector<Point>& points, int k, int generation);
std::vector<Point> start_points_deterministic(const std::vector<Point>& points, int k,
                                              int generation);

int make_cluster_mapping(const std::vector<Point>& centers,
                         const std::vector<KMeansClusterDefinition>& definitions,
                         std::unordered_map<size_t, size_t>& matches);
