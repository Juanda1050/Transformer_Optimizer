#pragma once

#include "models.hpp"

EvaluationResult evaluateDesign(
    const TransformerInstance &instance,
    const Design &design);

bool isFeasible(
    const TransformerInstance &instance,
    const Design &design,
    const EvaluationResult &result);

bool isPotentiallyFeasible(
    const TransformerInstance &instance,
    const Design &design);