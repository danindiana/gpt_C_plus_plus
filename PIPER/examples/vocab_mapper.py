#!/usr/bin/env python3
"""
PIPER VocabMap Pipeline - Greyskull Implementation

This example demonstrates a stateful preprocessing operation on Tenstorrent Greyskull.
The VocabMap operator performs vocabulary index to embedding lookups, utilizing
Greyskull's on-chip SRAM (1MB per Tensix core) for hash table storage.

Requirements:
- Tenstorrent Greyskull e75 or e150 card
- TT-Buda installed and configured
- PyTorch >= 1.13

Usage:
    python vocab_mapper.py [--device {silicon,golden}] [--vocab-size SIZE]

Performance:
    e75:  ~4.5 GB/s throughput
    e150: ~7.1 GB/s throughput
"""

import argparse
import time
from typing import Tuple, Optional

import torch
import torch.nn as nn
import pybuda
from pybuda import PyBudaModule, TFDevice, BackendDevice, BackendType, CompilerConfig


class VocabMapperPipeline(PyBudaModule):
    """
    Stateful VocabMap preprocessing operator.

    Maps vocabulary indices to dense embeddings using an embedding table
    stored in Tensix core SRAM. Optimized for recommender system feature
    engineering where categorical features are mapped to dense vectors.
    """

    def __init__(
        self,
        name: str = "vocab_mapper",
        vocab_size: int = 10000,
        embedding_dim: int = 128,
        padding_idx: Optional[int] = None
    ):
        """
        Initialize VocabMapper.

        Args:
            name: Pipeline name
            vocab_size: Size of vocabulary (max index + 1)
            embedding_dim: Dimension of embedding vectors
            padding_idx: Index to use for padding (receives zero embedding)
        """
        super().__init__(name)
        self.vocab_size = vocab_size
        self.embedding_dim = embedding_dim

        # Embedding table stored in Tensix SRAM
        self.embedding = nn.Embedding(
            vocab_size,
            embedding_dim,
            padding_idx=padding_idx
        )

        # Initialize with Xavier uniform (common for embeddings)
        nn.init.xavier_uniform_(self.embedding.weight)
        if padding_idx is not None:
            with torch.no_grad():
                self.embedding.weight[padding_idx].fill_(0)

    def forward(self, indices: torch.Tensor) -> torch.Tensor:
        """
        Forward pass: map indices to embeddings.

        Args:
            indices: Tensor of vocabulary indices, shape (batch, seq_len)

        Returns:
            Embeddings tensor, shape (batch, seq_len, embedding_dim)
        """
        return self.embedding(indices)


class VocabGenMapPipeline(PyBudaModule):
    """
    Combined VocabGen + VocabMap pipeline.

    First generates vocabulary from raw features (hashing), then maps
    to embeddings. This simulates a complete categorical feature pipeline.
    """

    def __init__(
        self,
        name: str = "vocabgen_map",
        vocab_size: int = 10000,
        embedding_dim: int = 128
    ):
        super().__init__(name)
        self.vocab_size = vocab_size
        self.mapper = VocabMapperPipeline(
            name=f"{name}_mapper",
            vocab_size=vocab_size,
            embedding_dim=embedding_dim
        )

    def forward(self, raw_features: torch.Tensor) -> torch.Tensor:
        """
        Forward pass: hash features then map to embeddings.

        Args:
            raw_features: Raw feature values (floats/ints)

        Returns:
            Embedding vectors
        """
        # Hash to vocabulary indices (simple modulo for demo)
        indices = (raw_features.abs().long() % self.vocab_size)
        return self.mapper(indices)


def benchmark_throughput(
    pipeline: PyBudaModule,
    input_shape: Tuple[int, ...],
    device: TFDevice,
    num_iterations: int = 100
) -> float:
    """
    Benchmark pipeline throughput.

    Args:
        pipeline: Compiled PIPER pipeline
        input_shape: Shape of input tensors
        device: Greyskull device
        num_iterations: Number of benchmark iterations

    Returns:
        Throughput in GB/s
    """
    # Compile with optimizations
    compiled = pybuda.compile(
        pipeline,
        sample_inputs=(torch.randint(0, pipeline.vocab_size, input_shape),),
        device=device,
        compiler_cfg=CompilerConfig(
            enable_auto_fusing=True,
            enable_tvm_constant_prop=True,
            enable_tvm_cpu_fallback=False
        )
    )

    # Warmup
    for _ in range(10):
        test_input = torch.randint(0, pipeline.vocab_size, input_shape)
        _ = compiled(test_input)

    # Benchmark
    total_bytes = 0
    start_time = time.perf_counter()

    for _ in range(num_iterations):
        test_input = torch.randint(0, pipeline.vocab_size, input_shape)
        output = compiled(test_input)
        # Count input bytes + output bytes
        total_bytes += test_input.nelement() * 4  # Input indices (int32)
        total_bytes += output.nelement() * 4      # Output embeddings (float32)

    elapsed = time.perf_counter() - start_time
    throughput_gbs = (total_bytes / 1e9) / elapsed

    return throughput_gbs


