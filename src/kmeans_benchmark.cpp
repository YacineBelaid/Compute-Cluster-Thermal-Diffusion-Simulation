#include "kmeans.h"
#include "kmeans_parallel.h"
#include "modules.h"
#include <thread>
#include <vector>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "missing argument: problem file" << std::endl;
  }

  int occurrence = 10;
  int forEachOcc = 20;
  int maxThreads = std::thread::hardware_concurrency();

  std::vector<KMeansClusterDefinition> definitions(KMeansClusterDefinition::load(argv[1]));
  if (definitions.size() == 0) {
    std::printf("failed to open problem file %s\n", argv[1]);
    return -1;
  }

  struct BenchmarkData {
    std::string  methode;
    double durationFit;
    double durationClassify;
    double durationCompare;
  };

  struct MeansData {
      double accelerationFit;
      double efficienceFit;

      double accelerationClassify;
      double efficienceClassify;

      double accelerationCompare;
      double efficienceCompare;
  };

  std::vector<BenchmarkData> benchmarkResults;
  std::vector<MeansData> meansResults;

  DataSet training_set;
  training_set.generate_random(definitions);

  DataSet eval_set;
  eval_set.generate_random(definitions);

  double serialTotalTimeFit = 0;
  double serialTotalTimeClassify = 0;
  double serialTotalTimeCompare = 0;

  double parallelTotalTimeFit = 0;
  double parallelTotalTimeClassify = 0;
  double parallelTotalTimeCompare = 0;

  for (int i = 0; i < occurrence; i++) {
    // Points de départ pour initialiser l'algorithme
    std::vector<Point> start_points
        = start_points_random(training_set.m_points, definitions.size(), 0);

    for (const struct test_klass& item : algo) {
        std::cout << "BENCHMARK " << item.klass << "\n";
        std::shared_ptr<KMeans> kmeans = item.factory();

            // Trouver les groupes dans le jeu de données
            auto serialStartTimeFit = std::chrono::steady_clock::now();
            kmeans->fit(training_set.m_points, start_points);
            auto serialEndTimeFit = std::chrono::steady_clock::now();
            double serialDurationFit = std::chrono::duration_cast<std::chrono::milliseconds>(serialEndTimeFit - serialStartTimeFit).count();


            // classifier un nouveau jeu de données
            std::vector<uint8_t> eval_cluster;
            auto serialStartTimeClassify = std::chrono::steady_clock::now();
            kmeans->classify(eval_set.m_points, eval_cluster);
            auto serialEndTimeClassify = std::chrono::steady_clock::now();
            double serialDurationClassify = std::chrono::duration_cast<std::chrono::milliseconds>(serialEndTimeClassify - serialStartTimeClassify).count();


            // comparer pour obtenir le taux de précision
            std::unordered_map<size_t, size_t> mapping;
            make_cluster_mapping(kmeans->get_centers(), definitions, mapping);
            auto serialStartTimeCompare = std::chrono::steady_clock::now();
            size_t goodSerie = kmeans->compare(eval_set.m_cluster, eval_cluster, mapping);
            auto serialEndTimeCompare = std::chrono::steady_clock::now();
            double serialDurationCompare = std::chrono::duration_cast<std::chrono::milliseconds>(serialEndTimeCompare - serialStartTimeCompare).count();
            std::cout << "precision serie: " << 100 * goodSerie / eval_cluster.size() << " %" << std::endl;

      BenchmarkData data;
      if (item.klass == "serial") {
        data.methode = "serial";
        serialTotalTimeFit += serialDurationFit;
        serialTotalTimeClassify += serialDurationClassify;
        serialTotalTimeCompare += serialDurationCompare;
      } else {
        data.methode = "parallel";
        parallelTotalTimeFit += serialDurationFit;
        parallelTotalTimeClassify += serialDurationClassify;
        parallelTotalTimeCompare += serialDurationCompare;
      }

      data.durationFit = serialDurationFit;
      data.durationClassify = serialDurationClassify;
      data.durationCompare = serialDurationCompare;
      benchmarkResults.push_back(data);
    }
  }

  for (int i = 0; i < forEachOcc; i+=2) {
    MeansData dMeans;
    dMeans.accelerationFit = benchmarkResults[i].durationFit / benchmarkResults[i+1].durationFit;
    dMeans.efficienceFit = dMeans.accelerationFit / (maxThreads - 1);

    dMeans.accelerationClassify = benchmarkResults[i].durationClassify / benchmarkResults[i+1].durationClassify;
    dMeans.efficienceClassify = dMeans.accelerationClassify / (maxThreads - 1);

    dMeans.accelerationCompare = benchmarkResults[i].durationCompare / benchmarkResults[i+1].durationCompare;
    dMeans.efficienceCompare = dMeans.accelerationCompare / (maxThreads - 1);
    meansResults.push_back(dMeans);
  }

  double avgSerialFit = serialTotalTimeFit / occurrence;
  double avgParallelFit = parallelTotalTimeFit / occurrence;
  double avgAccelerationFit = avgSerialFit / avgParallelFit;
  double avgEfficienceFit = avgAccelerationFit / (maxThreads - 1);

  double avgSerialClassify = serialTotalTimeClassify / occurrence;
  double avgParallelClassify = parallelTotalTimeClassify / occurrence;
  double avgAccelerationClassify = avgSerialClassify / avgParallelClassify;
  double avgEfficienceFitClassify = avgAccelerationClassify / (maxThreads - 1);

  double avgSerialCompare = serialTotalTimeCompare / occurrence;
  double avgParallelCompare = parallelTotalTimeCompare / occurrence;
  double avgAccelerationCompare = avgSerialCompare / avgParallelCompare;
  double avgEfficienceCompare = avgAccelerationCompare / (maxThreads - 1);

  std::ofstream outputFile("benchmark2.csv");
    if (outputFile.is_open()) {
      outputFile << "Fonction, Duree Serie, Duree Parallele, Acceleration, Efficience, " << std::endl;

      for (int i = 0, j = 0; i < forEachOcc; i+=2, j++) {
          outputFile << "Fit" << ", "
                      << benchmarkResults[i].durationFit << ", " << benchmarkResults[i+1].durationFit << ", "
                      << meansResults[j].accelerationFit << ", " << meansResults[j].efficienceFit << std::endl;

          outputFile << "Classify" << ", "
                      << benchmarkResults[i].durationClassify << ", " << benchmarkResults[i+1].durationClassify << ", "
                      << meansResults[j].accelerationClassify << ", " << meansResults[j].efficienceClassify << std::endl;

          outputFile << "Compare" << ", "
                      << benchmarkResults[i].durationCompare << ", " << benchmarkResults[i+1].durationCompare << ", "
                      << meansResults[j].accelerationCompare << ", " << meansResults[j].efficienceCompare << std::endl;
      }

      outputFile << "" << std::endl;
      outputFile << "Fonction, Moyenne Serie, Moyenne Parallele, Moyenne Acceleration, Moyenne Efficience, " << std::endl;
      outputFile << "Fit" << ", "
                  << avgSerialFit << ", " << avgParallelFit << ", "
                  << avgAccelerationFit << ", " << avgEfficienceFit << std::endl;

      outputFile << "Classify" << ", "
                  << avgSerialClassify << ", " << avgParallelClassify << ", "
                  << avgAccelerationClassify << ", " << avgEfficienceFitClassify << std::endl;

      outputFile << "Compare" << ", "
                  << avgSerialCompare << ", " << avgParallelCompare << ", "
                  << avgAccelerationCompare << ", " << avgEfficienceCompare << std::endl;
      outputFile.close();
    } else {
      std::cerr << "Impossible d'ouvrir le fichier benchmark2.csv pour écriture." << std::endl;
      return -1;
    }

  return 0;
}
