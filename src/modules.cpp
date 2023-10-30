#include "modules.h"

const std::vector<struct test_klass> algo = {
    {"serial", CreateKMeansSerial},
    {"parallel", CreateKMeansParallel},
};

std::shared_ptr<KMeans> CreateKMeansSerial() { return std::make_shared<KMeans>(); }

std::shared_ptr<KMeansParallel> CreateKMeansParallel() {
  return std::make_shared<KMeansParallel>();
}
