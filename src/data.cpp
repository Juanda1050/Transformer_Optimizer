#include "data.hpp"

const std::array<CoreMaterial, 3> CORE_MATERIALS = {{{"M1", 4.8, 0.00190, 1.00, 1.70},
                                                     {"M2", 5.6, 0.00155, 0.96, 1.75},
                                                     {"M3", 6.8, 0.00110, 1.08, 1.58}}};

const std::array<ConductorMaterial, 3> CONDUCTOR_MATERIALS = {{{"C1", 9.2, 1.00, 1.00, 3.20},
                                                               {"C2", 10.5, 0.92, 0.98, 3.50},
                                                               {"C3", 3.5, 1.60, 0.55, 2.40}}};

const std::array<CoolingOption, 2> COOLING_OPTIONS = {{{"Q0", 0.0, 0.0},
                                                       {"Q1", 1500.0, 8.0}}};

const std::array<TransformerInstance, 3> TRANSFORMER_INSTANCES = {{{"A", 500.0, 1200.0, 450.0, 4.5, 5.5, 65.0, 2.20},
                                                                   {"B", 1500.0, 3000.0, 1100.0, 4.5, 6.5, 65.0, 3.00},
                                                                   {"C", 3000.0, 5200.0, 1900.0, 4.5, 7.2, 70.0, 3.80}}};

const std::array<CoreMaterial, 3> &getCoreMaterials()
{
  return CORE_MATERIALS;
}

const std::array<ConductorMaterial, 3> &getConductorMaterials()
{
  return CONDUCTOR_MATERIALS;
}

const std::array<CoolingOption, 2> &getCoolingOptions()
{
  return COOLING_OPTIONS;
}

const std::array<TransformerInstance, 3> &getTransformerInstances()
{
  return TRANSFORMER_INSTANCES;
}