#pragma once

#include "models.hpp"
#include <array>

const std::array<CoreMaterial, 3> &getCoreMaterials();
const std::array<ConductorMaterial, 3> &getConductorMaterials();
const std::array<CoolingOption, 2> &getCoolingOptions();
const std::array<TransformerInstance, 3> &getTransformerInstances();