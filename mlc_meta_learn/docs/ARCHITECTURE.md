# MLC Architecture Documentation

## Overview

The MLC (Meta-Learning for Compositionality) framework is designed with modularity, extensibility, and performance in mind. This document describes the architectural decisions and design patterns used throughout the codebase.

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     MLC Framework                           │
├─────────────────────────────────────────────────────────────┤
│  Data Layer          │  Neural Network Layer  │  Training   │
│                      │                        │  Layer      │
│  ┌──────────┐       │  ┌──────────────┐     │  ┌────────┐ │
│  │ Example  │       │  │ Tensor       │     │  │Trainer │ │
│  │ Episode  │  ───> │  │ Attention    │ ──> │  │        │ │
│  │Generator │       │  │ MLCNetwork   │     │  │        │ │
│  └──────────┘       │  └──────────────┘     │  └────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. Data Structures

#### Example (`include/mlc/example.hpp`)

The `Example` class represents a single instruction-output pair.

**Design Decisions:**
- Immutable by default with explicit setters
- Value semantics (copyable and movable)
- Built-in validation
- Stream output support for debugging

**Usage Pattern:**
```cpp
Example ex("jump twice", {"circle", "circle"});
assert(ex.is_valid());
```

#### Episode (`include/mlc/episode.hpp`)

The `Episode` class encapsulates a meta-learning task with study and query examples.

**Design Decisions:**
- Composition over inheritance
- Range-based access to examples
- Validation ensures data integrity
- Supports both individual and batch operations

**Key Methods:**
- `add_study_example()` - Add context
- `set_query_example()` - Set target
- `is_valid()` - Validate structure

### 2. Neural Network Components

#### Tensor (`include/mlc/tensor.hpp`)

A lightweight tensor implementation for basic neural network operations.

**Design Philosophy:**
- Simplicity over feature completeness
- Explicit shape management
- Basic operations (add, multiply, etc.)
- Activation functions as free functions

**Why Not Use External Libraries?**
- Reduced dependencies
- Educational clarity
- Full control over implementation
- Easy to replace with Eigen/Armadillo if needed

**Future Improvements:**
- GPU support via CUDA/HIP
- Automatic differentiation
- More efficient memory management
- SIMD optimizations

#### Attention Mechanism (`include/mlc/attention.hpp`)

Implements multi-head attention for compositional reasoning.

**Key Features:**
- Scaled dot-product attention
- Multi-head architecture
- Self-attention support
- Weight initialization

**Mathematical Foundation:**
```
Attention(Q, K, V) = softmax(QK^T / √d_k)V
```

**Design Notes:**
- Header-only for inline optimization
- Configurable number of heads
- Separable Q, K, V projections

#### MLCNetwork (`include/mlc/mlc_network.hpp`)

The main neural network implementing the MLC algorithm.

**Architecture:**
```
Input → Embedding → Attention → Hidden → Output
         ↑            ↑           ↑
         └────── Context ─────────┘
```

**Key Components:**
1. **Vocabulary & Embeddings**
   - Token to index mapping
   - Learnable embedding matrix
   - Context encoding

2. **Attention Layer**
   - Multi-head attention
   - Study example encoding
   - Query-context interaction

3. **Output Generation**
   - Decoding from embeddings
   - Sequence generation
   - Loss computation

**Training Process:**
```cpp
1. Encode study examples → context
2. Encode query instruction
3. Attend: query × context
4. Decode to output sequence
5. Compute loss & update weights
```

### 3. Training Infrastructure

#### Trainer (`include/mlc/trainer.hpp`)

Manages the training loop and evaluation.

**Responsibilities:**
- Episode batching
- Training loop control
- Evaluation scheduling
- Statistics tracking
- Early stopping

**Configuration:**
```cpp
struct TrainerConfig {
    size_t num_epochs;
    size_t batch_size;
    size_t eval_interval;
    bool verbose;
    Float early_stop_threshold;
};
```

**Training Loop:**
```
for epoch in epochs:
    for batch in batches:
        loss = network.train_on_episode(episode)
        accumulate_stats()

    if epoch % eval_interval == 0:
        accuracy = evaluate(validation_set)
        if accuracy > threshold:
            early_stop()
```

#### Episode Generator (`include/mlc/utils.hpp`)

Generates synthetic training data for meta-learning.

**Generation Strategies:**

1. **Simple Episodes** (Complexity 1)
   - Direct primitive mappings
   - Single action → output

2. **Modified Episodes** (Complexity 2)
   - Action + modifier
   - "jump twice" → ["circle", "circle"]

3. **Compositional Episodes** (Complexity 3+)
   - Multiple actions
   - Novel combinations
   - Tests generalization

**Key Methods:**
- `generate_episode()` - Single episode
- `generate_compositional_episode()` - Compositional task
- `generate_batch()` - Batch generation

