#include <iomanip>
#include <iostream>

#include "data.hpp"
#include "reporting.hpp"

int main()
{
  std::cout << "\nTransformer Optimizer\n";
  std::cout << "=====================\n";

  reporting::printTopDesignsReport(10);

  const reporting::ValidationReport validation = reporting::buildValidationReport(10);
  reporting::printValidationReport(validation);

  const bool csvOk = reporting::writeResultsCsv("results.csv");
  std::cout << "\nCSV Export\n";
  std::cout << "----------\n";
  std::cout << "Results file: " << (csvOk ? "PASS" : "FAIL") << '\n';

  return validation.allPassed() && csvOk ? 0 : 1;
}