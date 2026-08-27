#pragma once

#include <string>

#include "data.hpp"
#include "evaluator.hpp"
#include "optimizer.hpp"

namespace reporting
{
  bool validateOptimizationAgainstBruteforce();
  void printTopDesignsReport(std::size_t topN = 10);
  void printBenchmarkSummary();
  bool writeResultsCsv(const std::string &outputPath = "results.csv");
}
