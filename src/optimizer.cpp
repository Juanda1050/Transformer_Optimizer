#include "optimizer.hpp"

#include "data.hpp"

#include <algorithm>
#include <cmath>

namespace
{
  bool isBetter(const EvaluationResult &lhs, const EvaluationResult &rhs)
  {
    if (!rhs.feasible)
      return lhs.feasible;
    if (!lhs.feasible)
      return false;

    if (std::abs(lhs.manufacturingCost - rhs.manufacturingCost) > 1e-9)
      return lhs.manufacturingCost < rhs.manufacturingCost;

    if (std::abs(lhs.totalLosses - rhs.totalLosses) > 1e-9)
      return lhs.totalLosses < rhs.totalLosses;

    return lhs.diameter < rhs.diameter;
  }
}

EvaluationResult optimizeDesign(const TransformerInstance &instance)
{
  const auto &cores = getCoreMaterials();
  const auto &conductors = getConductorMaterials();
  const auto &coolingOptions = getCoolingOptions();

  EvaluationResult best{};
  bool foundCandidate = false;

  for (const auto &core : cores)
  {
    for (const auto &conductor : conductors)
    {
      for (const auto &cooling : coolingOptions)
      {
        constexpr double FLUX_STEP = 0.05;
        constexpr double CURRENT_STEP = 0.10;

        for (double fluxDensity = 1.35; fluxDensity <= core.maxFluxDensity + 1e-9; fluxDensity += FLUX_STEP)
        {
          for (double currentDensity = 1.60; currentDensity <= conductor.maxCurrentDensity + 1e-9; currentDensity += CURRENT_STEP)
          {
            for (int layers = 4; layers <= 16; ++layers)
            {
              for (int ducts = 0; ducts <= 5; ++ducts)
              {
                Design design{&core, &conductor, &cooling, fluxDensity, currentDensity, layers, ducts};
                EvaluationResult candidate = evaluateDesign(instance, design);

                if (!foundCandidate || isBetter(candidate, best))
                {
                  best = candidate;
                  foundCandidate = true;
                }
              }
            }
          }
        }
      }
    }
  }

  return foundCandidate ? best : EvaluationResult{};
}
