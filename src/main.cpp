#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#include <sys/stat.h>
#define GET_CURRENT_DIR _getcwd
#define MAKE_DIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define GET_CURRENT_DIR getcwd
#define MAKE_DIR(path) mkdir(path, 0777)
#endif

#include "data.hpp"
#include "evaluator.hpp"
#include "optimizer.hpp"

namespace
{
  bool validateOptimizationAgainstBruteforce()
  {
    const auto &instances = getTransformerInstances();

    for (const auto &instance : instances)
    {
      EvaluationResult optimum = optimizeDesign(instance);
      EvaluationResult bruteForce{};
      bool found = false;

      for (const auto &core : getCoreMaterials())
      {
        for (const auto &conductor : getConductorMaterials())
        {
          for (const auto &cooling : getCoolingOptions())
          {
            for (double fluxDensity = 1.35; fluxDensity <= core.maxFluxDensity + 1e-9; fluxDensity += 0.05)
            {
              for (double currentDensity = 1.60; currentDensity <= conductor.maxCurrentDensity + 1e-9; currentDensity += 0.10)
              {
                for (int layers = 4; layers <= 16; ++layers)
                {
                  for (int ducts = 0; ducts <= 5; ++ducts)
                  {
                    Design design{&core, &conductor, &cooling, fluxDensity, currentDensity, layers, ducts};
                    EvaluationResult candidate = evaluateDesign(instance, design);

                    if (candidate.feasible && (!found ||
                                               candidate.manufacturingCost < bruteForce.manufacturingCost ||
                                               (std::abs(candidate.manufacturingCost - bruteForce.manufacturingCost) < 1e-9 &&
                                                (candidate.totalLosses < bruteForce.totalLosses ||
                                                 (std::abs(candidate.totalLosses - bruteForce.totalLosses) < 1e-9 &&
                                                  candidate.diameter < bruteForce.diameter)))))
                    {
                      bruteForce = candidate;
                      found = true;
                    }
                  }
                }
              }
            }
          }
        }
      }

      if (!found || !optimum.feasible || !bruteForce.feasible)
      {
        std::cout << "Validation failed for instance " << instance.id << '\n';
        return false;
      }

      const double deltaCost = std::abs(optimum.manufacturingCost - bruteForce.manufacturingCost);
      const double deltaLoss = std::abs(optimum.totalLosses - bruteForce.totalLosses);
      const double deltaDiameter = std::abs(optimum.diameter - bruteForce.diameter);

      if (deltaCost > 1e-6 || deltaLoss > 1e-6 || deltaDiameter > 1e-6)
      {
        std::cout << "Mismatch for instance " << instance.id << '\n';
        return false;
      }
    }

    return true;
  }

  struct BenchmarkSummary
  {
    int totalEvaluated = 0;
    int feasibleCount = 0;
    double bestCost = std::numeric_limits<double>::infinity();
    double elapsedMs = 0.0;
  };

