# Usage Guide

## Table of Contents

- [Quick Start](#quick-start)
- [Command Line Interface](#command-line-interface)
- [Configuration](#configuration)
- [Running Tests](#running-tests)
- [Interpreting Results](#interpreting-results)
- [Advanced Usage](#advanced-usage)
- [Troubleshooting](#troubleshooting)

## Quick Start

### Basic Testing

```bash
# Build the project
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel

# Run with default settings
./compiler_tester --generate 5 --compiler both

# Check the results
ls ../output/reports/
```

### Using Bash Scripts

```bash
# Test existing programs
cd scripts
./test_optimization.sh ../examples/seed_programs

# Enable verbose output
./test_optimization.sh -v ../examples/seed_programs

# Custom optimization level
./test_optimization.sh -o O2 ../examples/seed_programs
```

## Command Line Interface

### Main Executable Options

```
compiler_tester [options]

Options:
  --help, -h              Show help message
  --version, -v           Show version information
  --generate <count>      Number of test programs to generate (default: 10)
  --compiler <name>       Compiler to test: gcc, clang, or both (default: both)
  --output-dir <path>     Output directory (default: ./output)
  --optimization <level>  Optimization level: O0, O1, O2, O3, Os, Ofast (default: O3)
```

### Examples

```bash
# Test with GCC only, generate 20 programs
./compiler_tester --generate 20 --compiler gcc

# Test with both compilers at O2
./compiler_tester --generate 10 --compiler both --optimization O2

# Custom output directory
./compiler_tester --generate 5 --output-dir /tmp/compiler_tests
```

### Bash Script Options

```
test_optimization.sh [options] [program_directory]

Options:
  -h, --help              Show help message
  -v, --verbose           Enable verbose output
  -o, --optimization LEVEL Set optimization level (O0, O1, O2, O3, Os, Ofast)
  -t, --threshold VALUE   Set binary size threshold (default: 0.05)
  --gcc PATH              Path to GCC compiler
  --clang PATH            Path to Clang compiler
  --output DIR            Output directory for results
```

### Script Examples

```bash
# Test with verbose logging
./test_optimization.sh -v ./examples

# Change threshold to 10%
./test_optimization.sh -t 0.10 ./examples

# Use custom compiler
./test_optimization.sh --gcc /usr/local/bin/gcc-14 ./examples
```

## Configuration

### Environment Variables

```bash
# Set compiler paths
export GCC=/usr/local/bin/gcc-14
export CLANG=/opt/llvm/bin/clang

# Set default optimization level
export OPTIMIZATION_LEVEL=O3

# Set binary size threshold (5% = 0.05)
export BINARY_SIZE_THRESHOLD=0.05

# Enable verbose logging
export VERBOSE=1

# Run tests
./compiler_tester --generate 10
```

### Configuration Files

Create a `config.sh` file:

```bash
#!/bin/bash

# Compiler Configuration
export GCC=/usr/bin/gcc-13
export CLANG=/usr/bin/clang-18

# Testing Configuration
export OPTIMIZATION_LEVEL=O3
export BINARY_SIZE_THRESHOLD=0.05
export OUTPUT_DIR=./results

# Program Generation
export CSMITH_PATH=/usr/local/bin/csmith
export YARPGEN_PATH=/usr/local/bin/yarpgen
```

Source it before running:

```bash
source config.sh
./compiler_tester --generate 20
```

## Running Tests

### Test Workflow

1. **Generate Test Programs**
   ```bash
   ./compiler_tester --generate 10 --compiler gcc
   ```

2. **Review Generated Programs**
   ```bash
   ls output/generated/
   ```

3. **Check Results**
   ```bash
   cat output/reports/report_*.md
   ```

### Testing Existing Programs

```bash
# Create test programs directory
mkdir examples/seed_programs

# Add your C files
cp my_test.c examples/seed_programs/

# Run tests
./scripts/test_optimization.sh examples/seed_programs
```

### Batch Testing

```bash
# Test multiple optimization levels
for opt in O0 O1 O2 O3 Os Ofast; do
    echo "Testing with -${opt}"
    ./compiler_tester --generate 5 --optimization ${opt} \
        --output-dir "./output_${opt}"
done
```

### Parallel Testing

```bash
# Run multiple instances in parallel
./compiler_tester --generate 10 --output-dir ./output_gcc_O2 \
    --compiler gcc --optimization O2 &

./compiler_tester --generate 10 --output-dir ./output_gcc_O3 \
    --compiler gcc --optimization O3 &

wait
echo "All tests complete"
```

## Interpreting Results

### Report Structure

Reports are generated in `output/reports/` with the following structure:

```markdown
# Compiler Optimization Testing Report

## Summary
- Total Programs Tested: 50
- Inconsistencies Found: 3
- Inconsistency Rate: 6.0%
- Duration: 245 seconds

## Detected Inconsistencies

### Issue #1
Type: Binary Size Increase
Compiler: GCC 13.3.0
Original: test_1.c
Refined: test_1_refined.c
Severity: 12.5%
Description: Refined program binary is 12.5% larger...
```

### Understanding Inconsistencies

#### Binary Size Increase

```
Original binary: 15,234 bytes
Refined binary:  17,145 bytes
Increase:        12.5%
```

**Interpretation**: The refined program (with optimization hints) produced a larger binary, indicating the compiler failed to leverage the additional information.

#### Dead Code Elimination (DCE) Regression

```
DCE markers in original:  5
DCE markers in refined:   8
```

**Interpretation**: The refined program has more dead code markers, suggesting the compiler eliminated less dead code.

#### Value Range Degradation

```
Optimizable range checks in original:  10
Optimizable range checks in refined:   7
```

**Interpretation**: The compiler performed fewer value range optimizations on the refined program.

### Exit Codes

| Code | Meaning |
|------|---------|
| 0 | All tests passed, no inconsistencies found |
| 1 | Error occurred (missing prerequisites, invalid config, etc.) |
| 2 | Tests completed but inconsistencies were detected |

## Advanced Usage

### Custom Program Generation

```cpp
#include "compiler_tester/program_generator.hpp"

using namespace compiler_tester;

GeneratorConfig config{
    .output_dir = "./my_programs",
    .seed = 12345,
    .deterministic = true,
    .max_depth = 15,
    .max_functions = 25
};

auto generator = GeneratorFactory::create_csmith();
auto programs = generator->generate_batch(config, 100);
```

### Custom Information Injection

```cpp
#include "compiler_tester/information_injector.hpp"

using namespace compiler_tester;

// Create custom injection configuration
InjectorConfig config{
    .use_builtin_unreachable = true,
    .use_cpp23_assume = false,
    .preserve_formatting = true,
    .add_comments = true
};

InformationInjector injector(config);

// Inject custom information
ProgramInformation info;
info.dead_code_locations.push_back({
    .line_number = 42,
    .column = 5,
    .function_name = "process_data",
    .condition = "x > 0 && x < 100"
});

auto result = injector.inject_all("program.c", info);
```

### Creating Custom Oracles

```cpp
#include "compiler_tester/oracle.hpp"

class MyCustomOracle : public Oracle {
public:
    auto check(const CompilationResult& original,
               const CompilationResult& refined,
               const OracleConfig& config)
        -> std::optional<OptimizationInconsistency> override {

        // Custom detection logic
        if (custom_condition(original, refined)) {
            return OptimizationInconsistency{
                .type = InconsistencyType::Custom,
                .description = "Custom issue detected"
            };
        }

        return std::nullopt;
    }

    auto name() const noexcept -> std::string_view override {
        return "MyCustomOracle";
    }
};
```

### Integration with CI/CD

#### GitHub Actions

```yaml
name: Compiler Testing

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y gcc-13 clang-18 cmake

      - name: Build
        run: |
          mkdir build && cd build
          cmake -DCMAKE_BUILD_TYPE=Release ..
          cmake --build . --parallel

      - name: Run tests
        run: |
          cd build
          ./compiler_tester --generate 50 --compiler both

      - name: Upload results
        uses: actions/upload-artifact@v3
        with:
          name: test-results
          path: output/reports/
```

#### GitLab CI

```yaml
test:
  image: gcc:13
  script:
    - apt-get update && apt-get install -y clang-18 cmake
    - mkdir build && cd build
    - cmake -DCMAKE_BUILD_TYPE=Release ..
    - cmake --build . --parallel
    - ./compiler_tester --generate 50 --compiler both
  artifacts:
    paths:
      - output/reports/
```

### Automated Bug Reporting

```cpp
#include "compiler_tester/reporter.hpp"

using namespace compiler_tester;

Reporter reporter;

// Detect inconsistency
if (auto issue = oracle->check(original, refined, config)) {
    reporter.add_inconsistency(*issue);

    // Minimize test case
    auto minimized = reporter.minimize_test_case(
        original_file,
        *issue
    );

    // Generate bug report
    auto bug_report = reporter.generate_bug_report(
        *issue,
        *minimized
    );

    // Save as markdown for GitHub issue
    std::ofstream("bug_report.md") << bug_report.to_markdown();
}
```

## Troubleshooting

### Common Issues

#### No Program Generator Available

```
Error: No program generator available
```

**Solution**: Install Csmith or YarpGen:

```bash
# Ubuntu/Debian
sudo apt-get install csmith

# From source
git clone https://github.com/csmith-project/csmith.git
cd csmith
cmake . && make && sudo make install
```

#### Compiler Not Found

```
Error: GCC not found at /usr/bin/gcc
```

**Solution**: Install compiler or update path:

```bash
# Install GCC
sudo apt-get install gcc-13

# Or set custom path
export GCC=/usr/local/bin/gcc-13
```

#### Compilation Failures

```
Warning: Failed to compile original program
```

**Solution**: Check generated program:

```bash
# Manually compile to see errors
gcc -O3 output/generated/program_123.c -o test

# Enable verbose mode
./compiler_tester -v --generate 5
```

#### Permission Denied

```
Error: Permission denied: ./output
```

**Solution**: Create directory or change permissions:

```bash
mkdir -p output
chmod 755 output

# Or use different directory
./compiler_tester --output-dir /tmp/tests
```

### Debug Mode

Enable detailed logging:

```bash
# C++ executable
./compiler_tester --verbose --generate 5

# Bash script
VERBOSE=1 ./scripts/test_optimization.sh examples/
```

### Getting Help

```bash
# Show help
./compiler_tester --help
./scripts/test_optimization.sh --help

# Check version
./compiler_tester --version

# List available compilers
./scripts/utils.sh
source ./scripts/utils.sh
find_compilers
```

---

For more information, see:
- [Architecture Documentation](ARCHITECTURE.md)
- [API Documentation](API.md)
- [GitHub Issues](https://github.com/danindiana/gpt_C_plus_plus/issues)
