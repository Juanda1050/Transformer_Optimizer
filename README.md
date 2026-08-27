# Transformer Optimizer

## Overview

Transformer Optimizer is a C++17 project that performs exhaustive search for transformer design alternatives and returns the top feasible options for each problem instance.

## Problem

For each transformer instance, the program evaluates combinations of:

- Core material
- Conductor material
- Cooling option
- Flux density
- Current density
- Number of layers
- Number of ducts

Each candidate is evaluated for cost and technical performance, then filtered by engineering constraints.

## Features

- Exhaustive search over the defined design space.
- Feasibility filtering in the evaluator.
- Top-N ranking of feasible solutions per instance.
- CSV export with ranked alternatives.
- Numerical validation against brute-force best candidate.

## Architecture

- include/models.hpp: data model for materials, instances, designs, and evaluation results.
- src/data.cpp: static datasets for materials and instances (A, B, C).
- src/evaluator.cpp: equations + feasibility checks.
- src/optimizer.cpp: exhaustive enumeration + ranked selection.
- src/reporting.cpp: benchmark summary, brute-force validation, CSV export.
- src/main.cpp: program entry point and execution pipeline.

## Requirements

- CMake 3.16+
- C++17 compiler (Clang, GCC, or MSVC)

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
cd build
./bin/transformer_optimizer
```

## Validation Test Plan

This section documents the minimum validation tests for delivery.

### Test 1 - Normal execution

Expected behavior:

- Instance A returns 10 ranked feasible solutions.
- Instance B returns 10 ranked feasible solutions.
- Instance C returns 10 ranked feasible solutions.

Verification command:

```bash
awk -F',' 'NR>1{count[$1]++} END{for (i in count) printf "%s %d\n", i, count[i]}' results.csv | sort
```

### Test 2 - All reported solutions are feasible

Expected behavior:

- Every exported row satisfies all feasibility constraints.

Constraints checked:

- Temperature limit
- Impedance range
- Diameter limit
- Flux density range
- Current density range
- Layers range
- Ducts range

### Test 3 - Ranking order

Expected behavior:

- Ranking is lexicographic with this priority:

1. Lower manufacturing cost
2. Lower total losses (only if cost is tied)
3. Lower diameter (only if cost and losses are tied)

### Test 4 - Invalid designs are rejected

Expected behavior:

- Evaluator returns `feasible=false` for designs violating any of:

- temperature
- impedance
- diameter
- flux density
- current density
- layers
- ducts

## Output

Execution prints:

- Best feasible reference values
- Top-10 table for Instance_A, Instance_B, and Instance_C
- Numerical validation result
- Benchmark summary per instance
- CSV export status

The ranked alternatives are written to:

- build/results.csv

The CSV includes rows for instances A, B, and C with columns:

- Instance, Rank, Core, Conductor, Cooling, B, J, Layers, Ducts, Cost, Losses, Temperature, Impedance, Diameter

## Optimization Strategy

Ranking priority is implemented in src/optimizer.cpp with comparator logic:

1. Lower manufacturing cost
2. If tied, lower total losses
3. If still tied, lower diameter

Implementation details:

- Comparator function `rankedBefore(...)` applies the exact lexicographic rule.
- `std::sort(...)` is called with that comparator in both incremental maintenance (`keepTopDesigns`) and final ordering (`optimizeDesigns`).
- Infeasible designs are excluded from top list insertion.

## Constraints

Feasibility is computed in src/evaluator.cpp (`isFeasible(...)`) and requires all of the following:

- Flux density in allowed range: [1.35, core.maxFluxDensity]
- Current density in allowed range: [1.60, conductor.maxCurrentDensity]
- Layers in [4, 16]
- Ducts in [0, 5]
- Temperature <= instance.maxTemperature
- Impedance in [instance.minImpedance, instance.maxImpedance]
- Diameter <= instance.maxDiameter

## Results

Validated on 2026-08-27 in macOS, from build directory.

Observed execution evidence:

- Test 1: `A 10`, `B 10`, `C 10`
- Test 2: `ALL_ROWS_FEASIBLE_BY_CONSTRAINTS=true` and `ALL_ROWS_VALID_BJ_LAYERS_DUCTS=true`
- Test 3: `ORDER_COST_LOSSES_DIAMETER_OK=true`
- Test 4: all injected invalid probes reported `feasible=false` (with baseline `feasible=true`)

## Project Structure

```text
include/
	data.hpp
	evaluator.hpp
	models.hpp
	optimizer.hpp
	reporting.hpp
src/
	data.cpp
	evaluator.cpp
	main.cpp
	optimizer.cpp
	reporting.cpp
build/
	transformer_optimizer
	results.csv
```

## Limitations

- Current output is CSV-first (no pretty table printer by default for top-10 list).
- Search space and formulas are deterministic and static; there is no stochastic optimizer.
- No unit-test target is currently defined in CMake.

## Author

Project developed for transformer design optimization study and validation.