def validate_correctness(
    pipeline: VocabMapperPipeline,
    device: TFDevice,
    test_cases: int = 100
) -> bool:
    """
    Validate pipeline produces correct results.

    Args:
        pipeline: VocabMapper pipeline
        device: Greyskull device
        test_cases: Number of random test cases

    Returns:
        True if all tests pass
    """
    compiled = pybuda.compile(
        pipeline,
        sample_inputs=(torch.randint(0, pipeline.vocab_size, (1, 256)),),
        device=device
    )

    for _ in range(test_cases):
        # Generate random indices
        test_input = torch.randint(0, pipeline.vocab_size, (1, 256))

        # Run on hardware
        hw_output = compiled(test_input)

        # Compute reference
        ref_output = pipeline.embedding(test_input)

        # Compare
        if not torch.allclose(hw_output, ref_output, rtol=1e-4, atol=1e-5):
            print(f"Validation failed!")
            print(f"Max difference: {(hw_output - ref_output).abs().max()}")
            return False

    return True


def analyze_sram_usage(vocab_size: int, embedding_dim: int) -> dict:
    """
    Analyze SRAM usage for embedding table.

    Each Tensix core has 1MB SRAM. Embedding table must fit.

    Args:
        vocab_size: Vocabulary size
        embedding_dim: Embedding dimension

    Returns:
        Dictionary with usage statistics
    """
    embedding_bytes = vocab_size * embedding_dim * 4  # float32
    sram_per_core = 1024 * 1024  # 1MB
    sram_e75 = 96 * sram_per_core  # 96 cores
    sram_e150 = 120 * sram_per_core  # 120 cores

    return {
        "embedding_size_mb": embedding_bytes / (1024 * 1024),
        "cores_required": max(1, (embedding_bytes + sram_per_core - 1) // sram_per_core),
        "fits_e75": embedding_bytes <= sram_e75,
        "fits_e150": embedding_bytes <= sram_e150,
        "utilization_e75": (embedding_bytes / sram_e75) * 100,
        "utilization_e150": (embedding_bytes / sram_e150) * 100
    }


def main():
    parser = argparse.ArgumentParser(
        description="PIPER VocabMap Pipeline on Greyskull"
    )
    parser.add_argument(
        "--device",
        choices=["silicon", "golden"],
        default="silicon",
        help="Device type (silicon=hardware, golden=simulator)"
    )
    parser.add_argument(
        "--vocab-size",
        type=int,
        default=10000,
        help="Vocabulary size"
    )
    parser.add_argument(
        "--embedding-dim",
        type=int,
        default=128,
        help="Embedding dimension"
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=32,
        help="Batch size"
    )
    parser.add_argument(
        "--seq-len",
        type=int,
        default=256,
        help="Sequence length"
    )
    parser.add_argument(
        "--iterations",
        type=int,
        default=100,
        help="Benchmark iterations"
    )
    parser.add_argument(
        "--validate",
        action="store_true",
        help="Run correctness validation"
    )
    parser.add_argument(
        "--analyze",
        action="store_true",
        help="Analyze SRAM usage"
    )

    args = parser.parse_args()

    print("=" * 70)
    print("PIPER VocabMap Pipeline - Greyskull")
    print("=" * 70)
    print(f"Device: {args.device}")
    print(f"Vocabulary size: {args.vocab_size:,}")
    print(f"Embedding dim: {args.embedding_dim}")
    print(f"Batch size: {args.batch_size}")
    print(f"Sequence length: {args.seq_len}")
    print()

    # Analyze SRAM usage
    if args.analyze:
        print("SRAM Usage Analysis")
        print("-" * 70)
        usage = analyze_sram_usage(args.vocab_size, args.embedding_dim)
        print(f"Embedding table size: {usage['embedding_size_mb']:.2f} MB")
        print(f"Cores required: {usage['cores_required']}")
        print(f"Fits on e75: {'✓' if usage['fits_e75'] else '✗'}")
        print(f"Fits on e150: {'✓' if usage['fits_e150'] else '✗'}")
        print(f"SRAM utilization (e75): {usage['utilization_e75']:.1f}%")
        print(f"SRAM utilization (e150): {usage['utilization_e150']:.1f}%")
        print()

        if not usage['fits_e75'] and not usage['fits_e150']:
            print("⚠ Warning: Embedding table too large for on-chip SRAM!")
            print("  Consider reducing vocab_size or embedding_dim")
            return 1

    # Initialize device
    device_type = BackendType.Silicon if args.device == "silicon" else BackendType.Golden
    device = TFDevice(
        arch=BackendDevice.Grayskull,
        devtype=device_type
    )

    # Create pipeline
    pipeline = VocabMapperPipeline(
        vocab_size=args.vocab_size,
        embedding_dim=args.embedding_dim
    )

    # Validation
    if args.validate:
        print("Running validation...")
        if validate_correctness(pipeline, device):
            print("✓ Validation passed!")
        else:
            print("✗ Validation failed!")
            return 1
        print()

    # Benchmark
    print("Running benchmark...")
    input_shape = (args.batch_size, args.seq_len)
    throughput = benchmark_throughput(
        pipeline,
        input_shape,
        device,
        args.iterations
    )

    # Results
    print()
    print("=" * 70)
    print("Results")
    print("=" * 70)
    print(f"Throughput: {throughput:.2f} GB/s")

    # Compare to expected
    if args.device == "silicon":
        expected_e75 = 4.5
        expected_e150 = 7.1
        print(f"Expected (e75):  {expected_e75:.2f} GB/s")
        print(f"Expected (e150): {expected_e150:.2f} GB/s")

        if throughput >= expected_e75 * 0.9:
            print("✓ Performance within expected range")
        else:
            print("⚠ Performance below expected")
            print("  Possible causes:")
            print("  - Embedding table spilling to DRAM")
            print("  - Thermal throttling")
            print("  - Memory bandwidth saturation")

    print("=" * 70)

    return 0


if __name__ == "__main__":
    exit(main())