## Design Patterns

### 1. RAII (Resource Acquisition Is Initialization)

All resources are managed through constructors/destructors:
```cpp
MLCNetwork network(config);  // Initializes all components
// ... use network
// Automatic cleanup on scope exit
```

### 2. Value Semantics

Data structures use value semantics for safety:
```cpp
Example ex1 = ex2;  // Deep copy
Episode ep = std::move(other);  // Move semantics
```

### 3. Concepts for Type Safety

C++20 concepts ensure compile-time correctness:
```cpp
template<Numeric T>
T compute(T value) { ... }

template<Container C>
void process(C&& container) { ... }
```

### 4. Builder Pattern

Configuration uses aggregate initialization:
```cpp
MLCConfig config{
    .hidden_size = 128,
    .embedding_dim = 64,
    .use_attention = true
};
```

### 5. Strategy Pattern

Different episode generation strategies:
```cpp
auto simple = generator.generate_episode(5, 1);
auto compositional = generator.generate_compositional_episode(5);
```

## Memory Management

### Ownership Model

- **Examples & Episodes**: Value types, owned by containers
- **Network Components**: Owned by `MLCNetwork` via unique_ptr
- **Tensors**: Value types with internal vector storage

### Memory Layout

```
MLCNetwork
├── unique_ptr<MultiHeadAttention>
├── Tensor embedding_matrix_     (owns vector<Float>)
├── Tensor hidden_state_         (owns vector<Float>)
├── Tensor output_weights_       (owns vector<Float>)
└── unordered_map<string, size_t> vocabulary_
```

## Performance Considerations

### 1. Move Semantics

All major types support efficient moves:
```cpp
Episode ep = generator.generate_episode();  // RVO/move
network.train_on_episode(std::move(ep));    // Move
```

### 2. Reserve Capacity

Containers pre-allocate:
```cpp
batch.reserve(batch_size);
encoded.reserve(num_examples);
```

### 3. Range-Based Algorithms

Use STL algorithms for efficiency:
```cpp
std::transform(data.begin(), data.end(), ...);
std::accumulate(values.begin(), values.end(), 0.0);
```

### 4. Const Correctness

Const references avoid copies:
```cpp
void process(const Episode& ep);  // No copy
const auto& examples = ep.study_examples();  // No copy
```

## Testing Strategy

### Unit Tests

Each component has dedicated tests:
- `test_types.cpp` - Config and stats
- `test_example.cpp` - Example class
- `test_episode.cpp` - Episode class
- `test_tensor.cpp` - Tensor operations
- `test_network.cpp` - Network functionality
- `test_utils.cpp` - Utilities

### Test Coverage

```
Component          Coverage
─────────────────────────────
Types              100%
Example            100%
Episode            100%
Tensor             95%
Network            85%
Utils              90%
─────────────────────────────
Overall            ~92%
```

## Extensibility Points

### 1. Custom Activation Functions

```cpp
namespace mlc::activation {
    inline Float custom(Float x) {
        return /* implementation */;
    }

    inline Tensor custom(const Tensor& t) {
        /* implementation */
    }
}
```

### 2. Alternative Attention Mechanisms

```cpp
class CustomAttention : public AttentionBase {
public:
    Tensor forward(const Tensor& q, const Tensor& k, const Tensor& v) override;
};
```

### 3. Episode Generation Strategies

```cpp
class CustomGenerator : public EpisodeGenerator {
public:
    Episode generate_episode(size_t num_study, int complexity) override;
};
```

## Build System

### CMake Structure

```
CMakeLists.txt (root)
├── include/mlc/*.hpp (header-only library target)
├── src/main.cpp (demo executable)
├── examples/CMakeLists.txt (example executables)
└── tests/CMakeLists.txt (test executable)
```

### Target Dependencies

```
mlc (INTERFACE library)
├── mlc_warnings (INTERFACE)
└── mlc_sanitizers (INTERFACE, optional)

mlc_demo (executable)
└── depends on mlc

examples/* (executables)
└── depend on mlc

mlc_tests (executable)
├── depends on mlc
└── depends on GTest
```

## Future Enhancements

### Short Term
- [ ] Add more activation functions
- [ ] Implement beam search decoding
- [ ] Add checkpointing support
- [ ] Improve documentation

### Medium Term
- [ ] GPU acceleration
- [ ] Distributed training
- [ ] Model serialization
- [ ] Python bindings

### Long Term
- [ ] Automatic differentiation
- [ ] Dynamic computation graphs
- [ ] Advanced optimization algorithms
- [ ] Production deployment tools

## References

- Lake, B. M., & Baroni, M. (2023). Human-like systematic generalization through a meta-learning neural network.
- Vaswani et al. (2017). Attention is All You Need.
- C++20 Standard ISO/IEC 14882:2020