  BenchmarkSummary benchmarkInstance(const TransformerInstance &instance)
  {
    BenchmarkSummary summary{};
    const auto start = std::chrono::steady_clock::now();

    for (const auto &core : getCoreMaterials())
    {
      for (const auto &conductor : getConductorMaterials())
      {
        for (const auto &cooling : getCoolingOptions())
        {
          for (double fluxDensity = 1.35; fluxDensity <= core.maxFluxDensity + 1e-9; fluxDensity += 0.05)
          {
            for (double currentDensity = 1.60; currentDensity <= conductor.maxCurrentDensity + 1e-9; currentDensity += 0.10)
            {
              for (int layers = 4; layers <= 16; ++layers)
              {
                for (int ducts = 0; ducts <= 5; ++ducts)
                {
                  ++summary.totalEvaluated;
                  Design design{&core, &conductor, &cooling, fluxDensity, currentDensity, layers, ducts};
                  const EvaluationResult candidate = evaluateDesign(instance, design);

                  if (candidate.feasible)
                  {
                    ++summary.feasibleCount;
                    if (candidate.manufacturingCost < summary.bestCost)
                    {
                      summary.bestCost = candidate.manufacturingCost;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    const auto end = std::chrono::steady_clock::now();
    summary.elapsedMs = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.0;
    return summary;
  }

  std::string normalizeDirectory(const std::string &path)
  {
    if (path.empty())
      return path;

    std::string normalized = path;
    while (!normalized.empty() && (normalized.back() == '/' || normalized.back() == '\\'))
      normalized.pop_back();
    return normalized;
  }

  std::string resolveProjectOutputPath()
  {
    char buffer[4096];
    if (GET_CURRENT_DIR(buffer, sizeof(buffer)) == nullptr)
      return "results.csv";

    std::string currentDir = normalizeDirectory(buffer);
    const std::string buildMarkers[] = {"/build", "\\build", "/Debug", "\\Debug", "/Release", "\\Release", "/bin", "\\bin"};

    for (const std::string &marker : buildMarkers)
    {
      const auto pos = currentDir.rfind(marker);
      if (pos != std::string::npos && pos + marker.size() == currentDir.size())
      {
        return currentDir.substr(0, pos) + "/results.csv";
      }
    }

    return currentDir + "/results.csv";
  }

  std::string shellQuote(const std::string &value)
  {
    std::string quoted = "'";
    for (char ch : value)
    {
      if (ch == '\'')
      {
        quoted += "'\"'\"'";
      }
      else
      {
        quoted += ch;
      }
    }
    quoted += "'";
    return quoted;
  }

  bool writeResultsWorkbook()
  {
    const auto &instances = getTransformerInstances();
    const std::string outputPath = resolveProjectOutputPath();
    const std::string workbookPath = outputPath.substr(0, outputPath.find_last_of("/\\")) + "/results.xlsx";
    const std::string jsonPath = outputPath.substr(0, outputPath.find_last_of("/\\")) + "/results_workbook.json";

    std::ofstream jsonFile(jsonPath);
    if (!jsonFile)
      return false;

    jsonFile << "[\n";
    for (std::size_t idx = 0; idx < instances.size(); ++idx)
    {
      const auto &instance = instances[idx];
      const auto topDesigns = getTopDesigns(instance, 10);

      jsonFile << "  {\n";
      jsonFile << "    \"instance\": \"" << instance.id << "\",\n";
      jsonFile << "    \"rows\": [\n";
      jsonFile << "      [\"Instancia\", \"Clasificacion\", \"MaterialNucleo\", \"MaterialConductor\", \"Enfriamiento\", \"B\", \"J\", \"Capas\", \"Ductos\", \"CostoManufactura\", \"PerdidasTotales\", \"Temperatura\", \"Impedancia\", \"Diametro\"]\n";

      for (std::size_t i = 0; i < topDesigns.size(); ++i)
      {
        const auto &design = topDesigns[i];
        jsonFile << "      , [\"" << instance.id << "\", "
                 << (i + 1) << ", \"" << design.design.core->id << "\", \"" << design.design.conductor->id << "\", \"" << design.design.cooling->id << "\", "
                 << std::fixed << std::setprecision(2) << design.design.fluxDensity << ", "
                 << design.design.currentDensity << ", "
                 << design.design.layers << ", "
                 << design.design.ducts << ", "
                 << design.manufacturingCost << ", "
                 << design.totalLosses << ", "
                 << design.temperature << ", "
                 << design.impedance << ", "
                 << design.diameter << "]\n";
      }

      jsonFile << "    ]\n";
      jsonFile << "  }";
      if (idx + 1 != instances.size())
        jsonFile << ",";
      jsonFile << "\n";
    }
    jsonFile << "]\n";
    jsonFile.close();

    std::string pythonScript = R"(import json, sys, zipfile, os
from xml.sax.saxutils import escape

xlsx_path, json_path = sys.argv[1], sys.argv[2]
with open(json_path, 'r', encoding='utf-8') as f:
    data = json.load(f)


def col_name(n):
    name = ''
    while n:
        n, r = divmod(n - 1, 26)
        name = chr(65 + r) + name
    return name


def cell_ref(r, c):
    return f'{col_name(c)}{r}'


def xml_cell(row_idx, col_idx, value):
    ref = cell_ref(row_idx, col_idx)
    if isinstance(value, str):
        return f'<c r="{ref}" t="inlineStr"><is><t>{escape(str(value))}</t></is></c>'
    return f'<c r="{ref}" t="n"><v>{value}</v></c>'


def sheet_xml(rows):
    xml = '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"><sheetData>'
    for r_idx, row in enumerate(rows, start=1):
        xml += f'<row r="{r_idx}">'
        for c_idx, value in enumerate(row, start=1):
            xml += xml_cell(r_idx, c_idx, value)
        xml += '</row>'
    xml += '</sheetData></worksheet>'
    return xml

sheet_entries = []
rels = []
for index, entry in enumerate(data, start=1):
    instance_id = str(entry['instance'])
    sheet_name = f"Instance {instance_id}"
    rows = entry['rows']
    sheet_entries.append(f'<sheet name="{escape(sheet_name)}" sheetId="{index}" r:id="rId{index}"/>')
    rels.append(f'<Relationship Id="rId{index}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet{index}.xml"/>')
    with open(f'/tmp/sheet{index}.xml', 'w', encoding='utf-8') as out:
        out.write(sheet_xml(rows))

content_types = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml" ContentType="application/xml"/>
  <Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
  <Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
  <Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>
'''
for i in range(1, len(data) + 1):
    content_types += f'  <Override PartName="/xl/worksheets/sheet{i}.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>\n'
content_types += '</Types>'

workbook_xml = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
  <sheets>''' + ''.join(sheet_entries) + '''</sheets>
</workbook>'''

workbook_rels = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
''' + ''.join(rels) + '''
</Relationships>'''

root_rels = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>'''

doc_props_core = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:dcmitype="http://purl.org/dc/dcmitype/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <dc:title>Transformer Optimizer Results</dc:title>
  <dc:creator>Transformer Optimizer</dc:creator>
  <cp:lastModifiedBy>Transformer Optimizer</cp:lastModifiedBy>
</cp:coreProperties>'''

doc_props_app = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"><Application>Transformer Optimizer</Application></Properties>'''

with zipfile.ZipFile(xlsx_path, 'w', compression=zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', content_types)
    z.writestr('_rels/.rels', root_rels)
    z.writestr('docProps/core.xml', doc_props_core)
    z.writestr('docProps/app.xml', doc_props_app)
    z.writestr('xl/workbook.xml', workbook_xml)
    z.writestr('xl/_rels/workbook.xml.rels', workbook_rels)
    for i in range(1, len(data) + 1):
        z.write(f'/tmp/sheet{i}.xml', f'xl/worksheets/sheet{i}.xml')
        os.remove(f'/tmp/sheet{i}.xml')

os.remove(json_path)
)";

    std::string command = "python3 -c " + shellQuote(pythonScript) + " \"" + workbookPath + "\" \"" + jsonPath + "\"";
    const int exitCode = std::system(command.c_str());
    if (exitCode != 0)
      return false;

    return true;
  }
}

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

  const EvaluationResult best = optimizeDesign(instances[0]);

  std::cout << std::fixed << std::setprecision(6);

  std::cout << "Reference total losses: "
            << result.totalLosses << '\n';

  std::cout << "Best feasible total losses: "
            << best.totalLosses << '\n';

  std::cout << "Best feasible manufacturing cost: "
            << best.manufacturingCost << '\n';

  std::cout << "Best feasible temperature: "
            << best.temperature << '\n';

  std::cout << "Best feasible impedance: "
            << best.impedance << '\n';

  std::cout << "Best feasible diameter: "
            << best.diameter << '\n';

  std::cout << "Best feasible design: "
            << best.design.core->id << " / "
            << best.design.conductor->id << " / "
            << best.design.cooling->id << " / flux="
            << best.design.fluxDensity << " / current="
            << best.design.currentDensity << " / layers="
            << best.design.layers << " / ducts="
            << best.design.ducts << '\n';

  std::cout << "Feasible: "
            << std::boolalpha
            << best.feasible << '\n';

  const bool validationOk = validateOptimizationAgainstBruteforce();
  std::cout << "Numerical validation: "
            << std::boolalpha
            << validationOk << '\n';

  std::cout << "\nBenchmark summary:\n";
  for (const auto &instance : instances)
  {
    const BenchmarkSummary summary = benchmarkInstance(instance);
    std::cout << "Instance " << instance.id
              << ": evaluated=" << summary.totalEvaluated
              << ", feasible=" << summary.feasibleCount
              << ", bestCost=" << summary.bestCost
              << ", elapsedMs=" << summary.elapsedMs << '\n';
  }

  const bool workbookOk = writeResultsWorkbook();
  std::cout << "Workbook results: " << std::boolalpha << workbookOk << '\n';

  return validationOk && workbookOk ? 0 : 1;
}