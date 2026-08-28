#pragma once

#include <string>

#include "data.hpp"
#include "evaluator.hpp"
#include "optimizer.hpp"

namespace reporting
{
  struct ValidationReport
  {
    bool referenceComparison = false;
    bool bestFeasibleDesign = false;
    bool rankingOrder = false;
    bool constraintChecks = false;

    bool allPassed() const
    {
      return referenceComparison && bestFeasibleDesign && rankingOrder && constraintChecks;
    }
  };

  bool validateOptimizationAgainstBruteforce();
  ValidationReport buildValidationReport(std::size_t topN = 10);
  void printValidationReport(const ValidationReport &report);
  void printTopDesignsReport(std::size_t topN = 10);
  void printBenchmarkSummary();
  bool writeResultsCsv(const std::string &outputPath = "results.csv");
}
