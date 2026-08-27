#pragma once

#include <cstddef>
#include <vector>

#include "data.hpp"
#include "evaluator.hpp"

std::vector<EvaluationResult> optimizeDesigns(const TransformerInstance &instance, std::size_t limit = 10);
std::vector<EvaluationResult> getTopDesigns(const TransformerInstance &instance, std::size_t limit = 10);
EvaluationResult optimizeDesign(const TransformerInstance &instance);
