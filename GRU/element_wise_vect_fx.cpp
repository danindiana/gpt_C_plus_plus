vector<double> multiply(vector<double> a, vector<double> b) {
    int n = a.size();
    vector<double> result(n);
    for (int i = 0; i < n; i++) {
        result[i] = a[i] * b[i];
    }
    return result;
}
