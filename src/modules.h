#pragma once

#include <functional>
#include <memory>
#include <string>

#include "kmeans.h"
#include "kmeans_parallel.h"

using factory_function_t = std::function<std::shared_ptr<KMeans>()>;

struct test_klass {
  std::string klass;
  factory_function_t factory;
};

std::shared_ptr<KMeans> CreateKMeansSerial();
std::shared_ptr<KMeansParallel> CreateKMeansParallel();

extern const std::vector<struct test_klass> algo;
