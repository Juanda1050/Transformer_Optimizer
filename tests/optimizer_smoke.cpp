#include "optimizer.hpp"
#include <cassert>

int main()
{
  const auto &instances = getTransformerInstances();
  const auto best = optimizeDesign(instances[0]);
  assert(best.feasible);
  return 0;
}
