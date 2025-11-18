# Contributing to Fortuna

Thank you for your interest in contributing to Fortuna! This document provides guidelines and instructions for contributing.

## Code of Conduct

This project adheres to a code of conduct. By participating, you are expected to uphold this code. Please be respectful and constructive in your interactions.

## Getting Started

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- CMake 3.20+
- Git

### Setting Up Development Environment

1. Fork the repository
2. Clone your fork:
   ```bash
   git clone https://github.com/YOUR_USERNAME/gpt_C_plus_plus.git
   cd gpt_C_plus_plus/FORTUNA
   ```

3. Create a development branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```

4. Build the project:
   ```bash
   mkdir build && cd build
   cmake -DFORTUNA_BUILD_EXAMPLES=ON -DFORTUNA_BUILD_TESTS=ON ..
   cmake --build .
   ```

## How to Contribute

### Reporting Bugs

If you find a bug, please create an issue with:
- Clear description of the problem
- Steps to reproduce
- Expected vs actual behavior
- System information (OS, compiler version, etc.)
- Code sample if applicable

### Suggesting Features

Feature requests are welcome! Please create an issue with:
- Clear description of the feature
- Use cases and benefits
- Proposed API (if applicable)
- Any implementation ideas

### Pull Requests

1. **Create an issue first** for major changes to discuss the approach
2. **Write clear commit messages** following conventional commits format
3. **Add tests** for new functionality
4. **Update documentation** including API docs and examples
5. **Ensure all tests pass** before submitting
6. **Follow the coding style** (see below)

## Coding Standards

### C++ Style Guide

- Follow modern C++20 best practices
- Use meaningful variable and function names
- Prefer `const` and `constexpr` where applicable
- Use RAII for resource management
- Avoid raw pointers; use smart pointers when needed

### Code Formatting

```cpp
// Class names: PascalCase
class MyClassName {
public:
    // Public methods: snake_case
    void my_public_method();

private:
    // Private members: snake_case with trailing underscore
    int my_member_variable_;
};

// Namespace: lowercase
namespace fortuna::core {
    // ...
}

// Constants: UPPER_SNAKE_CASE
constexpr int MAX_ITERATIONS = 100;

// Template parameters: PascalCase
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;
```

### Documentation

- Use Doxygen-style comments for all public APIs
- Include `@brief` description
- Document parameters with `@param`
- Document return values with `@return`
- Add usage examples for complex APIs

Example:
```cpp
/**
 * @brief Performs forward pass through the layer
 *
 * @param inputs Input vector of size input_size()
 * @return RealVector Output after applying weights, biases, and activation
 *
 * @throws std::invalid_argument if input size doesn't match
 *
 * Example:
 * @code
 * Layer layer(2, 3, ActivationType::RELU);
 * RealVector input = {1.0, 2.0};
 * RealVector output = layer.forward(input);
 * @endcode
 */
[[nodiscard]] RealVector forward(std::span<const Real> inputs) const;
```

### Testing

- Write unit tests for new features
- Aim for high code coverage
- Test edge cases and error conditions
- Use descriptive test names

### Commit Messages

Follow the Conventional Commits format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

Example:
```
feat(bayesian): implement ADVI method

Add Automatic Differentiation Variational Inference as a new
Bayesian method. Includes:
- ADVI training implementation
- Variational parameters tracking
- ELBO computation

Closes #42
```

## Project Structure

### Adding New Features

#### New Activation Function

1. Add enum value to `ActivationType` in `include/fortuna/core/types.hpp`
2. Implement function in `include/fortuna/utils/activations.hpp`
3. Add implementation in `src/utils/activations.cpp`
4. Update `apply()` method to handle new type
5. Add tests and documentation

#### New Bayesian Method

1. Add enum value to `BayesianMethod` in `include/fortuna/core/types.hpp`
2. Add training method declaration in `include/fortuna/bayesian/prob_classifier.hpp`
3. Implement training logic in `src/bayesian/prob_classifier.cpp`
4. Update `train()` switch statement
5. Add example program demonstrating the method
6. Update documentation and README

#### New Layer Type

1. Create header in `include/fortuna/core/`
2. Implement in `src/core/`
3. Add factory method to `Model` class
4. Add examples and tests
5. Update documentation

## Development Workflow

### Branch Strategy

- `main` - Stable releases
- `develop` - Integration branch
- `feature/*` - New features
- `bugfix/*` - Bug fixes
- `docs/*` - Documentation updates

### Release Process

1. Update version in `CMakeLists.txt`
2. Update `CHANGELOG.md`
3. Update documentation
4. Tag release: `git tag -a v1.x.x -m "Release v1.x.x"`
5. Push tags: `git push --tags`

## Performance Considerations

- Profile code before optimizing
- Benchmark performance-critical sections
- Prefer algorithms with better complexity
- Consider cache-friendly data structures
- Use compiler optimizations appropriately

## Questions?

If you have questions about contributing:
- Open an issue with the `question` label
- Reach out to the maintainers
- Check existing issues and documentation

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to Fortuna! 🎉
