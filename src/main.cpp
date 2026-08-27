#include <cmath>
#include <iomanip>
#include <iostream>

#include "data.hpp"
#include "evaluator.hpp"
#include "optimizer.hpp"

namespace
{
  bool validateOptimizationAgainstBruteforce()
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

                    if (candidate.feasible && (!found ||
                                               candidate.manufacturingCost < bruteForce.manufacturingCost ||
                                               (std::abs(candidate.manufacturingCost - bruteForce.manufacturingCost) < 1e-9 &&
                                                (candidate.totalLosses < bruteForce.totalLosses ||
                                                 (std::abs(candidate.totalLosses - bruteForce.totalLosses) < 1e-9 &&
                                                  candidate.diameter < bruteForce.diameter)))))
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
      {
        std::cout << "Validation failed for instance " << instance.id << '\n';
        return false;
      }

      const double deltaCost = std::abs(optimum.manufacturingCost - bruteForce.manufacturingCost);
      const double deltaLoss = std::abs(optimum.totalLosses - bruteForce.totalLosses);
      const double deltaDiameter = std::abs(optimum.diameter - bruteForce.diameter);

      if (deltaCost > 1e-6 || deltaLoss > 1e-6 || deltaDiameter > 1e-6)
      {
        std::cout << "Mismatch for instance " << instance.id << '\n';
        return false;
      }
    }

    return true;
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

  const EvaluationResult result =
      evaluateDesign(instances[0], referenceDesign);

  const EvaluationResult best = optimizeDesign(instances[0]);

  std::cout << std::fixed << std::setprecision(6);

  std::cout << "Reference total losses: "
            << result.totalLosses << '\n';

  std::cout << "Best feasible total losses: "
            << best.totalLosses << '\n';

  std::cout << "Best feasible manufacturing cost: "
            << best.manufacturingCost << '\n';

  std::cout << "Best feasible temperature: "
            << best.temperature << '\n';

  std::cout << "Best feasible impedance: "
            << best.impedance << '\n';

  std::cout << "Best feasible diameter: "
            << best.diameter << '\n';

  std::cout << "Best feasible design: "
            << best.design.core->id << " / "
            << best.design.conductor->id << " / "
            << best.design.cooling->id << " / flux="
            << best.design.fluxDensity << " / current="
            << best.design.currentDensity << " / layers="
            << best.design.layers << " / ducts="
            << best.design.ducts << '\n';

  std::cout << "Feasible: "
            << std::boolalpha
            << best.feasible << '\n';

  const bool validationOk = validateOptimizationAgainstBruteforce();
  std::cout << "Numerical validation: "
            << std::boolalpha
            << validationOk << '\n';

  return validationOk ? 0 : 1;
}