#include <iomanip>
#include <iostream>

#include "data.hpp"
#include "evaluator.hpp"
#include "optimizer.hpp"
#include "reporting.hpp"

namespace
{
  void printDesignSummary(
      const EvaluationResult &reference,
      const EvaluationResult &best)
  {
    std::cout << std::fixed << std::setprecision(6);

    std::cout << "PerdidasTotalesDeReferencia: "
              << reference.totalLosses << '\n';

    std::cout << "MejoresPerdidasTotalesFactibles: "
              << best.totalLosses << '\n';

    std::cout << "CostoFabricacionFactible: "
              << best.manufacturingCost << '\n';

    std::cout << "TemperaturaFactible: "
              << best.temperature << '\n';

    std::cout << "ImpedanciaFactible: "
              << best.impedance << '\n';

    std::cout << "DiametroFactible: "
              << best.diameter << '\n';

    std::cout << "DisenoFactible: "
              << best.design.core->id << " / "
              << best.design.conductor->id << " / "
              << best.design.cooling->id << " / Flujo="
              << best.design.fluxDensity << " / Corriente="
              << best.design.currentDensity << " / Capas="
              << best.design.layers << " / Conductos="
              << best.design.ducts << '\n';

    std::cout << "Factible: "
              << std::boolalpha << best.feasible << '\n';
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

  printDesignSummary(reference, best);

  const bool validationOk = reporting::validateOptimizationAgainstBruteforce();
  std::cout << "ValidacionNumerica: " << std::boolalpha << validationOk << '\n';

  reporting::printTopDesignsReport(10);

  reporting::printBenchmarkSummary();

  const bool csvOk = reporting::writeResultsCsv("results.csv");
  std::cout << "ExportacionCsv: " << std::boolalpha << csvOk << '\n';

  return validationOk && csvOk ? 0 : 1;
}