# Performance Tuning Guide

## Optimization Checklist

### Hardware Configuration

- [ ] PCIe slot is Gen 4 x16 (not x8 or x4)
- [ ] HugePages allocated (16GB minimum)
- [ ] Adequate cooling (≥30 CFM airflow)
- [ ] Power supply sufficient (750W for e75, 1000W for e150)

### Software Configuration

- [ ] Latest firmware flashed
- [ ] TT-KMD driver loaded
- [ ] ARCH_NAME=grayskull set
- [ ] TT_METAL_HOME configured

### Monitoring

Check these during execution:
```bash
# Temperature (should be <75°C)
tt-smi --temperature

# Power (should be <75W for e75, <200W for e150)
tt-smi --power

# PCIe link
lspci -vvv -s $(lspci | grep Tenstorrent | cut -d' ' -f1) | grep LnkSta
```

## Common Performance Issues

### Issue: Low Throughput

**Symptoms**: 50% or less of expected performance

**Diagnosis**:
```bash
# Check thermal throttling
tt-smi --temperature

# Check PCIe link speed
lspci -vvv -s XX:XX.X | grep "Speed"

# Check memory bandwidth
cd ~/tt-metal
./build/test/tt_metal/perf_microbenchmark/dram_read
```

**Solutions**:
1. Improve cooling
2. Verify PCIe Gen 4 x16
3. Increase HugePages allocation
4. Close background applications

### Issue: High Latency

**Symptoms**: Long startup time, slow first iteration

**Causes**:
- PCIe latency
- Compilation overhead
- Cold cache

**Solutions**:
```python
# Pre-compile models
compiled = pybuda.compile(pipeline, ...)

# Warmup runs
for _ in range(10):
    output = compiled(input)

# Then benchmark
```

### Issue: Memory Errors

**Symptoms**: "Failed to allocate" errors

**Diagnosis**:
```bash
# Check HugePages
grep HugePages /proc/meminfo

# Check SRAM usage
python -c "
vocab_size = 10000
embedding_dim = 128
sram_mb = (vocab_size * embedding_dim * 4) / (1024 * 1024)
print(f'SRAM required: {sram_mb:.2f} MB')
print(f'Available (e75): 96 MB')
print(f'Available (e150): 120 MB')
"
```

**Solutions**:
1. Increase HugePages: `sudo ./scripts/setup_hugepages.sh 32`
2. Reduce vocab_size or embedding_dim
3. Split large tables across multiple operators

## Optimization Techniques

### 1. Batch Size Tuning

```python
# Test different batch sizes
for batch_size in [8, 16, 32, 64, 128]:
    throughput = benchmark(batch_size)
    print(f"Batch {batch_size}: {throughput:.2f} GB/s")
```

Optimal batch sizes:
- Stateless ops: 64-128
- Stateful ops: 32-64

### 2. Kernel Fusion

Combine multiple operators:
```python
class FusedPipeline(PyBudaModule):
    def forward(self, x):
        # Fused: neg2zero + log
        x = torch.clamp(x, min=0.0)
        x = torch.log(x + 1e-7)
        return x
```

Benefits:
- Reduced memory traffic
- Fewer kernel launches
- +20-30% throughput

### 3. Data Placement

Keep hot data in SRAM:
```python
# Good: Embedding table fits in SRAM (1MB per core)
small_embedding = nn.Embedding(1000, 128)  # 0.5 MB

# Bad: Table spills to DRAM
large_embedding = nn.Embedding(100000, 512)  # 200 MB
```

### 4. Compiler Flags

Enable optimizations:
```python
compiler_cfg = CompilerConfig(
    enable_auto_fusing=True,           # Automatic kernel fusion
    enable_tvm_constant_prop=True,     # Constant propagation
    enable_tvm_cpu_fallback=False      # Force on-device
)
```

## Benchmarking Best Practices

### Proper Warmup

```python
# Warmup (excludes compilation, cold cache)
for _ in range(10):
    _ = compiled(input)

# Benchmark
start = time.perf_counter()
for _ in range(100):
    output = compiled(input)
elapsed = time.perf_counter() - start
```

### Throughput Calculation

```python
# Account for both input and output
input_bytes = input.nelement() * 4
output_bytes = output.nelement() * 4
total_bytes = (input_bytes + output_bytes) * iterations
throughput_gbs = (total_bytes / 1e9) / elapsed
```

### Statistical Significance

Run multiple trials:
```python
import numpy as np

trials = 10
throughputs = []
for _ in range(trials):
    throughput = benchmark()
    throughputs.append(throughput)

mean = np.mean(throughputs)
std = np.std(throughputs)
print(f"Throughput: {mean:.2f} ± {std:.2f} GB/s")
```

## Expected Performance Ranges

### e75 (96 cores @ 1.0 GHz)

| Workload | Min | Typical | Max |
|----------|-----|---------|-----|
| Stateless | 6 GB/s | 8 GB/s | 10 GB/s |
| Stateful | 3 GB/s | 4.5 GB/s | 6 GB/s |

### e150 (120 cores @ 1.2 GHz)

| Workload | Min | Typical | Max |
|----------|-----|---------|-----|
| Stateless | 10 GB/s | 12 GB/s | 14 GB/s |
| Stateful | 5 GB/s | 7 GB/s | 9 GB/s |

## Profiling Tools

```bash
# TT-Metal profiler
export TT_METAL_PROFILER=1
python your_script.py

# Check output
cat generated/profiler/ops_perf_results.csv
```

## See Also

- [Architecture](architecture.md)
- [Operators](operators.md)
- [Troubleshooting](../readme.md#troubleshooting)
