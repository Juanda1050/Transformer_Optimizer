#include <iomanip>
#include <iostream>

#include "data.hpp"
#include "evaluator.hpp"
#include "optimizer.hpp"
#include "reporting.hpp"

namespace
{
  void printDesignSummary(const EvaluationResult &result)
  {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "PerdidasTotalesDeReferencia: " << result.totalLosses << '\n';
    std::cout << "MejoresPerdidasTotalesFactibles: " << result.totalLosses << '\n';
    std::cout << "CostoFabricacionFactible: " << result.manufacturingCost << '\n';
    std::cout << "TemperaturaFactible: " << result.temperature << '\n';
    std::cout << "ImpedanciaFactible: " << result.impedance << '\n';
    std::cout << "DiametroFactible: " << result.diameter << '\n';
    std::cout << "DisenoFactible: "
              << result.design.core->id << " / "
              << result.design.conductor->id << " / "
              << result.design.cooling->id << " / Flujo="
              << result.design.fluxDensity << " / Corriente="
              << result.design.currentDensity << " / Capas="
              << result.design.layers << " / Conductos="
              << result.design.ducts << '\n';
    std::cout << "Factible: " << std::boolalpha << result.feasible << '\n';
  }
}

int main()
{
  const auto &instances = getTransformerInstances();
  const auto &cores = getCoreMaterials();
  const auto &conductors = getConductorMaterials();
  const auto &coolingOptions = getCoolingOptions();

  const Design referenceDesign{
      &cores[0],
      &conductors[0],
      &coolingOptions[0],
      1.50,
      2.00,
      8,
      1};

  const EvaluationResult reference = evaluateDesign(instances[0], referenceDesign);
  const EvaluationResult best = optimizeDesign(instances[0]);

  printDesignSummary(best);

  const bool validationOk = reporting::validateOptimizationAgainstBruteforce();
  std::cout << "ValidacionNumerica: " << std::boolalpha << validationOk << '\n';

  reporting::printBenchmarkSummary();

  const bool csvOk = reporting::writeResultsCsv("results.csv");
  std::cout << "ExportacionCsv: " << std::boolalpha << csvOk << '\n';

  return validationOk && csvOk ? 0 : 1;
}