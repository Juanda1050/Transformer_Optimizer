# Transformer Optimizer

Proyecto C++ para explorar el espacio de diseño de transformadores y seleccionar las mejores alternativas factibles según costo, pérdidas y tamaño.

## ¿Qué hace?

La aplicación:

- evalúa combinaciones de núcleo, conductor, enfriamiento, flujo, corriente, capas y ductos,
- descarta opciones no válidas por restricciones técnicas,
- ordena las soluciones factibles con la prioridad de menor costo, luego pérdidas y finalmente diámetro,
- exporta las 10 mejores alternativas por instancia en un archivo CSV.

## Requisitos

- CMake 3.16+
- compilador C++17 (Clang, GCC o MSVC)

## Uso

### macOS

Desde la carpeta de release:

```bash
cd release/macos
./transformer_optimizer
```

### Windows

Desde la carpeta de release/windows:

```powershell
cd release/windows
./transformer_optimizer.exe
```

Si necesitas recompilarlo, usa:

```bash
cmake -S . -B build
cmake --build build
```

## Salida

Al ejecutarse, el programa genera una carpeta `results` con un archivo CSV por instancia:

- `results/Instance_A.csv`
- `results/Instance_B.csv`
- `results/Instance_C.csv`

Cada archivo contiene las 10 mejores alternativas de esa instancia, listas para abrirse en Excel como una hoja independiente o como un CSV por instancia.

## Estructura principal

- `src/main.cpp`: punto de entrada y salida del programa
- `src/optimizer.cpp`: búsqueda exhaustiva y ordenación de soluciones
- `src/evaluator.cpp`: validación y cálculo de rendimiento
- `src/data.cpp`: datos del problema
- `include/`: interfaces públicas
