#include "optimizer.hpp"

#include "data.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

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

  bool isPotentiallyFeasible(
      const TransformerInstance &instance,
      const CoreMaterial &core,
      const ConductorMaterial &conductor,
      const CoolingOption &cooling,
      double fluxDensity,
      double currentDensity,
      int layers,
      int ducts)
  {
    if (fluxDensity < 1.35 || fluxDensity > core.maxFluxDensity)
      return false;
    if (currentDensity < 1.60 || currentDensity > conductor.maxCurrentDensity)
      return false;
    if (layers < 4 || layers > 16)
      return false;
    if (ducts < 0 || ducts > 5)
      return false;

    const double temperature = 35.0 + 12.0 * std::pow(currentDensity / 2.2, 2.0) - 4.0 * static_cast<double>(ducts) - cooling.temperatureReduction;
    if (temperature > instance.maxTemperature)
      return false;

    const double impedance = 5.2 + 0.25 * static_cast<double>(layers - 8) + 0.8 * (fluxDensity - 1.55) - 0.35 * (currentDensity - 2.4) + 0.15 * static_cast<double>(ducts);
    if (impedance < instance.minImpedance || impedance > instance.maxImpedance)
      return false;

    const double coreWeight = instance.coreWeightFactor / fluxDensity * core.massFactor * (1.0 + 0.015 * std::pow(static_cast<double>(layers - 8), 2.0));
    const double conductorWeight = instance.conductorWeightFactor / currentDensity * conductor.massFactor * (1.0 + 0.04 * static_cast<double>(layers) + 0.03 * static_cast<double>(ducts));
    const double diameter = 1.1 + 0.00008 * conductorWeight + 0.00012 * coreWeight + 0.015 * static_cast<double>(layers);
    if (diameter > instance.maxDiameter)
      return false;

    return true;
  }
}

std::vector<EvaluationResult> getTopDesigns(const TransformerInstance &instance, std::size_t limit)
{
  const auto &cores = getCoreMaterials();
  const auto &conductors = getConductorMaterials();
  const auto &coolingOptions = getCoolingOptions();

  std::vector<EvaluationResult> feasibleDesigns;

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
                if (!isPotentiallyFeasible(instance, core, conductor, cooling, fluxDensity, currentDensity, layers, ducts))
                  continue;

                Design design{&core, &conductor, &cooling, fluxDensity, currentDensity, layers, ducts};
                EvaluationResult candidate = evaluateDesign(instance, design);

                if (candidate.feasible)
                  feasibleDesigns.push_back(candidate);
              }
            }
          }
        }
      }
    }
  }

  std::sort(feasibleDesigns.begin(), feasibleDesigns.end(), [](const EvaluationResult &lhs, const EvaluationResult &rhs)
            {
    if (std::abs(lhs.manufacturingCost - rhs.manufacturingCost) > 1e-9)
      return lhs.manufacturingCost < rhs.manufacturingCost;
    if (std::abs(lhs.totalLosses - rhs.totalLosses) > 1e-9)
      return lhs.totalLosses < rhs.totalLosses;
    return lhs.diameter < rhs.diameter; });

  if (feasibleDesigns.size() > limit)
    feasibleDesigns.resize(limit);

  return feasibleDesigns;
}

EvaluationResult optimizeDesign(const TransformerInstance &instance)
{
  const auto candidates = getTopDesigns(instance, 10);
  if (candidates.empty())
    return EvaluationResult{};

  return candidates.front();
}
