#include "reporting.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace
{
  struct BenchmarkSummary
  {
    int totalEvaluated = 0;
    int feasibleCount = 0;
    double bestCost = std::numeric_limits<double>::infinity();
    double elapsedMs = 0.0;
  };

  bool isBetterCandidate(const EvaluationResult &candidate, const EvaluationResult &current)
  {
    if (!current.feasible)
      return candidate.feasible;
    if (!candidate.feasible)
      return false;

    if (std::abs(candidate.manufacturingCost - current.manufacturingCost) > 1e-9)
      return candidate.manufacturingCost < current.manufacturingCost;

    if (std::abs(candidate.totalLosses - current.totalLosses) > 1e-9)
      return candidate.totalLosses < current.totalLosses;

    return candidate.diameter < current.diameter;
  }

  BenchmarkSummary benchmarkInstance(const TransformerInstance &instance)
  {
    BenchmarkSummary summary{};
    const auto start = std::chrono::steady_clock::now();

    for (const auto &core : getCoreMaterials())
    {
      for (const auto &conductor : getConductorMaterials())
      {
        for (const auto &cooling : getCoolingOptions())
        {
          for (double fluxDensity = 1.35; fluxDensity <= core.maxFluxDensity + 1e-9; fluxDensity += 0.05)
          {
            for (double currentDensity = 1.60; currentDensity <= conductor.maxCurrentDensity + 1e-9; currentDensity += 0.10)
            {
              for (int layers = 4; layers <= 16; ++layers)
              {
                for (int ducts = 0; ducts <= 5; ++ducts)
                {
                  ++summary.totalEvaluated;
                  Design design{&core, &conductor, &cooling, fluxDensity, currentDensity, layers, ducts};
                  const EvaluationResult candidate = evaluateDesign(instance, design);

                  if (candidate.feasible)
                  {
                    ++summary.feasibleCount;
                    if (candidate.manufacturingCost < summary.bestCost)
                    {
                      summary.bestCost = candidate.manufacturingCost;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    const auto end = std::chrono::steady_clock::now();
    summary.elapsedMs = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.0;
    return summary;
  }
}

bool reporting::validateOptimizationAgainstBruteforce()
{
  const auto &instances = getTransformerInstances();

  for (const auto &instance : instances)
  {
    EvaluationResult optimum = optimizeDesign(instance);
    EvaluationResult bruteForce{};
    bool found = false;

    for (const auto &core : getCoreMaterials())
    {
      for (const auto &conductor : getConductorMaterials())
      {
        for (const auto &cooling : getCoolingOptions())
        {
          for (double fluxDensity = 1.35; fluxDensity <= core.maxFluxDensity + 1e-9; fluxDensity += 0.05)
          {
            for (double currentDensity = 1.60; currentDensity <= conductor.maxCurrentDensity + 1e-9; currentDensity += 0.10)
            {
              for (int layers = 4; layers <= 16; ++layers)
              {
                for (int ducts = 0; ducts <= 5; ++ducts)
                {
                  Design design{&core, &conductor, &cooling, fluxDensity, currentDensity, layers, ducts};
                  EvaluationResult candidate = evaluateDesign(instance, design);

                  if (isBetterCandidate(candidate, bruteForce))
                  {
                    bruteForce = candidate;
                    found = true;
                  }
                }
              }
            }
          }
        }
      }
    }

    if (!found || !optimum.feasible || !bruteForce.feasible)
      return false;

    if (std::abs(optimum.manufacturingCost - bruteForce.manufacturingCost) > 1e-6 ||
        std::abs(optimum.totalLosses - bruteForce.totalLosses) > 1e-6 ||
        std::abs(optimum.diameter - bruteForce.diameter) > 1e-6)
    {
      return false;
    }
  }

  return true;
}

void reporting::printTopDesignsReport(std::size_t topN)
{
  if (topN == 0)
    return;

  std::cout << std::fixed << std::setprecision(2);

  for (const auto &instance : getTransformerInstances())
  {
    const auto topDesigns = getTopDesigns(instance, topN);

    std::cout << "\nInstance_" << instance.id << "\n\n";
    std::cout << "Rank | Core | Conductor | Cooling | Flux | Current | Layers | Ducts | Cost | Losses | Diameter\n";
    std::cout << "----------------------------------------------------------------------------------------------------------\n";

    for (std::size_t i = 0; i < topDesigns.size(); ++i)
    {
      const auto &result = topDesigns[i];
      std::cout << std::setw(4) << (i + 1) << " | "
                << std::setw(4) << result.design.core->id << " | "
                << std::setw(9) << result.design.conductor->id << " | "
                << std::setw(7) << result.design.cooling->id << " | "
                << std::setw(4) << result.design.fluxDensity << " | "
                << std::setw(7) << result.design.currentDensity << " | "
                << std::setw(6) << result.design.layers << " | "
                << std::setw(5) << result.design.ducts << " | "
                << std::setw(8) << result.manufacturingCost << " | "
                << std::setw(6) << result.totalLosses << " | "
                << std::setw(8) << result.diameter << '\n';
    }
  }
}

void reporting::printBenchmarkSummary()
{
  std::cout << "\nResumenDelBenchmark:\n";
  for (const auto &instance : getTransformerInstances())
  {
    const BenchmarkSummary summary = benchmarkInstance(instance);
    std::cout << "Instancia" << instance.id
              << ": Evaluados=" << summary.totalEvaluated
              << ", Factibles=" << summary.feasibleCount
              << ", MejorCosto=" << summary.bestCost
              << ", TiempoMs=" << summary.elapsedMs << '\n';
  }
}

bool reporting::writeResultsCsv(const std::string &outputPath)
{
  std::ofstream output(outputPath);
  if (!output)
    return false;

  const auto &instances = getTransformerInstances();

  output << "Instancia,Ranking,Material Nucleo,Material Conductor,Refrigeracion,Flujo,Corriente,Capas,Conductos,Costo Fabricacion,Perdidas Totales,Temperatura,Impedancia,Diametro\n";

  for (const auto &instance : instances)
  {
    const auto topDesigns = getTopDesigns(instance, 10);

    for (std::size_t i = 0; i < topDesigns.size(); ++i)
    {
      const auto &design = topDesigns[i];
      output << instance.id << ','
             << (i + 1) << ','
             << design.design.core->id << ','
             << design.design.conductor->id << ','
             << design.design.cooling->id << ','
             << std::fixed << std::setprecision(2) << design.design.fluxDensity << ','
             << design.design.currentDensity << ','
             << design.design.layers << ','
             << design.design.ducts << ','
             << design.manufacturingCost << ','
             << design.totalLosses << ','
             << design.temperature << ','
             << design.impedance << ','
             << design.diameter << '\n';
    }
  }

  return true;
}
