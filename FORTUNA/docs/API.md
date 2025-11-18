# Fortuna API Documentation

## Table of Contents

1. [Core Module](#core-module)
2. [Bayesian Module](#bayesian-module)
3. [Utils Module](#utils-module)
4. [Data Types](#data-types)

## Core Module

### `fortuna::core::Layer`

Represents a single layer in a neural network.

#### Constructor

```cpp
Layer(Size input_size, Size output_size,
      ActivationType activation = ActivationType::RELU,
      std::string name = "");
```

**Parameters:**
- `input_size`: Number of input neurons
- `output_size`: Number of output neurons
- `activation`: Activation function type
- `name`: Optional layer name

#### Methods

##### `forward`
```cpp
[[nodiscard]] RealVector forward(std::span<const Real> inputs) const;
```

Performs forward pass through the layer with activation.

**Parameters:**
- `inputs`: Input vector

**Returns:** Output after applying weights, biases, and activation

##### `forward_linear`
```cpp
[[nodiscard]] RealVector forward_linear(std::span<const Real> inputs) const;
```

Performs forward pass without activation (useful for output layers).

##### `initialize_weights`
```cpp
void initialize_weights(const std::string& method = "xavier");
```

Initializes layer weights using the specified method.

**Parameters:**
- `method`: Initialization method ("xavier", "he", "lecun", etc.)

##### Getters

- `get_weights()` - Returns const reference to weight matrix
- `get_biases()` - Returns const reference to bias vector
- `input_size()` - Returns number of input neurons
- `output_size()` - Returns number of output neurons
- `activation_type()` - Returns activation function type
- `name()` - Returns layer name
- `num_parameters()` - Returns total parameters (weights + biases)

##### Setters

- `set_weights(const RealMatrix& weights)` - Sets layer weights
- `set_biases(const RealVector& biases)` - Sets layer biases

---

### `fortuna::core::Model`

Represents a complete neural network model.

#### Constructor

```cpp
explicit Model(std::string name = "FortunaModel");
```

#### Methods

##### `build`
```cpp
void build(std::span<const Size> layer_sizes,
           ActivationType activation = ActivationType::RELU,
           ActivationType output_activation = ActivationType::LINEAR);
```

Builds the model with specified layer sizes.

**Parameters:**
- `layer_sizes`: Vector of layer sizes [input, hidden1, ..., output]
- `activation`: Activation for hidden layers
- `output_activation`: Activation for output layer

**Example:**
```cpp
std::vector<Size> sizes = {784, 128, 64, 10};
model->build(sizes, ActivationType::RELU, ActivationType::SOFTMAX);
```

##### `add_layer`
```cpp
void add_layer(const Layer& layer);
void add_layer(Layer&& layer);
```

Adds a layer to the model manually.

##### `forward`
```cpp
[[nodiscard]] RealVector forward(std::span<const Real> inputs) const;
```

Performs forward pass through entire model.

##### `forward_with_activations`
```cpp
[[nodiscard]] std::vector<RealVector> forward_with_activations(
    std::span<const Real> inputs) const;
```

Performs forward pass and returns all intermediate activations.

##### `summary`
```cpp
[[nodiscard]] std::string summary() const;
```

Returns a string summary of model architecture.

##### Getters

- `num_layers()` - Number of layers
- `get_layer(Size index)` - Get specific layer
- `layers()` - Get all layers
- `num_parameters()` - Total parameters across all layers
- `name()` - Model name

##### Setters

- `set_name(std::string name)` - Set model name
- `initialize_weights(const std::string& method)` - Initialize all layer weights

---

### `fortuna::core::DataLoader`

Handles data loading, batching, and shuffling.

#### Constructor

```cpp
DataLoader(RealMatrix inputs, RealMatrix targets,
           Size batch_size = 32, bool shuffle = true);
```

**Parameters:**
- `inputs`: Input data [num_samples][input_dim]
- `targets`: Target data [num_samples][output_dim]
- `batch_size`: Batch size for mini-batch training
- `shuffle`: Whether to shuffle data each epoch

#### Methods

##### `next_batch`
```cpp
[[nodiscard]] std::optional<Batch> next_batch();
```

Returns the next batch of data, or `std::nullopt` if epoch is complete.

##### `reset`
```cpp
void reset();
```

Resets the data loader for a new epoch.

##### `train_val_split` (static)
```cpp
static std::pair<DataLoader, DataLoader> train_val_split(
    const DataLoader& data, Real train_ratio = 0.8, unsigned int seed = 42);
```

Splits data into training and validation sets.

**Example:**
```cpp
auto [train, val] = DataLoader::train_val_split(data, 0.8);
```

##### Getters

- `num_samples()` - Total number of samples
- `num_batches()` - Number of batches per epoch
- `batch_size()` - Batch size
- `input_dim()` - Input feature dimension
- `output_dim()` - Output/target dimension
- `inputs()` - All input data
- `targets()` - All target data

---

## Bayesian Module

### `fortuna::bayesian::ProbClassifier`

Probabilistic classifier using Bayesian neural networks.

#### Constructor

```cpp
explicit ProbClassifier(std::shared_ptr<core::Model> model,
                       BayesianMethod method = BayesianMethod::MAP);
```

**Parameters:**
- `model`: Shared pointer to neural network model
- `method`: Bayesian inference method

#### Methods

##### `train`
```cpp
void train(core::DataLoader& train_data,
          std::optional<std::reference_wrapper<core::DataLoader>> val_data,
          const TrainingConfig& config);
```

Trains the Bayesian model.

**Parameters:**
- `train_data`: Training data loader
- `val_data`: Optional validation data loader
- `config`: Training configuration

**Example:**
```cpp
TrainingConfig config;
config.num_epochs = 100;
config.learning_rate = 0.001;
classifier.train(train_data, val_data, config);
```

##### `predict`
```cpp
[[nodiscard]] PredictionResult predict(std::span<const Real> inputs,
                                      Size num_samples = 100) const;
```

Predicts with uncertainty quantification.

**Returns:** `PredictionResult` containing:
- `mean`: Predictive mean
- `std`: Predictive standard deviation
- `variance`: Predictive variance
- `epistemic_uncertainty`: Model uncertainty
- `aleatoric_uncertainty`: Data uncertainty

##### `predict_mean`
```cpp
[[nodiscard]] RealVector predict_mean(std::span<const Real> inputs) const;
```

Faster prediction returning only the mean (no uncertainty).

##### Getters

- `model()` - Get underlying model
- `method()` - Get Bayesian method
- `loss_history()` - Get training loss history

##### Setters

- `set_method(BayesianMethod method)` - Set Bayesian method

---

### `fortuna::bayesian::TrainingConfig`

Training configuration structure.

```cpp
struct TrainingConfig {
    Size num_epochs = 100;
    Real learning_rate = 0.001;
    Real weight_decay = 0.0001;
    Size batch_size = 32;
    bool verbose = true;
    Size print_every = 10;

    // Bayesian-specific
    Real prior_std = 1.0;
    Size num_samples = 100;
    Size burnin = 50;
    Real temperature = 1.0;
};
```

---

## Utils Module

### `fortuna::utils::Activations`

Activation functions for neural networks.

#### Static Methods

- `sigmoid(Real x)` - Sigmoid activation
- `tanh(Real x)` - Hyperbolic tangent
- `relu(Real x)` - ReLU
- `leaky_relu(Real x, Real alpha = 0.01)` - Leaky ReLU
- `elu(Real x, Real alpha = 1.0)` - ELU
- `linear(Real x)` - Identity/linear
- `softmax(const RealVector& x)` - Softmax (vector)
- `apply(Real x, ActivationType type)` - Apply by type
- `apply(const RealVector& x, ActivationType type)` - Apply to vector

#### Derivatives

- `sigmoid_derivative(Real x)`
- `tanh_derivative(Real x)`
- `relu_derivative(Real x)`
- `leaky_relu_derivative(Real x, Real alpha = 0.01)`

---

### `fortuna::utils::Initializers`

Weight initialization strategies.

#### Static Methods

- `xavier_uniform(RealMatrix& weights, Size fan_in, Size fan_out, unsigned int seed)`
- `xavier_normal(RealMatrix& weights, Size fan_in, Size fan_out, unsigned int seed)`
- `he_uniform(RealMatrix& weights, Size fan_in, Size fan_out, unsigned int seed)`
- `he_normal(RealMatrix& weights, Size fan_in, Size fan_out, unsigned int seed)`
- `lecun_uniform(RealMatrix& weights, Size fan_in, Size fan_out, unsigned int seed)`
- `lecun_normal(RealMatrix& weights, Size fan_in, Size fan_out, unsigned int seed)`
- `zeros(RealMatrix& weights, Size fan_in, Size fan_out)`
- `constant(RealMatrix& weights, Size fan_in, Size fan_out, Real value)`
- `zero_bias(RealVector& biases, Size size)`
- `initialize(RealMatrix& weights, Size fan_in, Size fan_out, const std::string& method, unsigned int seed)`

---

## Data Types

### Type Aliases

```cpp
using Real = double;
using Size = std::size_t;
using RealVector = std::vector<Real>;
using RealMatrix = std::vector<RealVector>;
using Shape = std::vector<Size>;
```

### Enumerations

#### `BayesianMethod`
```cpp
enum class BayesianMethod {
    MAP,                // Maximum A Posteriori
    ADVI,              // Automatic Differentiation Variational Inference
    DEEP_ENSEMBLES,    // Deep Ensembles
    LAPLACE_GN,        // Laplace approximation
    SWAG,              // Stochastic Weight Averaging Gaussian
    SGHMC,             // Stochastic Gradient Hamiltonian Monte Carlo
    CSGDLD,            // Cyclical Stochastic Gradient Langevin Dynamics
    SNNGP              // Spectral-normalized Neural Gaussian Process
};
```

#### `ActivationType`
```cpp
enum class ActivationType {
    SIGMOID,
    TANH,
    RELU,
    LEAKY_RELU,
    ELU,
    SOFTMAX,
    LINEAR
};
```

---

## Complete Example

```cpp
#include <fortuna/fortuna.hpp>
#include <iostream>

using namespace fortuna;

int main() {
    // Create data
    RealMatrix inputs = /* your data */;
    RealMatrix targets = /* your targets */;

    // Create data loader
    core::DataLoader data(inputs, targets, 32, true);
    auto [train, val] = core::DataLoader::train_val_split(data, 0.8);

    // Build model
    auto model = std::make_shared<core::Model>("MyModel");
    std::vector<Size> sizes = {input_dim, 64, 32, output_dim};
    model->build(sizes, ActivationType::RELU, ActivationType::SIGMOID);

    std::cout << model->summary();

    // Create Bayesian classifier
    bayesian::ProbClassifier classifier(model, BayesianMethod::MAP);

    // Configure training
    bayesian::TrainingConfig config;
    config.num_epochs = 100;
    config.learning_rate = 0.001;
    config.verbose = true;

    // Train
    classifier.train(train, val, config);

    // Predict
    RealVector test_input = /* test data */;
    auto result = classifier.predict(test_input, 100);

    std::cout << "Prediction: " << result.mean[0]
              << " ± " << result.std[0] << std::endl;

    return 0;
}
```
