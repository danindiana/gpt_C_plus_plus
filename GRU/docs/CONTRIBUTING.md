# Contributing to GRU C++20

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## Code of Conduct

- Be respectful and constructive
- Focus on improving the educational value
- Welcome newcomers and questions

## Getting Started

### Prerequisites

- C++20 compliant compiler (GCC 13+, Clang 17+)
- CMake 3.20+
- Git

### Setting Up Development Environment

```bash
git clone https://github.com/danindiana/gpt_C_plus_plus.git
cd gpt_C_plus_plus/GRU
mkdir build && cd build
cmake ..
make
```

## Contribution Process

### 1. Fork and Branch

```bash
git checkout -b feature/your-feature-name
```

### 2. Make Changes

Follow the coding standards below.

### 3. Test

```bash
cd build
ctest --output-on-failure
```

### 4. Commit

Write clear, descriptive commit messages:

```
Add feature X to improve Y

- Detailed explanation
- Why this change is needed
- Any breaking changes
```

### 5. Pull Request

- Describe your changes
- Reference any related issues
- Ensure CI passes

## Coding Standards

### C++ Style

- **Standard**: C++20
- **Formatting**: 4 spaces, no tabs
- **Line length**: 100 characters max
- **Naming**:
  - Classes: `PascalCase`
  - Functions: `snake_case`
  - Variables: `snake_case`
  - Constants: `UPPER_CASE`

### Modern C++ Guidelines

```cpp
// Good: Use auto where type is obvious
auto result = compute_value();

// Good: Use const by default
const auto& get_data() const;

// Good: Use [[nodiscard]] for pure functions
[[nodiscard]] Vector transform(const Vector& v);

// Good: Use concepts
template<Numeric T>
T square(T x) { return x * x; }
```

### Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Brief description
 *
 * Detailed description with more context.
 *
 * @param x Description of parameter
 * @return Description of return value
 * @throws std::exception When this might throw
 */
```

### Error Handling

- Use exceptions for errors
- Validate inputs early
- Provide clear error messages

```cpp
if (x.size() != expected_size) {
    throw std::invalid_argument("Expected size " + std::to_string(expected_size));
}
```

## Testing

### Writing Tests

Add tests for all new functionality:

```cpp
TEST(feature_name) {
    // Arrange
    auto input = create_test_input();

    // Act
    auto result = process(input);

    // Assert
    ASSERT_EQ(result.size(), expected_size);
    ASSERT_NEAR(result[0], expected_value, 1e-6);
}
```

### Test Coverage

Aim for:
- All public API functions
- Edge cases (empty input, large input)
- Error conditions

## Areas for Contribution

### High Priority

- [ ] Backpropagation implementation
- [ ] Additional activation functions (LeakyReLU, ELU, SELU)
- [ ] Gradient checking utilities
- [ ] Performance benchmarks

### Medium Priority

- [ ] Bidirectional GRU
- [ ] Multi-layer (stacked) GRU
- [ ] Weight serialization (save/load)
- [ ] Visualization tools

### Low Priority (Nice to Have)

- [ ] GPU acceleration (CUDA)
- [ ] Python bindings (pybind11)
- [ ] Additional examples (time series, NLP)
- [ ] Jupyter notebook tutorials

## Performance Guidelines

- Profile before optimizing
- Document performance characteristics
- Benchmark critical paths

## Documentation

Update documentation when:
- Adding new features
- Changing API
- Fixing bugs with behavioral changes

Required documentation:
- README.md updates
- API reference (Doxygen)
- Examples if applicable

## Questions?

Open an issue with the `question` label.

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing! 🎉
