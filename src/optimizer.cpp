#include "optimizer.hpp"

#include "data.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
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

  std::size_t countExhaustiveDesigns(const TransformerInstance &instance)
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
          const std::size_t fluxSteps = countDensitySteps(1.35, core.maxFluxDensity, 0.05);
          const std::size_t currentSteps = countDensitySteps(1.60, conductor.maxCurrentDensity, 0.10);
          const std::size_t layerCount = 16 - 4 + 1;
          const std::size_t ductCount = 5 - 0 + 1;

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

    topDesigns.pop_back();
    const auto insertionPoint = std::lower_bound(topDesigns.begin(), topDesigns.end(), candidate, rankedBefore);
    topDesigns.insert(insertionPoint, candidate);
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
  topDesigns.reserve(std::min(limit, countExhaustiveDesigns(instance)));

  std::size_t evaluated = 0;
  const std::size_t expectedEvaluations = countExhaustiveDesigns(instance);

  for (const auto &core : cores)
  {
    for (const auto &conductor : conductors)
    {
      for (const auto &cooling : coolingOptions)
      {
        for (double fluxDensity = 1.35; fluxDensity <= core.maxFluxDensity + 1e-9; fluxDensity += 0.05)
        {
          for (double currentDensity = 1.60; currentDensity <= conductor.maxCurrentDensity + 1e-9; currentDensity += 0.10)
          {
            for (int layers = 4; layers <= 16; ++layers)
            {
              for (int ducts = 0; ducts <= 5; ++ducts)
              {
                ++evaluated;

                Design design{&core, &conductor, &cooling, fluxDensity, currentDensity, layers, ducts};
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

  if (evaluated != expectedEvaluations)
    return topDesigns;

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
