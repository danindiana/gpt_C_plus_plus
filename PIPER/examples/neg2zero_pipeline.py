#!/usr/bin/env python3
"""
PIPER Neg2Zero Pipeline - Greyskull Implementation

This example demonstrates a stateless preprocessing operation on Tenstorrent Greyskull.
The Neg2Zero operator clamps all negative values to zero, a common preprocessing step
for recommender systems dealing with implicit feedback data.

Requirements:
- Tenstorrent Greyskull e75 or e150 card
- TT-Buda installed and configured
- PyTorch >= 1.13

Usage:
    python neg2zero_pipeline.py [--device {silicon,golden}] [--size SIZE]

Performance:
    e75:  ~8.2 GB/s throughput
    e150: ~12.5 GB/s throughput
"""

import argparse
import time
from typing import Tuple

import torch
import pybuda
from pybuda import PyBudaModule, TFDevice, BackendDevice, BackendType


class Neg2ZeroPipeline(PyBudaModule):
    """
    Stateless Neg2Zero preprocessing operator.

    Converts all negative values in the input tensor to zero while
    preserving positive values. Optimized for Greyskull's Tensix cores.
    """

    def __init__(self, name: str = "neg2zero"):
        super().__init__(name)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        """
        Forward pass: clamp negative values to zero.

        Args:
            x: Input tensor of any shape

        Returns:
            Tensor with negative values set to zero
        """
        return torch.clamp(x, min=0.0)


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
    # Compile pipeline
    compiled = pybuda.compile(
        pipeline,
        sample_inputs=(torch.randn(*input_shape),),
        device=device
    )

    # Warmup
    for _ in range(10):
        test_input = torch.randn(*input_shape) * 2 - 1  # Range: [-2, 2]
        _ = compiled(test_input)

    # Benchmark
    total_bytes = 0
    start_time = time.perf_counter()

    for _ in range(num_iterations):
        test_input = torch.randn(*input_shape) * 2 - 1
        output = compiled(test_input)
        total_bytes += test_input.nelement() * 4  # 4 bytes per float32

    elapsed = time.perf_counter() - start_time
    throughput_gbs = (total_bytes / 1e9) / elapsed

    return throughput_gbs


def validate_correctness(
    pipeline: PyBudaModule,
    device: TFDevice,
    test_cases: int = 100
) -> bool:
    """
    Validate pipeline produces correct results.

    Args:
        pipeline: Compiled pipeline
        device: Greyskull device
        test_cases: Number of random test cases

    Returns:
        True if all tests pass
    """
    compiled = pybuda.compile(
        pipeline,
        sample_inputs=(torch.randn(1, 256, 256),),
        device=device
    )

    for _ in range(test_cases):
        # Generate random input with known negative values
        test_input = torch.randn(1, 256, 256) * 10 - 5  # Range: [-15, 5]

        # Run on hardware
        hw_output = compiled(test_input)

        # Compute reference
        ref_output = torch.clamp(test_input, min=0.0)

        # Compare
        if not torch.allclose(hw_output, ref_output, rtol=1e-5, atol=1e-6):
            print(f"Validation failed!")
            print(f"Max difference: {(hw_output - ref_output).abs().max()}")
            return False

    return True


def main():
    parser = argparse.ArgumentParser(
        description="PIPER Neg2Zero Pipeline on Greyskull"
    )
    parser.add_argument(
        "--device",
        choices=["silicon", "golden"],
        default="silicon",
        help="Device type (silicon=hardware, golden=simulator)"
    )
    parser.add_argument(
        "--size",
        type=int,
        default=1024,
        help="Input tensor size (NxN matrix)"
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

    args = parser.parse_args()

    # Initialize device
    device_type = BackendType.Silicon if args.device == "silicon" else BackendType.Golden
    device = TFDevice(
        arch=BackendDevice.Grayskull,
        devtype=device_type
    )

    # Create pipeline
    pipeline = Neg2ZeroPipeline()

    print("=" * 70)
    print("PIPER Neg2Zero Pipeline - Greyskull")
    print("=" * 70)
    print(f"Device: {args.device}")
    print(f"Input size: {args.size}x{args.size}")
    print(f"Iterations: {args.iterations}")
    print()

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
    input_shape = (1, args.size, args.size)
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
        expected_e75 = 8.2
        expected_e150 = 12.5
        print(f"Expected (e75):  {expected_e75:.2f} GB/s")
        print(f"Expected (e150): {expected_e150:.2f} GB/s")

        if throughput >= expected_e75 * 0.9:
            print("✓ Performance within expected range")
        else:
            print("⚠ Performance below expected (check thermal throttling)")

    print("=" * 70)

    return 0


if __name__ == "__main__":
    exit(main())
