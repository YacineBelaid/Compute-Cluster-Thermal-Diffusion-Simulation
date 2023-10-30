#pragma once

#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>

#include "kmeanproblem.h"
#include "kmeans.h"
#include "point.h"

class KMeansParallel : public KMeans {
public:
  KMeansParallel();

  void fit(const std::vector<Point>& points, const std::vector<Point>& start_points) override;
  void classify(const std::vector<Point>& points, std::vector<uint8_t>& cluster) override;
  virtual size_t compare(const std::vector<uint8_t>& first, const std::vector<uint8_t>& second,
                         const std::unordered_map<size_t, size_t>& mapping) override;
};
