#include "optimizer.hpp"

#include "data.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
  constexpr double MIN_FLUX_DENSITY = 1.35;
  constexpr double FLUX_STEP = 0.05;
  constexpr double MIN_CURRENT_DENSITY = 1.60;
  constexpr double CURRENT_STEP = 0.10;
  constexpr int MIN_LAYERS = 4;
  constexpr int MAX_LAYERS = 16;
  constexpr int MIN_DUCTS = 0;
  constexpr int MAX_DUCTS = 5;

  bool rankedBefore(const EvaluationResult &lhs, const EvaluationResult &rhs)
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

  std::size_t countDensitySteps(double minValue, double maxValue, double step)
  {
    if (maxValue < minValue)
      return 0;

    const double span = maxValue - minValue;
    return static_cast<std::size_t>(std::floor(span / step + 1e-9)) + 1;
  }

  std::size_t countExhaustiveDesigns()
  {
    const auto &cores = getCoreMaterials();
    const auto &conductors = getConductorMaterials();
    const auto &coolingOptions = getCoolingOptions();

    std::size_t total = 0;

    for (const auto &core : cores)
    {
      for (const auto &conductor : conductors)
      {
        for (const auto &cooling : coolingOptions)
        {
          const std::size_t fluxSteps = countDensitySteps(MIN_FLUX_DENSITY, core.maxFluxDensity, FLUX_STEP);
          const std::size_t currentSteps = countDensitySteps(MIN_CURRENT_DENSITY, conductor.maxCurrentDensity, CURRENT_STEP);
          const std::size_t layerCount = static_cast<std::size_t>(MAX_LAYERS - MIN_LAYERS + 1);
          const std::size_t ductCount = static_cast<std::size_t>(MAX_DUCTS - MIN_DUCTS + 1);

          total += fluxSteps * currentSteps * layerCount * ductCount;
        }
      }
    }

    return total;
  }

  void keepTopDesigns(std::vector<EvaluationResult> &topDesigns,
                      const EvaluationResult &candidate,
                      std::size_t limit)
  {
    if (!candidate.feasible || limit == 0)
      return;

    if (topDesigns.size() < limit)
    {
      topDesigns.push_back(candidate);
      std::sort(topDesigns.begin(), topDesigns.end(), rankedBefore);
      return;
    }

    if (!rankedBefore(candidate, topDesigns.back()))
      return;

    topDesigns.back() = candidate;
    std::sort(topDesigns.begin(), topDesigns.end(), rankedBefore);
  }
}

std::vector<EvaluationResult> optimizeDesigns(const TransformerInstance &instance, std::size_t limit)
{
  if (limit == 0)
    return {};

  const auto &cores = getCoreMaterials();
  const auto &conductors = getConductorMaterials();
  const auto &coolingOptions = getCoolingOptions();

  std::vector<EvaluationResult> topDesigns;
  topDesigns.reserve(std::min(limit, countExhaustiveDesigns()));

  for (const auto &core : cores)
  {
    for (const auto &conductor : conductors)
    {
      for (const auto &cooling : coolingOptions)
      {
        for (double fluxDensity = MIN_FLUX_DENSITY; fluxDensity <= core.maxFluxDensity + 1e-9; fluxDensity += FLUX_STEP)
        {
          for (double currentDensity = MIN_CURRENT_DENSITY; currentDensity <= conductor.maxCurrentDensity + 1e-9; currentDensity += CURRENT_STEP)
          {
            for (int layers = MIN_LAYERS; layers <= MAX_LAYERS; ++layers)
            {
              for (int ducts = MIN_DUCTS; ducts <= MAX_DUCTS; ++ducts)
              {
                Design design{&core, &conductor, &cooling, fluxDensity, currentDensity, layers, ducts};

                if (!isPotentiallyFeasible(instance, design))
                  continue;

                const EvaluationResult candidate = evaluateDesign(instance, design);

                if (candidate.feasible)
                  keepTopDesigns(topDesigns, candidate, limit);
              }
            }
          }
        }
      }
    }
  }

  std::sort(topDesigns.begin(), topDesigns.end(), rankedBefore);
  return topDesigns;
}

std::vector<EvaluationResult> getTopDesigns(const TransformerInstance &instance, std::size_t limit)
{
  return optimizeDesigns(instance, limit);
}

EvaluationResult optimizeDesign(const TransformerInstance &instance)
{
  const auto candidates = optimizeDesigns(instance, 1);
  if (candidates.empty())
    return EvaluationResult{};

  return candidates.front();
}
