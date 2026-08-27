#include "evaluator.hpp"
#include <cmath>

namespace
{
  constexpr double MIN_FLUX_DENSITY = 1.35;
  constexpr double MIN_CURRENT_DENSITY = 1.60;

  constexpr int MIN_LAYERS = 4;
  constexpr int MAX_LAYERS = 16;

  constexpr int MIN_DUCTS = 0;
  constexpr int MAX_DUCTS = 5;

  EvaluationResult buildEvaluationResult(const TransformerInstance &instance, const Design &design)
  {
    EvaluationResult result{};
    result.design = design;

    result.coreWeight = instance.coreWeightFactor / design.fluxDensity * design.core->massFactor * (1.0 + 0.015 * std::pow(static_cast<double>(design.layers - 8), 2.0));
    result.conductorWeight = instance.conductorWeightFactor / design.currentDensity * design.conductor->massFactor * (1.0 + 0.04 * static_cast<double>(design.layers) + 0.03 * static_cast<double>(design.ducts));
    result.noLoadLosses = design.core->lossCoefficient * result.coreWeight * std::pow(design.fluxDensity, 2.1);
    result.loadLosses = 0.0045 * design.conductor->resistanceFactor * result.conductorWeight * std::pow(design.currentDensity, 2.0);
    result.totalLosses = result.noLoadLosses + 0.55 * result.loadLosses;
    result.manufacturingCost = result.coreWeight * design.core->costPerKg + result.conductorWeight * design.conductor->costPerKg + design.cooling->costFactor + 500.0 * static_cast<double>(design.ducts) + 80.0 * static_cast<double>(design.layers);
    result.temperature = 35.0 + 12.0 * std::pow(design.currentDensity / 2.2, 2.0) - 4.0 * static_cast<double>(design.ducts) - design.cooling->temperatureReduction;
    result.impedance = 5.2 + 0.25 * static_cast<double>(design.layers - 8) + 0.8 * (design.fluxDensity - 1.55) - 0.35 * (design.currentDensity - 2.4) + 0.15 * static_cast<double>(design.ducts);
    result.diameter = 1.1 + 0.00008 * result.conductorWeight + 0.00012 * result.coreWeight + 0.015 * static_cast<double>(design.layers);

    return result;
  }
}

EvaluationResult evaluateDesign(
    const TransformerInstance &instance,
    const Design &design)
{
  EvaluationResult result = buildEvaluationResult(instance, design);

  result.feasible = isFeasible(instance, design, result);

  return result;
}

bool isPotentiallyFeasible(const TransformerInstance &instance, const Design &design)
{
  const EvaluationResult result = buildEvaluationResult(instance, design);
  return isFeasible(instance, design, result);
}

bool isFeasible(const TransformerInstance &instance, const Design &design, const EvaluationResult &result)
{
  const bool validFluxDensity = design.fluxDensity >= MIN_FLUX_DENSITY && design.fluxDensity <= design.core->maxFluxDensity;

  const bool validCurrentDensity = design.currentDensity >= MIN_CURRENT_DENSITY && design.currentDensity <= design.conductor->maxCurrentDensity;

  const bool validLayers = design.layers >= MIN_LAYERS && design.layers <= MAX_LAYERS;

  const bool validDucts = design.ducts >= MIN_DUCTS && design.ducts <= MAX_DUCTS;

  const bool validTemperature = result.temperature <= instance.maxTemperature;

  const bool validImpedance = result.impedance >= instance.minImpedance && result.impedance <= instance.maxImpedance;

  const bool validDiameter = result.diameter <= instance.maxDiameter;

  return validFluxDensity && validCurrentDensity && validLayers && validDucts && validTemperature && validImpedance && validDiameter;
}