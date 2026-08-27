# Transformer Optimizer

C++17 application for evaluating and optimizing transformer design alternatives under a defined set of engineering and manufacturing constraints.

The application explores the available design space, evaluates each candidate, filters infeasible configurations, and returns the best alternatives according to a defined ranking strategy.

## Overview

A transformer design can be represented by a combination of material selections and operating parameters. The goal of this project is to systematically evaluate those combinations and identify feasible designs with the lowest manufacturing cost.

The optimizer considers parameters such as:

- Core material
- Conductor material
- Cooling configuration
- Flux density
- Current density
- Number of layers
- Number of ducts

Each candidate design is evaluated against the electrical, thermal, dimensional, and design constraints defined for the corresponding transformer instance.

## Features

- Exhaustive exploration of the defined design space.
- Centralized transformer design evaluation.
- Feasibility filtering based on engineering constraints.
- Deterministic multi-criteria ranking.
- Top-N selection of feasible alternatives.
- CSV export of optimization results.
- Numerical validation against a brute-force reference.

## Architecture

The application is organized around a simple optimization pipeline:

```text
Input Data
    │
    ▼
Design Enumeration
    │
    ▼
Design Evaluation
    │
    ├── Infeasible ──► Discard
    │
    ▼
Feasible Designs
    │
    ▼
Ranking & Top-N Selection
    │
    ▼
Results / CSV
```

### Main Components

#### `data`

Contains the static input data used by the application, including available materials and transformer instances.

#### `evaluator`

Calculates the relevant transformer characteristics and determines whether a design satisfies the required engineering constraints.

The evaluator is the central point for determining design feasibility.

#### `optimizer`

Generates the possible design configurations, evaluates them, and maintains the best feasible alternatives.

The implementation uses a bounded Top-N selection approach rather than retaining every feasible design.

#### `reporting`

Handles result presentation, CSV generation, and numerical validation of the optimization process.

#### `main`

Application entry point connecting the data, optimization, evaluation, and reporting components.

## Project Structure

```text
Transformer_Optimizer/
├── include/
│   ├── data.hpp
│   ├── evaluator.hpp
│   ├── models.hpp
│   ├── optimizer.hpp
│   └── reporting.hpp
│
├── src/
│   ├── data.cpp
│   ├── evaluator.cpp
│   ├── main.cpp
│   ├── optimizer.cpp
│   └── reporting.cpp
│
├── CMakeLists.txt
└── README.md
```

## Optimization Strategy

The current implementation uses exhaustive enumeration because the design space is finite and deterministic.

For each transformer instance, the optimizer:

1. Generates the possible design configurations.
2. Evaluates each configuration.
3. Discards infeasible designs.
4. Keeps the best feasible candidates.
5. Orders the final results according to the optimization criteria.

The ranking follows a lexicographic strategy:

1. Lower manufacturing cost.
2. Lower total losses when cost is equal.
3. Lower diameter when cost and losses are equal.

This produces deterministic and reproducible results.

## Engineering Constraints

A design is considered feasible only when all required conditions are satisfied.

The evaluator checks:

- Flux density range.
- Current density range.
- Number of layers.
- Number of ducts.
- Maximum operating temperature.
- Impedance range.
- Maximum diameter.

The applicable limits depend on the transformer instance and selected materials.

## Results

The optimizer was validated using three transformer instances:

| Instance | Ranked Alternatives | Feasibility | Ranking |
| -------- | ------------------- | ----------- | ------- |
| A        | 10                  | Valid       | Valid   |
| B        | 10                  | Valid       | Valid   |
| C        | 10                  | Valid       | Valid   |

The reported alternatives satisfy the defined engineering constraints and are ordered according to the optimization criteria.

The optimization results are also compared against a brute-force reference to verify the selected best candidates.

## Output

The application provides:

- Best feasible design information.
- Ranked alternatives for each transformer instance.
- Optimization and validation information.
- CSV output containing the ranked designs.

The generated CSV contains:

```text
Instance
Rank
Core
Conductor
Cooling
B
J
Layers
Ducts
Cost
Losses
Temperature
Impedance
Diameter
```

The output file is generated as:

```text
build/results.csv
```

## Validation

The optimization process includes numerical validation against a brute-force reference implementation.

Validation covers:

- Feasibility of reported designs.
- Validity of design parameter ranges.
- Correct ranking order.
- Rejection of invalid configurations.
- Consistency between the optimizer and brute-force reference.

## Requirements

- CMake 3.16 or newer
- C++17 compatible compiler
  - Clang
  - GCC
  - MSVC

## Build

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/bin/transformer_optimizer
```

If the executable is generated directly inside the build directory:

```bash
./build/transformer_optimizer
```

## Technical Approach

The project separates the optimization process from the engineering calculations.

The main responsibilities are divided between:

- **Data** — defines the available inputs.
- **Evaluator** — calculates design characteristics and determines feasibility.
- **Optimizer** — searches the design space and selects the best alternatives.
- **Reporting** — presents and exports the results.

This separation allows the evaluation equations and optimization strategy to evolve independently from the application's entry point and reporting layer.

## Release

Current stable release:

**v1.0.1**
