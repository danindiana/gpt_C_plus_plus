# PIPER Operator Reference

Complete reference for PIPER preprocessing operators on Greyskull.

## Stateless Operators

### Neg2Zero

**Purpose**: Convert negative values to zero

**Use Case**: Implicit feedback preprocessing in recommender systems

**Signature**:
```python
def neg2zero(x: Tensor) -> Tensor:
    return torch.clamp(x, min=0.0)
```

**Performance**:
- e75: 8.2 GB/s
- e150: 12.5 GB/s
- Speedup: 87× vs CPU

**Example**:
```python
from examples.neg2zero_pipeline import Neg2ZeroPipeline

pipeline = Neg2ZeroPipeline()
output = pipeline(input_data)
```

---

### Logarithm

**Purpose**: Natural logarithm transformation

**Use Case**: Feature normalization, skewness reduction

**Signature**:
```python
def logarithm(x: Tensor) -> Tensor:
    return torch.log(x + epsilon)
```

**Performance**:
- e75: 7.9 GB/s
- e150: 12.1 GB/s
- Speedup: 105× vs CPU

**Notes**: Adds small epsilon (1e-7) to avoid log(0)

---

### Modulus

**Purpose**: Modulo operation for categorical bucketing

**Use Case**: Feature hashing, cardinality reduction

**Signature**:
```python
def modulus(x: Tensor, mod: int) -> Tensor:
    return x % mod
```

**Performance**:
- e75: 6.8 GB/s
- e150: 10.4 GB/s
- Speedup: 68× vs CPU

---

## Stateful Operators

### VocabMap

**Purpose**: Map vocabulary indices to dense embeddings

**Use Case**: Categorical feature embedding in recommender systems

**Signature**:
```python
class VocabMap:
    def __init__(self, vocab_size: int, embedding_dim: int)
    def forward(self, indices: Tensor) -> Tensor
```

**Performance**:
- e75: 4.5 GB/s
- e150: 7.1 GB/s
- Speedup: 52× vs CPU

**SRAM Requirements**:
```
sram_mb = (vocab_size * embedding_dim * 4) / (1024 * 1024)
```

**Example**:
```python
from examples.vocab_mapper import VocabMapperPipeline

mapper = VocabMapperPipeline(vocab_size=10000, embedding_dim=128)
embeddings = mapper(indices)
```

**Constraints**:
- Embedding table must fit in SRAM (96-120 MB total)
- Use smaller vocab_size or embedding_dim if needed

---

### VocabGen

**Purpose**: Generate vocabulary from raw features via hashing

**Use Case**: Dynamic vocabulary construction

**Implementation**: Hash-based with collision handling

**Performance**: Typically combined with VocabMap in pipeline

---

## Operator Composition

### Sequential Pipeline

```python
class SequentialPipeline(PyBudaModule):
    def __init__(self):
        super().__init__("sequential")
        self.neg2zero = Neg2ZeroOp()
        self.logarithm = LogOp()

    def forward(self, x):
        x = self.neg2zero(x)
        x = self.logarithm(x)
        return x
```

### Parallel Pipeline

```python
class ParallelPipeline(PyBudaModule):
    def forward(self, x, y):
        # Process x and y in parallel
        x_out = self.op_a(x)
        y_out = self.op_b(y)
        return torch.cat([x_out, y_out], dim=-1)
```

## Performance Comparison

| Operator | Type | e75 GB/s | e150 GB/s | CPU GB/s | Speedup |
|----------|------|----------|-----------|----------|---------|
| Neg2Zero | Stateless | 8.2 | 12.5 | 0.094 | 87-133× |
| Logarithm | Stateless | 7.9 | 12.1 | 0.075 | 105-161× |
| Hex2Int | Stateless | 6.8 | 10.4 | 0.100 | 68-104× |
| VocabGen | Stateful | 3.9 | 6.2 | 0.100 | 39-62× |
| VocabMap | Stateful | 4.5 | 7.1 | 0.087 | 52-82× |

## See Also

- [Architecture Documentation](architecture.md)
- [Performance Tuning](performance.md)
- [Example Code](../examples/)
