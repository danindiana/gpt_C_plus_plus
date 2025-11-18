# MLC API Reference

## Core Classes

### Example

Represents a single instruction-output pair.

```cpp
class Example {
public:
    Example();
    explicit Example(Instruction instr);
    Example(Instruction instr, OutputSequence out);

    // Accessors
    const Instruction& instruction() const noexcept;
    const OutputSequence& output() const noexcept;

    // Setters
    void set_instruction(Instruction instr);
    void set_output(OutputSequence out);

    // Validation
    bool is_valid() const noexcept;
    size_t output_size() const noexcept;
};
```

**Example Usage:**
```cpp
Example ex("jump twice", {"circle", "circle"});
if (ex.is_valid()) {
    std::cout << "Instruction: " << ex.instruction() << "\n";
}
```

---

### Episode

Container for study examples and a query example.

```cpp
class Episode {
public:
    Episode();
    explicit Episode(std::vector<Example> study, Example query);

    // Accessors
    const std::vector<Example>& study_examples() const noexcept;
    const Example& query_example() const noexcept;

    // Modifiers
    void add_study_example(Example example);
    void set_query_example(Example example);
    void clear_study_examples() noexcept;

    // Utilities
    size_t num_study_examples() const noexcept;
    bool is_valid() const noexcept;
};
```

**Example Usage:**
```cpp
Episode ep;
ep.add_study_example(Example("jump", {"circle"}));
ep.add_study_example(Example("skip", {"square"}));
ep.set_query_example(Example("run", {"triangle"}));

std::cout << "Episode has " << ep.num_study_examples() << " study examples\n";
```

---

### MLCNetwork

Main neural network for meta-learning.

```cpp
class MLCNetwork {
public:
    explicit MLCNetwork(const MLCConfig& config = MLCConfig{});

    // Training
    Float train_on_episode(const Episode& episode);

    // Evaluation
    Float evaluate(const std::vector<Episode>& episodes);

    // Prediction
    OutputSequence predict(
        const Instruction& instruction,
        const std::vector<Example>& study_examples = {}
    );

    // Statistics
    const TrainingStats& stats() const noexcept;
    void reset_stats() noexcept;

    // Configuration
    const MLCConfig& config() const noexcept;
};
```

**Example Usage:**
```cpp
MLCConfig config;
config.hidden_size = 128;
config.learning_rate = 0.001;

MLCNetwork network(config);

Episode ep = /* ... */;
Float loss = network.train_on_episode(ep);

auto output = network.predict("jump twice", study_examples);
```

---

### Trainer

High-level training interface.

```cpp
class Trainer {
public:
    struct TrainerConfig {
        size_t num_epochs = 100;
        size_t batch_size = 32;
        size_t eval_interval = 10;
        bool verbose = true;
        Float early_stop_threshold = 0.95;
    };

    explicit Trainer(MLCNetwork& network,
                    const TrainerConfig& config = TrainerConfig{});

    void train(const std::vector<Episode>& train_episodes,
              const std::vector<Episode>& val_episodes = {});

    Float evaluate(const std::vector<Episode>& test_episodes);
};
```

**Example Usage:**
```cpp
MLCNetwork network(config);
Trainer::TrainerConfig trainer_config;
trainer_config.num_epochs = 50;
trainer_config.verbose = true;

Trainer trainer(network, trainer_config);
trainer.train(train_episodes, val_episodes);
```

---

## Utility Classes

### EpisodeGenerator

Generates synthetic training data.

```cpp
class EpisodeGenerator {
public:
    explicit EpisodeGenerator(unsigned int seed = 42);

    Episode generate_episode(size_t num_study = 5, int complexity = 1);
    Episode generate_compositional_episode(size_t num_study = 5);
    std::vector<Episode> generate_batch(size_t batch_size,
                                       size_t num_study = 5,
                                       int complexity = 1);
};
```

**Example Usage:**
```cpp
utils::EpisodeGenerator gen(42);

// Simple episodes
auto simple = gen.generate_episode(5, 1);

// Compositional episodes
auto comp = gen.generate_compositional_episode(5);

// Batch generation
auto batch = gen.generate_batch(100, 5, 2);
```

---

### Tensor

Lightweight tensor implementation.

```cpp
class Tensor {
public:
    Tensor();
    explicit Tensor(std::vector<size_t> shape, Float init_value = 0.0);
    Tensor(std::vector<size_t> shape, std::vector<Float> data);

    // Accessors
    const std::vector<Float>& data() const noexcept;
    const std::vector<size_t>& shape() const noexcept;
    size_t size() const noexcept;
    size_t ndim() const noexcept;

    // Element access
    Float& operator[](size_t idx);
    const Float& operator[](size_t idx) const;

    template<typename... Indices>
    Float& at(Indices... indices);

    // Operations
    Tensor& operator+=(const Tensor& other);
    Tensor& operator-=(const Tensor& other);
    Tensor& operator*=(Float scalar);
    Tensor operator+(const Tensor& other) const;
    Tensor operator-(const Tensor& other) const;
    Tensor operator*(Float scalar) const;

    // Utilities
    void fill(Float value);
    void zero();
    void randomize(Float min = -1.0, Float max = 1.0);
    Float sum() const;
    Float mean() const;
    Float max() const;
    Float min() const;
    Tensor reshape(std::vector<size_t> new_shape) const;
};
```

