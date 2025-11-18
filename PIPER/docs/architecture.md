# PIPER Architecture on Greyskull

## Overview

PIPER (Pipeline Processor for Efficient Recommender systems) is a preprocessing accelerator architecture adapted for Tenstorrent Greyskull's RISC-V Tensix cores.

## Hardware Architecture

### Greyskull Device Layout

```
┌─────────────────────────────────────────────────┐
│ Greyskull e75/e150                              │
├─────────────────────────────────────────────────┤
│                                                 │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐         │
│  │ Tensix  │  │ Tensix  │  │ Tensix  │  ...    │
│  │  Core   │  │  Core   │  │  Core   │         │
│  │  (0,0)  │  │  (0,1)  │  │  (0,2)  │         │
│  └─────────┘  └─────────┘  └─────────┘         │
│       │            │            │               │
│  ┌────┴────────────┴────────────┴──────┐        │
│  │    Network-on-Chip (NoC)            │        │
│  └────┬────────────┬────────────┬──────┘        │
│       │            │            │               │
│  ┌────┴────────────┴────────────┴──────┐        │
│  │    8GB LPDDR4 Memory                │        │
│  │    102-118 GB/s Bandwidth           │        │
│  └─────────────────────────────────────┘        │
│                                                 │
└─────────────────────────────────────────────────┘
         │
    PCIe Gen 4 x16
         │
┌────────┴──────────┐
│   Host System     │
└───────────────────┘
```

### Tensix Core Internals

Each Tensix core contains:
- **5× RISC-V Processors**: General computation and control
- **Tensor Math Unit**: Matrix/vector operations
- **1MB SRAM**: Low-latency storage for hot data
- **NoC Router**: Inter-core communication
- **Compression Engine**: Bandwidth optimization

## Software Stack

### Layer Hierarchy

| Layer | Component | Purpose |
|-------|-----------|---------|
| **Application** | PIPER Pipelines | User preprocessing workflows |
| **Framework** | TT-Buda/PyBuda | Model compilation, graph optimization |
| **Runtime** | TT-NN | Neural network operators |
| **SDK** | TT-Metalium | Low-level kernel programming |
| **Driver** | TT-KMD | Kernel-mode device driver |
| **Firmware** | TT-Firmware | On-device firmware |
| **Hardware** | Greyskull | Physical device |

## PIPER Operator Implementation

### Stateless Operators

**Neg2Zero** - Clamp negative values
- Fully parallelizable across cores
- Single-pass streaming
- ~8-12 GB/s throughput

**Logarithm** - Natural logarithm
- Utilizes Tensix math units
- Hardware transcendental functions
- ~8-12 GB/s throughput

### Stateful Operators

**VocabMap** - Embedding lookup
- Embedding table in SRAM
- Hash-based indexing
- ~4-7 GB/s throughput

## Memory Hierarchy

```
┌──────────────────────────────────────────┐
│  Host DRAM (64-128 GB)                   │  Slowest, Largest
│  ↕ PCIe Gen 4 (32 GB/s bidirectional)    │
├──────────────────────────────────────────┤
│  Device LPDDR4 (8 GB, 102-118 GB/s)      │
│  ↕ NoC                                   │
├──────────────────────────────────────────┤
│  Tensix SRAM (96-120 MB total)           │  Fastest, Smallest
│  (1 MB per core, 200 MHz)                │
└──────────────────────────────────────────┘
```

## Data Flow

1. **Input**: Data loaded from host to LPDDR4 via DMA
2. **Distribution**: NoC distributes work to Tensix cores
3. **Processing**: Cores execute PIPER operators
4. **Collection**: Results aggregated via NoC
5. **Output**: Data returned to host via DMA

## Performance Characteristics

### Bottlenecks

1. **PCIe Bandwidth**: 32 GB/s limits host ↔ device transfers
2. **LPDDR4 Bandwidth**: 102-118 GB/s limits DRAM access
3. **SRAM Capacity**: 1 MB/core limits table sizes

### Optimization Strategies

- **Kernel Fusion**: Combine operators to reduce memory traffic
- **Data Reuse**: Keep hot data in SRAM
- **Pipeline Overlap**: Overlap compute with data movement
- **Batching**: Amortize PCIe latency over larger transfers

## See Also

- [Operator Reference](operators.md)
- [Performance Tuning Guide](performance.md)
- [Migration Guide](migration.md)
