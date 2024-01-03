vector<double> concatenate(vector<double> a, vector<double> b) {
    vector<double> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}