**Example Usage:**
```cpp
Tensor t({3, 4}, 0.0);  // 3x4 tensor of zeros
t.randomize(-0.1, 0.1);

Float sum = t.sum();
Float mean = t.mean();

Tensor reshaped = t.reshape({2, 6});
```

---

## Configuration Structures

### MLCConfig

Configuration for MLC networks.

```cpp
struct MLCConfig {
    size_t hidden_size = 128;
    size_t num_layers = 2;
    size_t embedding_dim = 64;
    size_t attention_heads = 4;
    Float learning_rate = 0.001;
    Float dropout_rate = 0.1;
    size_t batch_size = 32;
    size_t max_sequence_length = 100;
    bool use_attention = true;
    size_t random_seed = 42;

    bool is_valid() const noexcept;
};
```

### TrainingStats

Training statistics.

```cpp
struct TrainingStats {
    size_t epoch = 0;
    Float loss = 0.0;
    Float accuracy = 0.0;
    Float validation_loss = 0.0;
    Float validation_accuracy = 0.0;
    size_t total_examples = 0;
    size_t correct_predictions = 0;

    void reset() noexcept;
    void update_accuracy() noexcept;
};
```

---

## Activation Functions

```cpp
namespace mlc::activation {
    // Scalar versions
    Float sigmoid(Float x);
    Float tanh(Float x);
    Float relu(Float x);

    // Tensor versions
    Tensor sigmoid(const Tensor& t);
    Tensor tanh(const Tensor& t);
    Tensor relu(const Tensor& t);
    Tensor softmax(const Tensor& t);
}
```

**Example Usage:**
```cpp
using namespace mlc::activation;

Tensor t({10}, 0.0);
t.randomize(-1.0, 1.0);

auto activated = relu(t);
auto normalized = softmax(t);
```

---

## Training Utilities

```cpp
namespace mlc::utils::training {
    // Split episodes into train/val/test
    auto split_episodes(
        const std::vector<Episode>& episodes,
        Float train_ratio = 0.7,
        Float val_ratio = 0.15
    );

    // Shuffle episodes
    void shuffle_episodes(std::vector<Episode>& episodes,
                         unsigned int seed = 42);
}
```

**Example Usage:**
```cpp
using namespace mlc::utils::training;

std::vector<Episode> all_episodes = /* ... */;

auto split = split_episodes(all_episodes, 0.7, 0.15);
// split.train, split.validation, split.test

shuffle_episodes(split.train, 42);
```

---

## Type Aliases

```cpp
namespace mlc {
    using Instruction = std::string;
    using OutputSequence = std::vector<std::string>;
    using Float = double;
}
```

---

## Concepts

```cpp
namespace mlc {
    template<typename T>
    concept Numeric = std::integral<T> || std::floating_point<T>;

    template<typename T>
    concept Container = requires(T t) {
        { std::ranges::begin(t) } -> std::input_or_output_iterator;
        { std::ranges::end(t) } -> std::input_or_output_iterator;
        { std::ranges::size(t) } -> std::convertible_to<std::size_t>;
    };

    template<typename T>
    concept Trainable = requires(T t) {
        { t.train() } -> std::same_as<void>;
        { t.evaluate() } -> Numeric;
    };
}
```

---

## Complete Example

```cpp
#include "mlc/mlc.hpp"
#include <iostream>

int main() {
    using namespace mlc;

    // 1. Configure network
    MLCConfig config;
    config.hidden_size = 128;
    config.embedding_dim = 64;
    config.attention_heads = 4;
    config.learning_rate = 0.001;

    // 2. Create network
    MLCNetwork network(config);

    // 3. Generate training data
    utils::EpisodeGenerator gen(42);
    auto train_eps = gen.generate_batch(1000, 5, 2);
    auto val_eps = gen.generate_batch(200, 5, 2);

    // 4. Train
    Trainer::TrainerConfig trainer_config;
    trainer_config.num_epochs = 100;
    trainer_config.eval_interval = 10;
    trainer_config.verbose = true;

    Trainer trainer(network, trainer_config);
    trainer.train(train_eps, val_eps);

    // 5. Predict
    std::vector<Example> context = {
        Example("jump", {"circle"}),
        Example("skip", {"square"})
    };

    auto output = network.predict("jump twice", context);

    std::cout << "Predicted output: ";
    for (const auto& item : output) {
        std::cout << item << " ";
    }
    std::cout << "\n";

    // 6. Final stats
    const auto& stats = network.stats();
    std::cout << "Final accuracy: " << (stats.accuracy * 100.0) << "%\n";

    return 0;
}
```
