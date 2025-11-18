# Contributing to Compiler Optimization Tester

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## Code of Conduct

This project follows a code of conduct based on respect and professionalism. Please be respectful in all interactions.

## How to Contribute

### Reporting Bugs

Before creating bug reports, please check existing issues. When creating a bug report, include:

- **Clear title and description**
- **Steps to reproduce**
- **Expected vs actual behavior**
- **Environment details** (OS, compiler versions, etc.)
- **Code samples** if applicable

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. Include:

- **Clear use case**
- **Benefits to users**
- **Possible implementation approach**
- **Any alternatives considered**

### Pull Requests

1. **Fork the repository**
2. **Create a feature branch**
   ```bash
   git checkout -b feature/amazing-feature
   ```

3. **Make your changes**
   - Follow the code style
   - Add tests if applicable
   - Update documentation

4. **Commit your changes**
   ```bash
   git commit -m "feat: add amazing feature"
   ```

5. **Push to your fork**
   ```bash
   git push origin feature/amazing-feature
   ```

6. **Open a Pull Request**

## Development Setup

### Prerequisites

- GCC 13+ or Clang 18+
- CMake 3.25+
- Git
- (Optional) Csmith, YarpGen for testing

### Building

```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/gpt_C_plus_plus.git
cd gpt_C_plus_plus/compiler

# Create build directory
mkdir build && cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ..

# Build
cmake --build . --parallel

# Run
./compiler_tester --help
```

### Running Tests

```bash
# Unit tests (when implemented)
ctest

# Integration tests
./scripts/test_optimization.sh examples/seed_programs/
```

## Code Style

### C++ Style

We follow modern C++ best practices:

- **C++23 standard**
- **Use modern features**: ranges, concepts, `std::optional`, etc.
- **RAII** for resource management
- **Smart pointers** over raw pointers
- **`const` correctness**
- **`[[nodiscard]]`** for functions returning values that shouldn't be ignored

### Naming Conventions

```cpp
// Classes: PascalCase
class ProgramGenerator { };

// Functions/Methods: snake_case
auto generate_program() -> void;

// Variables: snake_case
int test_count = 0;

// Constants: SCREAMING_SNAKE_CASE
constexpr int MAX_ITERATIONS = 100;

// Member variables: trailing underscore
class Foo {
    int member_variable_;
};
```

### Formatting

Use the provided `.clang-format` configuration:

```bash
clang-format -i src/*.cpp include/**/*.hpp
```

### Documentation

- **Document public APIs** with Doxygen-style comments
- **Explain "why"** not "what" in comments
- **Update docs/** when changing functionality

Example:

```cpp
/// Generate a test program with the given configuration
///
/// @param config Generation configuration including seed and output directory
/// @return Generated program on success, std::nullopt on failure
///
/// @note This function may take several seconds for complex programs
[[nodiscard]] auto generate(const GeneratorConfig& config)
    -> std::optional<GeneratedProgram>;
```

## Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks
- `perf`: Performance improvements

### Examples

```bash
feat(generator): add YarpGen support

Implements YarpGen-based program generation as an alternative
to Csmith. YarpGen is better for C++ testing.

Closes #123
```

```bash
fix(oracle): correct binary size calculation on macOS

The stat() call was using Linux-specific flags. Updated to
use cross-platform approach.
```

## Testing

### Writing Tests

Add tests for new features:

```cpp
// tests/unit/test_generator.cpp
#include <gtest/gtest.h>
#include "compiler_tester/program_generator.hpp"

TEST(GeneratorTest, CreatesValidProgram) {
    auto generator = GeneratorFactory::create_csmith();
    GeneratorConfig config{.seed = 42};

    auto program = generator->generate(config);

    ASSERT_TRUE(program.has_value());
    EXPECT_TRUE(program->is_valid());
}
```

### Test Coverage

Aim for high test coverage, especially for:

- Core algorithms
- Error handling
- Edge cases
- Platform-specific code

## Adding New Features

### Adding a New Compiler

1. **Create compiler class**
   ```cpp
   // include/compiler_tester/my_compiler.hpp
   class MyCompiler : public CompilerInterface {
       // Implement interface
   };
   ```

2. **Implement compilation logic**
3. **Add factory method**
4. **Add tests**
5. **Update documentation**

### Adding a New Oracle

1. **Create oracle class**
   ```cpp
   // include/compiler_tester/my_oracle.hpp
   class MyOracle : public Oracle {
       // Implement interface
   };
   ```

2. **Implement detection logic**
3. **Add factory method**
4. **Add tests**
5. **Update documentation**

## Documentation

### Code Documentation

- Use Doxygen comments for public APIs
- Keep comments up-to-date with code changes
- Explain complex algorithms

### User Documentation

Update relevant docs when changing features:

- `README.md` - Overview and quick start
- `docs/ARCHITECTURE.md` - Architecture changes
- `docs/USAGE.md` - New features or usage
- `docs/API.md` - API changes

## Performance

### Profiling

Profile before optimizing:

```bash
# Build with profiling
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .

# Run with profiler
perf record ./compiler_tester --generate 100
perf report
```

### Benchmarking

Add benchmarks for performance-critical code:

```cpp
#include <benchmark/benchmark.h>

static void BM_ProgramGeneration(benchmark::State& state) {
    auto generator = GeneratorFactory::create_csmith();
    GeneratorConfig config{.seed = 42};

    for (auto _ : state) {
        auto program = generator->generate(config);
        benchmark::DoNotOptimize(program);
    }
}
BENCHMARK(BM_ProgramGeneration);
```

## Release Process

1. Update version in `CMakeLists.txt`
2. Update `CHANGELOG.md`
3. Create git tag
4. Push tag to trigger release workflow
5. Review and publish GitHub release

## Getting Help

- **Documentation**: Check `docs/` directory
- **Issues**: Search existing issues
- **Discussions**: Use GitHub Discussions
- **Contact**: Open an issue for questions

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to make compiler testing better!
