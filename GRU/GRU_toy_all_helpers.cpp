#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>  // For rand()

using namespace std;

// Helper functions
vector<double> dot(vector<vector<double>> W, vector<double> x) {
    int m = W.size();
    int n = x.size();
    vector<double> result(m);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            result[i] += W[i][j] * x[j];
        }
    }
    return result;
}

vector<double> concatenate(vector<double> a, vector<double> b) {
    vector<double> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

vector<double> multiply(vector<double> a, vector<double> b) {
    int n = a.size();
    vector<double> result(n);
    for (int i = 0; i < n; i++) {
        result[i] = a[i] * b[i];
    }
    return result;
}

double sigmoid(double x) {
    return 1 / (1 + exp(-x));
}

double tanh(double x) {
    return (exp(x) - exp(-x)) / (exp(x) + exp(-x));
}

vector<vector<double>> random_matrix(int rows, int cols) {
    vector<vector<double>> matrix(rows, vector<double>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = rand() / (double)RAND_MAX;  // Generate random values between 0 and 1
        }
    }
    return matrix;
}

vector<double> random_vector(int size) {
    vector<double> vector(size);
    for (int i = 0; i < size; i++) {
        vector[i] = rand() / (double)RAND_MAX;
    }
    return vector;
}

// GRU class
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
};

int main() {
    // Example usage
    GRU gru(5, 1
