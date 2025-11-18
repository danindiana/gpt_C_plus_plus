# REALM Architecture Documentation

## Overview

The REALM (Retrieval-Augmented Language Model) implementation follows a modular architecture that separates concerns into distinct components while maintaining clean interfaces between them.

## Core Components

### 1. Type System (`types.hpp`)

The foundation of the project, providing:
- `DenseVector<T>`: Generic dense vector implementation with dot product, norm, and normalization
- `Document`: Text document representation with automatic tokenization
- `Query`: Query representation
- `HyperParams`: Centralized hyperparameter configuration
- `TrainingStats`: Training metrics tracking

### 2. Document Encoding (`document_encoder.hpp`)

Responsible for converting text into dense vector representations.

**Interfaces:**
- `IDocumentEncoder`: Abstract interface for all encoders

**Implementations:**
- `SimpleEncoder`: Bag-of-words style encoder with learned embeddings
- `BERTEncoder`: Placeholder for BERT-based encoding (requires external BERT model)

**Design Pattern:** Strategy pattern for encoder flexibility

### 3. MIPS - Maximum Inner Product Search (`mips.hpp`)

Efficient similarity search infrastructure.

**Interface:**
- `IMIPSRetriever`: Abstract retrieval interface

**Implementations:**

#### BruteForceMIPS
- **Algorithm**: Exhaustive search
- **Time Complexity**: O(nd) per query
- **Space Complexity**: O(nd)
- **Use Case**: Small datasets (<10K documents), baseline
- **Accuracy**: 100%

#### AsymmetricLSH
- **Algorithm**: Locality Sensitive Hashing with asymmetric transformations
- **Time Complexity**: O(log n) average per query
- **Space Complexity**: O(n^{1+ρ})
- **Use Case**: Large datasets (>100K documents)
- **Accuracy**: Configurable (typically 90-98%)

**Transformations:**
```
Query:    P(q) = [q; ||q||²/2; ||q||⁴/4]
Document: Q(x) = [x; 1; 1]
```

This asymmetric transformation converts inner product maximization to approximate nearest neighbor search.

#### FAISSRetriever
- **Algorithm**: Delegates to FAISS library
- **Use Case**: Production systems with millions of documents
- **Requires**: FAISS library linkage

### 4. Retriever Component (`retriever.hpp`)

Orchestrates document encoding and retrieval.

**Responsibilities:**
- Document indexing using encoder
- Query processing
- Top-k document retrieval via MIPS
- Retrieval loss computation for training

**Key Methods:**
```cpp
void index_documents(const std::vector<Document>&);
std::vector<RetrievalResult> retrieve(const Query&);
Float compute_retrieval_loss(query_vec, relevant_doc_ids);
```

**Training:**
- `RetrieverTrainer`: Handles supervised training with query-document relevance labels
- Loss: Negative log-likelihood of relevant documents

### 5. Generator Component (`generator.hpp`)

Generates text conditioned on query and retrieved documents.

**Responsibilities:**
- Masked language modeling (MLM)
- Text generation from query + context
- MLM loss computation

**Key Methods:**
```cpp
GenerationResult generate(query, retrieved_docs);
std::vector<std::string> predict_masked_tokens(input, context);
Float compute_mlm_loss(input, positions, targets, context);
```

**Training:**
- `GeneratorTrainer`: Handles MLM training
- Masking Strategy: 15% random token masking
- Loss: Cross-entropy on masked positions

### 6. RAG Model (`rag_model.hpp`)

Top-level orchestration of retrieval and generation.

**Architecture:**
```
Query → Retriever → Top-K Docs → Generator → Answer
```

**Training Modes:**

#### Separate Pre-training
1. Pre-train retriever on query-document pairs
2. Pre-train generator on masked language modeling
3. Optional joint fine-tuning

#### Joint Training
- Simultaneous training of retriever and generator
- Allows backpropagation through retrieval
- More computationally intensive

**Configuration:**
```cpp
TrainingConfig {
    bool joint_training;
    size_t retriever_pretrain_epochs;
    size_t generator_pretrain_epochs;
    size_t joint_training_epochs;
}
```

## Data Flow

### Indexing Phase

```
Documents → Encoder → Document Vectors → MIPS Index
```

1. Documents are tokenized
2. Encoder converts tokens to vectors
3. Vectors are indexed in MIPS structure
4. Index is built/optimized

### Retrieval Phase

```
Query → Encoder → Query Vector → MIPS Search → Top-K Document IDs
```

1. Query is tokenized
2. Encoder converts to query vector
3. MIPS finds top-k highest inner products
4. Document IDs and scores returned

