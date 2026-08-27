#include <iomanip>
#include <iostream>

#include "data.hpp"
#include "evaluator.hpp"

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

  std::cout << std::fixed << std::setprecision(6);

  std::cout << "Core weight: "
            << result.coreWeight << '\n';

  std::cout << "Conductor weight: "
            << result.conductorWeight << '\n';

  std::cout << "No-load losses: "
            << result.noLoadLosses << '\n';

  std::cout << "Load losses: "
            << result.loadLosses << '\n';

  std::cout << "Total losses: "
            << result.totalLosses << '\n';

  std::cout << "Manufacturing cost: "
            << result.manufacturingCost << '\n';

  std::cout << "Temperature: "
            << result.temperature << '\n';

  std::cout << "Impedance: "
            << result.impedance << '\n';

  std::cout << "Diameter: "
            << result.diameter << '\n';

  std::cout << "Feasible: "
            << std::boolalpha
            << result.feasible << '\n';

  return 0;
}