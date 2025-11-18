#include "realm/mips.hpp"
#include "realm/types.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>

using namespace realm;
using namespace std::chrono;

// Helper function to generate random vectors
DenseVector<Float> generate_random_vector(VectorDim dim, std::mt19937& gen) {
    std::normal_distribution<Float> dist(0.0f, 1.0f);
    DenseVector<Float> vec(dim);
    for (size_t i = 0; i < dim; ++i) {
        vec[i] = dist(gen);
    }
    vec.normalize();
    return vec;
}

// Benchmark function
void benchmark_mips(
    IMIPSRetriever* retriever,
    const std::string& name,
    const std::vector<DenseVector<Float>>& queries,
    size_t k
) {
    std::cout << "\n=== " << name << " ===" << std::endl;

    // Warm-up
    for (size_t i = 0; i < std::min(size_t(5), queries.size()); ++i) {
        retriever->search(queries[i], k);
    }

    // Benchmark
    auto start = high_resolution_clock::now();

    for (const auto& query : queries) {
        auto results = retriever->search(query, k);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);

    double avg_time_ms = static_cast<double>(duration.count()) / 1000.0 / static_cast<double>(queries.size());
    double qps = static_cast<double>(queries.size()) * 1000000.0 / static_cast<double>(duration.count());

    std::cout << "  Queries: " << queries.size() << std::endl;
    std::cout << "  Top-K: " << k << std::endl;
    std::cout << "  Total Time: " << std::fixed << std::setprecision(2)
              << static_cast<double>(duration.count()) / 1000.0 << " ms" << std::endl;
    std::cout << "  Avg Time/Query: " << std::fixed << std::setprecision(3)
              << avg_time_ms << " ms" << std::endl;
    std::cout << "  Queries/Second: " << std::fixed << std::setprecision(1)
              << qps << std::endl;
}

int main() {
    std::cout << "=== MIPS Benchmark ===" << std::endl << std::endl;

    // Configuration
    const VectorDim dim = 256;
    const size_t num_documents = 10000;
    const size_t num_queries = 100;
    const size_t k = 10;

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Vector Dimension: " << dim << std::endl;
    std::cout << "  Number of Documents: " << num_documents << std::endl;
    std::cout << "  Number of Queries: " << num_queries << std::endl;
    std::cout << "  Top-K: " << k << std::endl << std::endl;

    // Generate random data
    std::cout << "Generating random data..." << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<DenseVector<Float>> documents;
    documents.reserve(num_documents);
    for (size_t i = 0; i < num_documents; ++i) {
        documents.push_back(generate_random_vector(dim, gen));
    }

    std::vector<DenseVector<Float>> queries;
    queries.reserve(num_queries);
    for (size_t i = 0; i < num_queries; ++i) {
        queries.push_back(generate_random_vector(dim, gen));
    }

    std::cout << "Data generation complete!" << std::endl;

    // Benchmark Brute Force MIPS
    std::cout << "\nIndexing documents for Brute Force MIPS..." << std::endl;
    BruteForceMIPS brute_force(dim);
    auto start = high_resolution_clock::now();

    for (size_t i = 0; i < documents.size(); ++i) {
        brute_force.add_document(i, documents[i]);
    }
    brute_force.build_index();

    auto end = high_resolution_clock::now();
    auto index_time = duration_cast<milliseconds>(end - start);

    std::cout << "Indexing time: " << index_time.count() << " ms" << std::endl;

    benchmark_mips(&brute_force, "Brute Force MIPS", queries, k);

    // Benchmark Asymmetric LSH
    std::cout << "\n\nIndexing documents for Asymmetric LSH..." << std::endl;

    AsymmetricLSH::Params lsh_params;
    lsh_params.num_hash_tables = 10;
    lsh_params.hash_size = 64;
    lsh_params.U = 1.0f;
    lsh_params.r = 0.5f;

    AsymmetricLSH alsh(dim, lsh_params);

    start = high_resolution_clock::now();

    for (size_t i = 0; i < documents.size(); ++i) {
        alsh.add_document(i, documents[i]);
    }
    alsh.build_index();

    end = high_resolution_clock::now();
    index_time = duration_cast<milliseconds>(end - start);

    std::cout << "Indexing time: " << index_time.count() << " ms" << std::endl;
    std::cout << "LSH Parameters:" << std::endl;
    std::cout << "  Hash Tables: " << lsh_params.num_hash_tables << std::endl;
    std::cout << "  Hash Size: " << lsh_params.hash_size << std::endl;

    benchmark_mips(&alsh, "Asymmetric LSH", queries, k);

    // Compare accuracy
    std::cout << "\n\n=== Accuracy Comparison ===" << std::endl;
    std::cout << "Computing recall@" << k << "..." << std::endl;

    size_t total_overlap = 0;

    for (const auto& query : queries) {
        auto bf_results = brute_force.search(query, k);
        auto alsh_results = alsh.search(query, k);

        // Count overlap
        size_t overlap = 0;
        for (const auto& bf_result : bf_results) {
            for (const auto& alsh_result : alsh_results) {
                if (bf_result.doc_id == alsh_result.doc_id) {
                    ++overlap;
                    break;
                }
            }
        }
        total_overlap += overlap;
    }

    double recall = static_cast<double>(total_overlap) / static_cast<double>(queries.size() * k);

    std::cout << "ALSH Recall@" << k << ": " << std::fixed << std::setprecision(4)
              << recall << " (" << (recall * 100.0) << "%)" << std::endl;

    std::cout << "\nBenchmark completed successfully!" << std::endl;

    return 0;
}
