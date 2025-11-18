/**
 * @file sentiment_analysis.cpp
 * @brief Toy example demonstrating GRU for sentiment analysis
 *
 * Note: This is a conceptual example. In practice, you would need:
 * - Word embeddings (word2vec, GloVe, etc.)
 * - Training data and backpropagation
 * - Output layer for classification
 */

#include "gru/gru.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>

// Simple word to embedding mapping (normally would use word2vec/GloVe)
class WordEmbedding {
public:
    WordEmbedding(std::size_t embedding_dim) : embedding_dim_(embedding_dim) {
        // Initialize some example word embeddings (random for demo)
        vocab_["good"] = gru::random_vector(embedding_dim, 0.5, 0.2);
        vocab_["great"] = gru::random_vector(embedding_dim, 0.6, 0.2);
        vocab_["excellent"] = gru::random_vector(embedding_dim, 0.7, 0.2);
        vocab_["bad"] = gru::random_vector(embedding_dim, -0.5, 0.2);
        vocab_["terrible"] = gru::random_vector(embedding_dim, -0.6, 0.2);
        vocab_["awful"] = gru::random_vector(embedding_dim, -0.7, 0.2);
        vocab_["okay"] = gru::random_vector(embedding_dim, 0.0, 0.2);
        vocab_["movie"] = gru::random_vector(embedding_dim, 0.0, 0.1);
        vocab_["film"] = gru::random_vector(embedding_dim, 0.0, 0.1);
        vocab_["the"] = gru::random_vector(embedding_dim, 0.0, 0.05);
    }

    gru::Vector get_embedding(const std::string& word) const {
        auto it = vocab_.find(word);
        if (it != vocab_.end()) {
            return it->second;
        }
        // Return zero vector for unknown words
        return gru::zeros(embedding_dim_);
    }

    std::vector<gru::Vector> encode_sentence(const std::string& sentence) const {
        std::istringstream iss(sentence);
        std::string word;
        std::vector<gru::Vector> embeddings;

        while (iss >> word) {
            // Convert to lowercase
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            embeddings.push_back(get_embedding(word));
        }

        return embeddings;
    }

private:
    std::size_t embedding_dim_;
    std::map<std::string, gru::Vector> vocab_;
};

// Simple sentiment classifier (normally would have a trained output layer)
double classify_sentiment(const gru::Vector& hidden_state) {
    // For demo: just take the mean of the hidden state
    // Positive values = positive sentiment, negative = negative sentiment
    double sum = 0.0;
    for (double val : hidden_state) {
        sum += val;
    }
    return sum / static_cast<double>(hidden_state.size());
}

int main() {
    std::cout << "=== Sentiment Analysis Example (Toy Demo) ===\n\n";

    constexpr std::size_t embedding_dim = 16;
    constexpr std::size_t hidden_size = 32;

    // Create word embedding and GRU
    WordEmbedding embedder(embedding_dim);
    gru::GRU gru_cell(embedding_dim, hidden_size);

    // Test sentences
    std::vector<std::string> sentences = {
        "good movie",
        "terrible film",
        "great excellent",
        "bad awful",
        "okay movie"
    };

    std::cout << "Processing sentences:\n\n";

    for (const auto& sentence : sentences) {
        // Encode sentence
        auto embeddings = embedder.encode_sentence(sentence);

        // Process through GRU
        gru_cell.reset_state();
        auto output = gru_cell.forward_sequence(embeddings, false);

        // Classify
        double sentiment = classify_sentiment(output[0]);

        std::cout << std::setw(25) << std::left << ("\"" + sentence + "\"")
                  << " -> sentiment: " << std::fixed << std::setprecision(4)
                  << sentiment
                  << " (" << (sentiment > 0 ? "positive" : sentiment < 0 ? "negative" : "neutral") << ")\n";
    }

    std::cout << "\nNote: This is a toy example for demonstration purposes.\n";
    std::cout << "A real sentiment classifier would require:\n";
    std::cout << "  - Proper word embeddings (word2vec, GloVe, etc.)\n";
    std::cout << "  - Training data with labeled examples\n";
    std::cout << "  - Backpropagation for learning\n";
    std::cout << "  - Output layer for classification\n";

    std::cout << "\nExample completed successfully!\n";
    return 0;
}