### Generation Phase

```
Query + Retrieved Docs → Generator → Generated Text
```

1. Retrieved documents provide context
2. Generator conditions on query + context
3. Text is generated (currently placeholder)
4. Confidence score computed

## Training Pipeline

### Phase 1: Data Preparation
- Collect document corpus
- Create query-document relevance labels
- Split into train/validation/test sets

### Phase 2: Retriever Pre-training
```
for each epoch:
    for each batch of (query, relevant_docs):
        1. Encode query
        2. Retrieve top-k documents
        3. Compute retrieval loss
        4. Backpropagate and update encoder
```

**Loss Function:**
```
L_retrieval = -∑ log P(d_relevant | q)
where P(d | q) ∝ exp(q · d)
```

### Phase 3: Generator Pre-training
```
for each epoch:
    for each batch of documents:
        1. Create masked version
        2. Retrieve context documents
        3. Predict masked tokens
        4. Compute MLM loss
        5. Backpropagate and update
```

**Loss Function:**
```
L_MLM = -∑ log P(t_i | context, position_i)
```

### Phase 4: Joint Fine-tuning (Optional)
```
for each epoch:
    1. Sample query
    2. Retrieve documents (differentiable)
    3. Generate answer
    4. Compute combined loss
    5. Backpropagate through entire pipeline
```

**Combined Loss:**
```
L_total = λ₁ * L_retrieval + λ₂ * L_MLM
```

## Design Patterns

### Strategy Pattern
- Used for encoders (`IDocumentEncoder`)
- Used for MIPS (`IMIPSRetriever`)
- Allows easy swapping of implementations

### Template Method Pattern
- Used in trainers for epoch/batch iteration
- Allows customization of loss computation

### Factory Pattern (Implicit)
- Model construction through shared pointers
- Flexible component composition

### Dependency Injection
- Components receive dependencies via constructor
- Improves testability and modularity

## Memory Management

### Ownership Model
- **Unique Pointers**: For sole ownership (MIPS retriever)
- **Shared Pointers**: For shared ownership (Encoder, Retriever, Generator)
- **References**: For non-owning access

### Vector Storage
- Document vectors cached for fast access
- Trade-off: Memory vs. re-encoding cost
- For large datasets, consider external storage

## Threading and Concurrency

### Current State
- Single-threaded implementation
- Thread-safe for read-only operations

### Future Enhancements
- Parallel document encoding
- Concurrent MIPS search
- Parallel batch processing in training

## Error Handling

### Exceptions
- `std::invalid_argument`: Dimension mismatches, invalid parameters
- `std::runtime_error`: Index not built, file I/O errors

### Validation
- Dimension checks on vector operations
- Null pointer checks
- Range validation

## Performance Considerations

### Hot Paths
1. MIPS search (query time bottleneck)
2. Vector dot products
3. Document encoding

### Optimizations
- Vectorization of dot products (compiler auto-vectorization)
- Normalized vectors for faster similarity computation
- Batch processing to amortize overhead
- Early stopping in ALSH

### Profiling Recommendations
- Profile with `perf` on Linux
- Use compiler flags: `-O3 -march=native`
- Consider SIMD intrinsics for critical loops

## Extension Points

### Adding New Encoder
1. Implement `IDocumentEncoder` interface
2. Add to encoder factory/construction
3. Update tests

### Adding New MIPS Method
1. Implement `IMIPSRetriever` interface
2. Implement `add_document`, `build_index`, `search`
3. Add benchmarks

### Custom Loss Functions
1. Extend trainer classes
2. Override loss computation methods
3. Ensure gradient compatibility

## Testing Strategy

### Unit Tests
- Each component tested independently
- Mock dependencies where appropriate
- Test edge cases and error conditions

### Integration Tests
- Test component interactions
- Verify data flow through pipeline
- Check training convergence

### Performance Tests
- Benchmark MIPS implementations
- Profile memory usage
- Measure training throughput

## Future Work

### Planned Enhancements
1. **True BERT Integration**: Replace placeholder with actual BERT encoder
2. **Differentiable Retrieval**: Enable gradient flow through retrieval
3. **GPU Acceleration**: CUDA kernels for vector operations
4. **Distributed Training**: Multi-GPU and multi-machine support
5. **Production Serving**: Optimized inference serving infrastructure
6. **Advanced MIPS**: Product quantization, graph-based search
7. **Incremental Indexing**: Add/remove documents without full rebuild

### Research Directions
- Learned hashing functions
- Neural retrieval models
- Multi-modal retrieval (text + images)
- Federated learning for privacy
