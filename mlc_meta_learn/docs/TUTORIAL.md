# MLC Tutorial

## Getting Started with Meta-Learning for Compositionality

This tutorial will guide you through using the MLC framework to build and train compositional neural networks.

## Table of Contents

1. [Introduction](#introduction)
2. [Basic Concepts](#basic-concepts)
3. [Your First MLC Network](#your-first-mlc-network)
4. [Understanding Episodes](#understanding-episodes)
5. [Training a Network](#training-a-network)
6. [Compositional Generalization](#compositional-generalization)
7. [Advanced Topics](#advanced-topics)

## Introduction

MLC (Meta-Learning for Compositionality) enables neural networks to learn compositional rules from examples, similar to how humans learn language.

### What is Compositional Learning?

Compositional learning means understanding how to combine known primitives in novel ways:

- **Primitive**: "jump" → "circle"
- **Primitive**: "twice" → repeat action
- **Composition**: "jump twice" → "circle circle"

## Basic Concepts

### Examples

An `Example` pairs an instruction with its output:

```cpp
#include "mlc/mlc.hpp"
using namespace mlc;

// Create examples
Example ex1("jump", {"circle"});
Example ex2("skip", {"square"});
Example ex3("run", {"triangle"});

// Check validity
if (ex1.is_valid()) {
    std::cout << "Valid example!\n";
}

// Access components
std::cout << "Instruction: " << ex1.instruction() << "\n";
std::cout << "Output size: " << ex1.output_size() << "\n";
```

### Episodes

An `Episode` contains:
- **Study Examples**: Context for learning
- **Query Example**: What to predict

```cpp
Episode ep;

// Add study examples (these teach the network)
ep.add_study_example(Example("jump", {"circle"}));
ep.add_study_example(Example("skip", {"square"}));

// Set query (this tests the network)
ep.set_query_example(Example("run", {"triangle"}));

// Verify the episode
if (ep.is_valid()) {
    std::cout << "Episode has " << ep.num_study_examples()
              << " study examples\n";
}
```

## Your First MLC Network

Let's create and configure a simple network:

```cpp
#include "mlc/mlc.hpp"
#include <iostream>

int main() {
    using namespace mlc;

    // Step 1: Configure the network
    MLCConfig config;
    config.hidden_size = 64;        // Smaller for quick training
    config.embedding_dim = 32;
    config.attention_heads = 2;
    config.learning_rate = 0.01;

    // Verify configuration is valid
    if (!config.is_valid()) {
        std::cerr << "Invalid configuration!\n";
        return 1;
    }

    // Step 2: Create the network
    MLCNetwork network(config);

    std::cout << "Network created successfully!\n";
    std::cout << "Hidden size: " << config.hidden_size << "\n";
    std::cout << "Embedding dim: " << config.embedding_dim << "\n";

    return 0;
}
```

Compile and run:
```bash
g++ -std=c++20 -I../include first_network.cpp -o first_network
./first_network
```

## Understanding Episodes

Episodes are the core of meta-learning. Here's how to create them:

### Manual Episode Creation

```cpp
Episode create_manual_episode() {
    Episode ep;

    // Teach the network some primitives
    ep.add_study_example(Example("jump", {"circle"}));
    ep.add_study_example(Example("skip", {"square"}));
    ep.add_study_example(Example("run", {"triangle"}));

    // Test if it can generalize
    ep.set_query_example(Example("jump", {"circle"}));

    return ep;
}
```

### Automatic Episode Generation

```cpp
#include "mlc/utils.hpp"

void generate_episodes() {
    using namespace mlc;

    utils::EpisodeGenerator gen(42);  // Seed for reproducibility

    // Generate a single episode
    auto ep1 = gen.generate_episode(
        5,  // Number of study examples
        1   // Complexity level (1-3)
    );

    // Generate a compositional episode
    auto ep2 = gen.generate_compositional_episode(5);

    // Generate a batch
    auto batch = gen.generate_batch(
        100,  // Batch size
        5,    // Study examples per episode
        2     // Complexity
    );

    std::cout << "Generated " << batch.size() << " episodes\n";
}
```

## Training a Network

### Basic Training Loop

```cpp
#include "mlc/mlc.hpp"

void basic_training() {
    using namespace mlc;

    // Setup
    MLCConfig config;
    MLCNetwork network(config);

    // Create an episode
    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.set_query_example(Example("jump", {"circle"}));

    // Train for several iterations
    std::cout << "Training...\n";
    for (int i = 0; i < 10; ++i) {
        Float loss = network.train_on_episode(ep);
        std::cout << "Iteration " << (i + 1)
                  << ", Loss: " << loss << "\n";
    }

    // Check stats
    const auto& stats = network.stats();
    std::cout << "Final accuracy: "
              << (stats.accuracy * 100.0) << "%\n";
}
```

### Using the Trainer

The `Trainer` class provides a higher-level interface:

```cpp
#include "mlc/mlc.hpp"
#include "mlc/utils.hpp"

void advanced_training() {
    using namespace mlc;

    // Generate training data
    utils::EpisodeGenerator gen(42);
    auto train_eps = gen.generate_batch(800, 5, 2);
    auto val_eps = gen.generate_batch(200, 5, 2);

    // Configure network and trainer
    MLCConfig net_config;
    MLCNetwork network(net_config);

    Trainer::TrainerConfig trainer_config;
    trainer_config.num_epochs = 50;
    trainer_config.batch_size = 32;
    trainer_config.eval_interval = 5;
    trainer_config.verbose = true;
    trainer_config.early_stop_threshold = 0.90;

    // Train!
    Trainer trainer(network, trainer_config);
    trainer.train(train_eps, val_eps);

    std::cout << "Training complete!\n";
}
```

Output:
```
Epoch    0 | Loss: 0.850000 | Val Acc: 25.50% | Train Acc: 24.75%
Epoch    5 | Loss: 0.620000 | Val Acc: 42.00% | Train Acc: 43.50%
Epoch   10 | Loss: 0.450000 | Val Acc: 65.50% | Train Acc: 67.25%
...
Training complete!
```

## Compositional Generalization

The key capability of MLC is compositional generalization:

### Example: Learning Composition

```cpp
#include "mlc/mlc.hpp"
#include "mlc/utils.hpp"

void test_composition() {
    using namespace mlc;

    // Create network
    MLCConfig config;
    config.use_attention = true;  // Important for composition!
    MLCNetwork network(config);

    // Generate compositional training data
    utils::EpisodeGenerator gen(42);
    auto episodes = gen.generate_batch(500, 5, 3);

    // Train
    Trainer trainer(network);
    trainer.train(episodes);

    // Test compositional understanding
    std::vector<Example> context = {
        Example("jump", {"circle"}),
        Example("skip", {"square"}),
        Example("twice", {"repeat"})
    };

    // Novel composition: "jump twice"
    auto output = network.predict("jump twice", context);

    std::cout << "Predicted for 'jump twice': ";
    for (const auto& item : output) {
        std::cout << item << " ";
    }
    std::cout << "\n";
}
```

### Understanding the Results

The network learns to:
1. Understand primitive mappings from study examples
2. Compose these primitives in novel ways
3. Generalize to unseen combinations

## Advanced Topics

### Custom Episode Generation

Create your own episode generator:

```cpp
class CustomGenerator {
public:
    Episode generate_math_episode() {
        Episode ep;

        // Teach basic operations
        ep.add_study_example(Example("add 2 3", {"5"}));
        ep.add_study_example(Example("mul 2 3", {"6"}));
        ep.add_study_example(Example("sub 5 2", {"3"}));

        // Test composition
        ep.set_query_example(Example("add 1 1", {"2"}));

        return ep;
    }
};
```

### Monitoring Training

Track detailed statistics:

```cpp
void monitor_training() {
    using namespace mlc;

    MLCNetwork network(MLCConfig{});
    utils::EpisodeGenerator gen(42);

    for (int epoch = 0; epoch < 10; ++epoch) {
        network.reset_stats();

        // Train for one epoch
        auto episodes = gen.generate_batch(100, 5, 2);
        for (const auto& ep : episodes) {
            network.train_on_episode(ep);
        }

        // Get statistics
        const auto& stats = network.stats();
        std::cout << "Epoch " << epoch << ":\n"
                  << "  Total examples: " << stats.total_examples << "\n"
                  << "  Correct: " << stats.correct_predictions << "\n"
                  << "  Accuracy: " << (stats.accuracy * 100.0) << "%\n"
                  << "  Loss: " << stats.loss << "\n";
    }
}
```

### Saving and Loading (Future Feature)

```cpp
// Future API
void save_load() {
    MLCNetwork network(config);

    // Train...
    trainer.train(episodes);

    // Save model
    network.save("model.mlc");

    // Load model
    MLCNetwork loaded = MLCNetwork::load("model.mlc");
}
```

### Integration with Existing Code

Use MLC as a header-only library:

```cpp
// your_project.cpp
#include "mlc/mlc.hpp"

class YourApplication {
    mlc::MLCNetwork network_;

public:
    YourApplication() : network_(mlc::MLCConfig{}) {}

    void learn_from_user_input(const std::string& instruction,
                               const std::vector<std::string>& output) {
        mlc::Example ex(instruction, output);
        // ... use in training
    }
};
```

## Best Practices

### 1. Start Simple

Begin with small networks and few examples:
```cpp
MLCConfig config;
config.hidden_size = 64;
config.embedding_dim = 32;
```

### 2. Use Attention for Composition

Always enable attention for compositional tasks:
```cpp
config.use_attention = true;
config.attention_heads = 4;
```

### 3. Validate Your Data

Always check episode validity:
```cpp
if (!episode.is_valid()) {
    std::cerr << "Invalid episode!\n";
    return;
}
```

### 4. Monitor Training

Use verbose mode to watch progress:
```cpp
Trainer::TrainerConfig trainer_config;
trainer_config.verbose = true;
trainer_config.eval_interval = 10;
```

### 5. Use Proper Splits

Split your data into train/val/test:
```cpp
auto split = utils::training::split_episodes(
    all_episodes,
    0.7,   // 70% training
    0.15   // 15% validation, 15% test
);
```

## Complete Example Program

See `examples/simple_example.cpp` and `examples/compositional_test.cpp` for complete, runnable examples.

## Next Steps

1. **Experiment**: Try different configurations
2. **Custom Data**: Create domain-specific episodes
3. **Evaluate**: Test on novel compositions
4. **Optimize**: Tune hyperparameters
5. **Deploy**: Integrate into your application

## Resources

- [API Reference](API.md)
- [Architecture Documentation](ARCHITECTURE.md)
- [Research Paper](original_readme.txt)

## Getting Help

- GitHub Issues: Report bugs or request features
- Examples: Check the `examples/` directory
- Tests: See `tests/` for usage patterns
