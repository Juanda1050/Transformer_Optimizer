#pragma once

#include <string>

struct CoreMaterial
{
  std::string id;
  double costPerKg;
  double lossCoefficient;
  double massFactor;
  double maxFluxDensity;
};

struct ConductorMaterial
{
  std::string id;
  double costPerKg;
  double resistanceFactor;
  double massFactor;
  double maxCurrentDensity;
};

struct CoolingOption
{
  std::string id;
  double costFactor;
  double temperatureReduction;
};

struct TransformerInstance
{
  std::string id;

  double ratedPowerKva;
  double coreWeightFactor;
  double conductorWeightFactor;

  double minImpedance;
  double maxImpedance;
  double maxTemperature;
  double maxDiameter;
};

struct Design
{
  const CoreMaterial *core;
  const ConductorMaterial *conductor;
  const CoolingOption *cooling;

  double fluxDensity;
  double currentDensity;

  int layers;
  int ducts;
};

struct EvaluationResult
{
  Design design;

  double coreWeight;
  double conductorWeight;

  double noLoadLosses;
  double loadLosses;
  double totalLosses;

  double manufacturingCost;
  double temperature;
  double impedance;
  double diameter;

  bool feasible;
};