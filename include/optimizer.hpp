#pragma once

#include <cstddef>
#include <vector>

#include "data.hpp"
#include "evaluator.hpp"

EvaluationResult optimizeDesign(const TransformerInstance &instance);
std::vector<EvaluationResult> getTopDesigns(const TransformerInstance &instance, std::size_t limit = 10);
