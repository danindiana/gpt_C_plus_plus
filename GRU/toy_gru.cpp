#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

struct GRU {
    // Weights and biases for gates and update
    vector<vector<double>> Wz, Wr, Wh;
    vector<double> bz, br, bh;

    // Hidden state and output
    vector<double> h_prev, h_t;

    GRU(int input_size, int hidden_size) {
        // Initialize weights and biases with appropriate dimensions
        Wz = random_matrix(hidden_size, input_size + hidden_size);
        Wr = random_matrix(hidden_size, input_size + hidden_size);
        Wh = random_matrix(hidden_size, input_size + hidden_size);
        bz = random_vector(hidden_size);
        br = random_vector(hidden_size);
        bh = random_vector(hidden_size);

        // Initialize hidden state
        h_prev = random_vector(hidden_size);
    }

    vector<double> forward(vector<double> x_t) {
        // Combine input and previous hidden state
        vector<double> z = concatenate(x_t, h_prev);

        // Update gate
        vector<double> zt = sigmoid(dot(Wz, z) + bz);

        // Reset gate
        vector<double> rt = sigmoid(dot(Wr, z) + br);

        // Candidate activation
        vector<double> ht_hat = tanh(dot(Wh, concatenate(x_t, multiply(rt, h_prev))) + bh);

        // Final hidden state
        h_t = multiply(zt, h_prev) + multiply((1 - zt), ht_hat);

        return h_t;  // Return hidden state as output
    }

private:
    // Helper functions for matrix operations and activations
    // ... (implement dot product, sigmoid, tanh, etc.)
};

int main() {
    // Example usage
    GRU gru(5, 10);  // Input size 5, hidden size 10
    vector<double> input = {1, 2, 3, 4, 5};
    vector<double> output = gru.forward(input);
    // Process output...
}
