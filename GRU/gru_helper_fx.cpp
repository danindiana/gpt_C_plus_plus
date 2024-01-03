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
