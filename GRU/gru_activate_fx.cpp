double sigmoid(double x) {
    return 1 / (1 + exp(-x));
}

double tanh(double x) {
    return (exp(x) - exp(-x)) / (exp(x) + exp(-x));
}
