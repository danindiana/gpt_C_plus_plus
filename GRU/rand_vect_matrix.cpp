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
