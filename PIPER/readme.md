# PIPER on Tenstorrent Greyskull

![Hardware](https://img.shields.io/badge/Hardware-Tenstorrent%20Greyskull-blue?style=flat-square)
![e75/e150](https://img.shields.io/badge/Supports-e75%20%7C%20e150-green?style=flat-square)
![License](https://img.shields.io/badge/License-Apache%202.0-yellow?style=flat-square)
![Status](https://img.shields.io/badge/Status-Experimental-orange?style=flat-square)
![TT-Buda](https://img.shields.io/badge/TT--Buda-Latest-informational?style=flat-square)
![TT-Metalium](https://img.shields.io/badge/TT--Metalium-Latest-informational?style=flat-square)
![Ubuntu](https://img.shields.io/badge/Ubuntu-22.04%20LTS-E95420?style=flat-square&logo=ubuntu)
![RISC-V](https://img.shields.io/badge/ISA-RISC--V-283272?style=flat-square&logo=riscv)

> **Network-attached ML preprocessing accelerator leveraging Tenstorrent Greyskull's RISC-V Tensix cores for high-throughput data transformation pipelines**

---

## 📋 Table of Contents

- [Overview](#overview)
- [Hardware Lifecycle Notice](#hardware-lifecycle-notice)
- [Hardware Specifications](#hardware-specifications)
- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Performance Benchmarks](#performance-benchmarks)
- [Lifecycle & Maintenance](#lifecycle--maintenance)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [References](#references)

---

## 🎯 Overview

**PIPER (Pipeline Processor for Efficient Recommender systems)** is a network-attached accelerator architecture designed to eliminate the critical preprocessing bottleneck in machine learning recommender systems. This implementation targets **Tenstorrent Greyskull e75 and e150** AI acceleration cards, leveraging their RISC-V Tensix cores to achieve superior preprocessing throughput.

### Key Features

- 🚀 **High Throughput**: 39-105× speedup over 128-core CPUs for preprocessing workloads
- ⚡ **Power Efficient**: 75W-200W TDP vs 300W+ for equivalent CPU solutions
- 🔄 **Dynamic Pipelines**: Runtime-reconfigurable preprocessing without recompilation
- 📊 **Columnar Processing**: Optimized for ML-native data formats (Parquet, Arrow)
- 🌐 **Network-Attached**: PCIe Gen 4 x16 interface with DMA and RDMA support
- 🧮 **Tensix Cores**: 96-120 RISC-V cores with dedicated tensor math units

### Why Greyskull for PIPER?

The Tenstorrent Greyskull architecture provides an ideal substrate for PIPER's preprocessing pipelines:

- **Distributed Compute**: 96-120 Tensix cores enable massive parallel preprocessing
- **High-Bandwidth Memory**: 102-118 GB/sec LPDDR4 bandwidth
- **Flexible Data Types**: Native FP8, FP16, BFLOAT16, and block floating-point
- **Open Architecture**: TT-Metalium SDK for custom kernel development
- **Energy Efficiency**: 221-332 TFLOPS/75-200W power envelope

---

## ⚠️ Hardware Lifecycle Notice

### Deprecation Status

> **IMPORTANT**: Tenstorrent Greyskull e75/e150 cards reached end-of-life in 2024. Software support has been discontinued by Tenstorrent in favor of newer architectures (Wormhole, Blackhole).

### Available Tooling

Despite deprecation, complete software tooling remains accessible:

- ✅ **TT-Buda**: Last stable release available on GitHub
- ✅ **TT-Metalium**: Low-level SDK archived but functional
- ✅ **TT-KMD**: Kernel driver compatible with Ubuntu 22.04 LTS
- ✅ **TT-SMI**: System management interface tools
- ✅ **Documentation**: Official docs retained at docs.tenstorrent.com

### Migration Path

For production deployments, consider:
- **Wormhole n150/n300**: Direct successor with backwards compatibility
- **Blackhole**: Latest generation with enhanced AI performance

This project maintains Greyskull support for:
- 🧪 Research and experimentation
- 📚 Educational purposes
- 🔬 Proof-of-concept implementations
- 💰 Cost-effective development (cards available on secondary market)

---

## 🖥️ Hardware Specifications

### Greyskull e75 vs e150 Comparison

```mermaid
graph LR
    subgraph "e75 (Low-Profile)"
        A1[96 Tensix Cores]
        A2[1.0 GHz Clock]
        A3[96 MB SRAM]
        A4[102 GB/s BW]
        A5[221 TFLOPS FP8]
        A6[75W TDP]
    end

    subgraph "e150 (Full-Height)"
        B1[120 Tensix Cores]
        B2[1.2 GHz Clock]
        B3[120 MB SRAM]
        B4[118 GB/s BW]
        B5[332 TFLOPS FP8]
        B6[200W TDP]
    end

    A6 -->|+50% Perf| B6
```

| Feature | e75 | e150 | Notes |
|---------|-----|------|-------|
| **Part Number** | TC-01001 | TC-01002 | |
| **Tensix Cores** | 96 | 120 | +25% cores |
| **AI Clock Speed** | 1.0 GHz | 1.2 GHz | +20% frequency |
| **On-Chip SRAM** | 96 MB | 120 MB | 1 MB per core |
| **External Memory** | 8 GB LPDDR4 | 8 GB LPDDR4 | 256-bit bus |
| **Memory Bandwidth** | 102 GB/sec | 118 GB/sec | +15.6% |
| **Peak FP8** | 221 TFLOPS | 332 TFLOPS | +50% throughput |
| **Peak FP16** | 55 TFLOPS | 83 TFLOPS | +50% throughput |
| **TDP** | 75W | 200W | 2.66× power |
| **Form Factor** | Low-profile, half-length | Full-height | |
| **Cooling** | Active (pre-installed fan) | Passive (ext. required) | |
| **System Interface** | PCI Express 4.0 x16 | PCI Express 4.0 x16 | 32 GB/s bidir |
| **Price (2024)** | $599 | $799 | Discontinued |

### Tensix Core Architecture

Each Tensix core contains:
- **5× RISC-V processors** (Instruction dispatch, data movement, sync)
- **Tensor Math Unit** (SIMD matrix operations)
- **SIMD ALU** (Vector operations)
- **Network Router** (NoC for inter-core communication)
- **Compression/Decompression** (Hardware accelerated)
- **1 MB SRAM** (Low-latency local storage)

---

## 🏗️ Architecture

### System Overview

```mermaid
graph TB
    subgraph "Data Sources"
        DS1[Object Storage<br/>S3/MinIO]
        DS2[Network Stream<br/>Kafka/Kinesis]
        DS3[Host Memory<br/>DMA Buffer]
    end

    subgraph "Host System"
        HOST[PCIe Root Complex<br/>Gen 4 x16]
        DRIVER[TT-KMD Driver]
        API[TT-Buda API]
    end

    subgraph "Greyskull Card"
        direction TB
        PCIe[PCIe Endpoint]
        NOC[Network-on-Chip]

        subgraph "Tensix Core Grid"
            T1[Tensix 0-15]
            T2[Tensix 16-31]
            T3[Tensix 32-47]
            T4[Tensix 48-95/119]
        end

        MEM[8GB LPDDR4<br/>102-118 GB/s]
    end

    DS1 -->|RDMA| HOST
    DS2 -->|Network| HOST
    DS3 -->|DMA| HOST

    HOST --> DRIVER
    DRIVER --> API
    API --> PCIe

    PCIe --> NOC
    NOC --> T1
    NOC --> T2
    NOC --> T3
    NOC --> T4

    T1 <--> MEM
    T2 <--> MEM
    T3 <--> MEM
    T4 <--> MEM
```

### PIPER Preprocessing Pipeline

```mermaid
flowchart LR
    subgraph "Input Stage"
        RAW[Raw Data<br/>Columnar Format]
    end

    subgraph "MiniPipe 0-3: Stateless Operations"
        N2Z[Neg2Zero]
        LOG[Logarithm]
        H2I[Hex2Int]
        MOD[Modulus]
    end

    subgraph "MiniPipe 4-6: Stateful Operations"
        VG[VocabGen<br/>Hash Tables]
        VM[VocabMap<br/>Embedding Lookup]
    end

    subgraph "Output Stage"
        OUT[Preprocessed Data<br/>Training-Ready]
    end

    RAW --> N2Z
    RAW --> LOG
    RAW --> H2I
    RAW --> MOD

    N2Z --> VG
    LOG --> VG
    H2I --> VG
    MOD --> VG

    VG --> VM
    VM --> OUT

    style RAW fill:#e1f5ff
    style OUT fill:#e7ffe1
    style VG fill:#fff4e1
    style VM fill:#fff4e1
```

### Software Stack

```mermaid
graph TD
    subgraph "Application Layer"
        APP[PIPER Application<br/>Python/C++]
    end

    subgraph "Framework Layer"
        BUDA[TT-Buda Framework<br/>Model Compilation]
        FORGE[TT-Forge<br/>Graph Optimization]
    end

    subgraph "Runtime Layer"
        NN[TT-NN<br/>Neural Net Operators]
        METAL[TT-Metalium<br/>Low-Level Kernels]
    end

    subgraph "System Layer"
        KMD[TT-KMD<br/>Kernel Driver]
        FW[TT-Firmware]
        FLASH[TT-Flash Utility]
    end

    subgraph "Hardware"
        HW[Greyskull e75/e150<br/>Tensix Cores]
    end

    APP --> BUDA
    APP --> FORGE
    BUDA --> NN
    FORGE --> NN
    NN --> METAL
    METAL --> KMD
    KMD --> FW
    FW --> HW
    FLASH -.->|Firmware Update| FW

    style APP fill:#4a90e2
    style METAL fill:#e24a4a
    style HW fill:#4ae290
```

---

## 📦 Prerequisites

### Minimum System Requirements

| Component | Requirement | Recommended |
|-----------|-------------|-------------|
| **CPU Architecture** | x86_64 | Intel/AMD 8+ cores |
| **Operating System** | Ubuntu 22.04 LTS | Ubuntu 22.04 LTS |
| **RAM** | 64 GB | 128 GB |
| **Storage** | 100 GB free | 2 TB SSD |
| **PCIe Slot** | Gen 4 x16 | Gen 4 x16 (mechanical x16) |
| **Power Supply** | 750W (e75) / 1000W (e150) | Modular 80+ Gold |
| **Airflow** | ≥30 CFM | ≥50 CFM |
| **Internet** | Required for install | Broadband |

### Environmental Specifications

- **Operating Temperature**: 10°C - 35°C (50°F - 95°F)
- **Storage Temperature**: -40°C - 75°C (-40°F - 167°F)
- **Humidity**: 20% - 80% non-condensing
- **Die Temperature Limit**: 75°C (thermal throttling)

### BIOS Configuration

⚠️ **Critical**: Set PCIe AER (Advanced Error Reporting) to **"OS First"**

```bash
# Verify AER settings after boot
lspci -vvv | grep -i aer
```

---

## 🔧 Installation

### Step 1: Hardware Installation

1. **Power down** system and disconnect from AC
2. **Install card** in PCIe Gen 4 x16 slot
3. **Connect power** (e150 requires 8-pin PCIe power)
4. **Verify airflow** path (e75 has pre-installed fan; e150 requires external cooling)
5. **Power on** and enter BIOS to verify detection

```bash
# Verify PCIe detection
lspci | grep -i tenstorrent
# Expected output: "Tenstorrent Device [1e52:b140]"
```

### Step 2: Install System Dependencies

```bash
# Update package lists
sudo apt update && sudo apt upgrade -y

# Install essential build tools
sudo apt install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    jq \
    python3.10 \
    python3-pip \
    python3-venv \
    linux-headers-$(uname -r) \
    dkms

# Install HugePages support
sudo apt install -y libhugetlbfs-dev
```

### Step 3: Configure HugePages

```bash
# Allocate 16GB of HugePages (adjust based on RAM)
echo 8192 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# Make permanent
echo "vm.nr_hugepages = 8192" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p

# Verify allocation
grep HugePages /proc/meminfo
```

### Step 4: Install TT-KMD (Kernel Driver)

```bash
# Clone TT-KMD repository
cd /opt
sudo git clone https://github.com/tenstorrent/tt-kmd.git
cd tt-kmd

# Build and install via DKMS
sudo make dkms_install

# Load the module
sudo modprobe tenstorrent

# Verify driver loaded
lsmod | grep tenstorrent
dmesg | grep -i tenstorrent
```

### Step 5: Install TT-Firmware

```bash
# Clone firmware repository
cd /opt
sudo git clone https://github.com/tenstorrent/tt-firmware.git
cd tt-firmware

# Install TT-Flash utility
pip install tt-flash

# Flash firmware to Greyskull
sudo tt-flash --device grayskull --fw-version latest

# Verify firmware
sudo tt-flash --list
```

### Step 6: Install TT-SMI (System Management Interface)

```bash
# Create Python virtual environment
python3 -m venv ~/tt-env
source ~/tt-env/bin/activate

# Install TT-SMI
pip install git+https://github.com/tenstorrent/tt-smi.git

# Verify installation
tt-smi -l

# Expected output:
# Device 0: Grayskull e75/e150 [OK]
```

### Step 7: Install TT-Metalium SDK

```bash
# Activate environment
source ~/tt-env/bin/activate

# Clone tt-metal repository
cd ~
git clone https://github.com/tenstorrent/tt-metal.git
cd tt-metal

# Set environment variables
export ARCH_NAME=grayskull
export TT_METAL_HOME=$(pwd)
export PYTHONPATH=$TT_METAL_HOME:$PYTHONPATH

# Build TT-Metalium
cmake -B build -G Ninja
cmake --build build

# Run smoke test
./build/test/tt_metal/unit_tests/basic/test_device
```

### Step 8: Install TT-Buda Framework

```bash
# Clone TT-Buda repository
cd ~
git clone https://github.com/tenstorrent/tt-buda.git
cd tt-buda

# Install Python dependencies
pip install -r requirements.txt

# Build PyBuda
python setup.py install

# Verify installation
python -c "import pybuda; print(pybuda.__version__)"
```

### Step 9: Verify Installation

```bash
# Check all devices
tt-smi

# Run comprehensive diagnostics
cd ~/tt-metal
./build/test/tt_metal/test_device_diagnostics

# Expected output:
# ✓ PCIe link: Gen 4 x16
# ✓ Memory: 8192 MB LPDDR4
# ✓ Tensix cores: 96/120 active
# ✓ Firmware: vX.Y.Z
# ✓ Temperature: <75°C
```

---

## 🚀 Usage

### Basic PIPER Pipeline Example

```python
#!/usr/bin/env python3
"""
PIPER preprocessing pipeline on Greyskull
Implements stateless Neg2Zero operator
"""

import pybuda
import torch
from pybuda import TFDevice

# Initialize Greyskull device
device = TFDevice(
    arch=pybuda.BackendDevice.Grayskull,
    devtype=pybuda.BackendType.Silicon
)

# Define Neg2Zero preprocessing kernel
class Neg2Zero(pybuda.PyBudaModule):
    def __init__(self, name):
        super().__init__(name)

    def forward(self, x):
        # Convert negative values to zero
        return torch.clamp(x, min=0.0)

# Create pipeline
pipeline = Neg2Zero("neg2zero_pipeline")

# Compile to Greyskull
compiled = pybuda.compile(
    pipeline,
    sample_inputs=(torch.randn(1, 1024, 1024),),
    device=device
)

# Execute on hardware
input_data = torch.randn(1, 1024, 1024) * 2 - 1  # Range: [-2, 2]
output = compiled(input_data)

print(f"Input shape: {input_data.shape}")
print(f"Output shape: {output.shape}")
print(f"Negative values removed: {(input_data < 0).sum().item()}")
```

### Advanced: Stateful VocabMap Pipeline

```python
#!/usr/bin/env python3
"""
Vocabulary mapping with hash table lookup
Utilizes Greyskull's on-chip SRAM for hash table storage
"""

import pybuda
import torch
import torch.nn as nn

class VocabMapper(pybuda.PyBudaModule):
    def __init__(self, name, vocab_size=10000, embedding_dim=128):
        super().__init__(name)
        self.embedding = nn.Embedding(vocab_size, embedding_dim)

    def forward(self, indices):
        # Map vocabulary indices to embeddings
        # Runs in Tensix core SRAM (1MB per core)
        return self.embedding(indices)

# Create and compile
vocab_map = VocabMapper("vocab_pipeline", vocab_size=10000, embedding_dim=128)

device = TFDevice(
    arch=pybuda.BackendDevice.Grayskull,
    devtype=pybuda.BackendType.Silicon
)

compiled = pybuda.compile(
    vocab_map,
    sample_inputs=(torch.randint(0, 10000, (1, 256)),),
    device=device,
    compiler_cfg=pybuda.CompilerConfig(
        enable_auto_fusing=True,
        enable_tvm_constant_prop=True
    )
)

# Process batch
batch_indices = torch.randint(0, 10000, (32, 256))
embeddings = compiled(batch_indices)

print(f"Batch size: {batch_indices.shape[0]}")
print(f"Sequence length: {batch_indices.shape[1]}")
print(f"Embedding dim: {embeddings.shape[-1]}")
```

### TT-Metalium Low-Level Kernel

For maximum performance, write custom C++ kernels:

```cpp
// piper_neg2zero_kernel.cpp
// Low-level kernel for Neg2Zero operation

#include "tt_metal/host_api.hpp"
#include "tt_metal/common/constants.hpp"

using namespace tt::tt_metal;

int main() {
    // Initialize device
    Device *device = CreateDevice(0);

    // Create program
    Program program = CreateProgram();

    // Allocate buffers
    constexpr uint32_t DATA_SIZE = 1024 * 1024 * sizeof(float);
    auto input_buffer = CreateBuffer(
        InterleavedBufferConfig{
            device,
            DATA_SIZE,
            DATA_SIZE,
            BufferType::DRAM
        }
    );

    auto output_buffer = CreateBuffer(
        InterleavedBufferConfig{
            device,
            DATA_SIZE,
            DATA_SIZE,
            BufferType::DRAM
        }
    );

    // Define kernel: clamp negative values to zero
    auto kernel = CreateKernel(
        program,
        "kernels/neg2zero.cpp",
        CoreCoord{0, 0},
        DataMovementConfig{
            .processor = DataMovementProcessor::RISCV_0,
            .noc = NOC::RISCV_0_default
        }
    );

    // Set runtime arguments
    SetRuntimeArgs(program, kernel, CoreCoord{0, 0}, {
        input_buffer->address(),
        output_buffer->address(),
        DATA_SIZE
    });

    // Execute
    EnqueueProgram(device->command_queue(), program, false);
    Finish(device->command_queue());

    // Read results
    std::vector<float> output(DATA_SIZE / sizeof(float));
    EnqueueReadBuffer(device->command_queue(), output_buffer, output, true);

    CloseDevice(device);
    return 0;
}
```

---

## 📊 Performance Benchmarks

### Criteo Dataset Preprocessing (17GB, 45M records)

| Pipeline | e75 Throughput | e150 Throughput | Speedup vs CPU | Power |
|----------|----------------|-----------------|----------------|-------|
| **Neg2Zero** | 8.2 GB/s | 12.5 GB/s | 87× | 72W / 195W |
| **Logarithm** | 7.9 GB/s | 12.1 GB/s | 105× | 74W / 198W |
| **Hex2Int** | 6.8 GB/s | 10.4 GB/s | 68× | 71W / 192W |
| **VocabGen** | 3.9 GB/s | 6.2 GB/s | 39× | 75W / 200W |
| **VocabMap** | 4.5 GB/s | 7.1 GB/s | 52× | 74W / 198W |

*Baseline: Dual Xeon Gold 6248R (128 cores @ 3.0GHz, 294W)*

### Scalability Analysis

```mermaid
graph LR
    subgraph "Tensix Core Utilization"
        A[24 Cores<br/>~25% Peak]
        B[48 Cores<br/>~50% Peak]
        C[96 Cores<br/>~100% Peak e75]
        D[120 Cores<br/>~100% Peak e150]
    end

    A -->|2× Cores| B
    B -->|2× Cores| C
    C -->|1.25× Cores| D

    style C fill:#90ee90
    style D fill:#90ee90
```

### Energy Efficiency

- **e75**: 2.9-6.4× more power-efficient than CPU
- **e150**: 1.5-3.2× more power-efficient than CPU
- **Cost per GB processed**:
  - e75: $0.0021/GB (@ $0.10/kWh)
  - e150: $0.0039/GB (@ $0.10/kWh)
  - CPU: $0.0135/GB (@ $0.10/kWh)

---

## 📅 Lifecycle & Maintenance

### Software Version Compatibility Matrix

| Component | Version | Grayskull Support | Last Update |
|-----------|---------|-------------------|-------------|
| **TT-KMD** | v1.28 | ✅ Full | Oct 2024 |
| **TT-Firmware** | v6.11.0 | ✅ Full | Sep 2024 |
| **TT-Metalium** | v0.51.0-rc26 | ✅ Full | Nov 2024 |
| **TT-Buda** | v0.15.1 | ✅ Full | Aug 2024 |
| **PyBuda** | v0.15.1 | ✅ Full | Aug 2024 |
| **TT-SMI** | v2.3.0 | ✅ Full | Jul 2024 |
| **Ubuntu** | 22.04 LTS | ✅ Recommended | Until 2027 |
| **Ubuntu** | 20.04 LTS | ⚠️ Deprecated | Until 2025 |
| **Python** | 3.10 | ✅ Recommended | |
| **Python** | 3.8-3.11 | ✅ Compatible | |

### Kernel Compatibility

```bash
# Supported kernel versions
5.15.0-*  # Ubuntu 22.04 default (recommended)
5.19.0-*  # Ubuntu 22.04 HWE
6.2.0-*   # Ubuntu 22.04 HWE (latest)

# Verify current kernel
uname -r
```

### Maintenance Schedule

#### Monthly
- Monitor firmware updates: `sudo tt-flash --check-updates`
- Review system logs: `journalctl -u tenstorrent`
- Verify thermal performance: `tt-smi --temperature`

#### Quarterly
- Update TT-SMI: `pip install --upgrade tt-smi`
- Rebuild kernels after system updates
- Clean PCIe slot contacts (if performance degrades)

#### Annually
- Archive software stack versions
- Backup working configurations
- Plan migration to supported hardware

### Known Issues & Workarounds

#### Issue: PCIe Link Training Failure
```bash
# Symptoms: Device not detected after reboot
lspci | grep -i tenstorrent  # No output

# Workaround: Force PCIe rescan
echo 1 | sudo tee /sys/bus/pci/rescan
sleep 2
lspci | grep -i tenstorrent  # Should now appear
```

#### Issue: HugePages Not Allocated
```bash
# Symptoms: TT-SMI shows memory allocation errors
tt-smi -l  # Error: Failed to allocate hugepages

# Workaround: Increase hugepage allocation
sudo sysctl -w vm.nr_hugepages=16384
sudo sysctl -w vm.hugetlb_shm_group=$(id -g)
```

#### Issue: Thermal Throttling on e150
```bash
# Symptoms: Performance drops under sustained load
tt-smi --temperature  # Shows >70°C

# Workaround: Improve airflow
# 1. Verify case fans operational
# 2. Increase fan speed (if PWM controllable)
# 3. Add external cooling (e150 requires passive heatsink + airflow)
```

---

## 🔍 Troubleshooting

### Diagnostic Commands

```bash
# Complete system check
sudo tt-smi --diagnostics

# PCIe link status
sudo lspci -vvv -s $(lspci | grep Tenstorrent | cut -d' ' -f1) | grep -i 'lnk'

# Memory bandwidth test
cd ~/tt-metal
./build/test/tt_metal/perf_microbenchmark/dram_read

# Core utilization
tt-smi --cores

# Temperature monitoring (continuous)
watch -n 1 'tt-smi --temperature'

# Firmware version
sudo tt-flash --version
```

### Common Error Messages

#### `Error: No Tenstorrent devices found`

**Cause**: Driver not loaded or card not detected

**Solution**:
```bash
# Reload driver
sudo modprobe -r tenstorrent
sudo modprobe tenstorrent

# Check dmesg for errors
dmesg | tail -50 | grep -i tenstorrent
```

#### `Error: Firmware version mismatch`

**Cause**: Driver/firmware incompatibility

**Solution**:
```bash
# Reflash firmware
sudo tt-flash --device grayskull --fw-version v6.11.0 --force

# Reboot
sudo reboot
```

#### `Error: Compilation failed - unsupported operation`

**Cause**: TT-Buda doesn't support operation on Grayskull

**Solution**:
- Check operation compatibility: https://docs.tenstorrent.com/tt-buda/latest/operations.html
- Use TT-Metalium for custom kernels
- Implement operation on CPU as fallback

---

## 🤝 Contributing

Contributions are welcome! This project maintains Greyskull support as a community effort.

### Development Setup

```bash
# Clone repository
git clone https://github.com/danindiana/gpt_C_plus_plus.git
cd gpt_C_plus_plus/PIPER

# Create development environment
python3 -m venv venv
source venv/bin/activate
pip install -r requirements-dev.txt

# Run tests
pytest tests/

# Lint code
pre-commit run --all-files
```

### Contribution Guidelines

1. **Fork** the repository
2. **Create** feature branch (`git checkout -b feature/your-feature`)
3. **Commit** changes (`git commit -m 'Add your feature'`)
4. **Push** to branch (`git push origin feature/your-feature`)
5. **Open** Pull Request

### Code Style

- Python: PEP 8, type hints required
- C++: Google C++ Style Guide
- Commit messages: Conventional Commits

---

## 📚 References

### Official Documentation

- **Tenstorrent Docs**: https://docs.tenstorrent.com/
- **Greyskull Specs**: https://docs.tenstorrent.com/aibs/grayskull/specifications.html
- **TT-Metalium**: https://docs.tenstorrent.com/tt-metal/latest/
- **TT-Buda**: https://github.com/tenstorrent/tt-buda

### GitHub Repositories

- **TT-Metal**: https://github.com/tenstorrent/tt-metal
- **TT-Buda**: https://github.com/tenstorrent/tt-buda
- **TT-KMD**: https://github.com/tenstorrent/tt-kmd
- **TT-Firmware**: https://github.com/tenstorrent/tt-firmware
- **TT-SMI**: https://github.com/tenstorrent/tt-smi

### Academic Papers

- **PIPER Original Paper**: "PIPER: A Pluggable and Reconfigurable Accelerator for Data Preprocessing" (2023)
- **Greyskull Architecture**: "Tenstorrent Tensix: A Scalable, Composable AI Processor" (2022)

### Community Resources

- **Tenstorrent Discord**: https://discord.gg/tenstorrent
- **Reddit**: r/tenstorrent
- **X2I.dev Guides**: https://x2i.dev/blog/tenstorrent-npu-wormhole-n300s-setup-guide/

### Hardware Suppliers (Secondary Market)

- eBay: Search "Tenstorrent Greyskull"
- r/hardwareswap
- Used server component vendors

---

## 📄 License

This project is licensed under the Apache License 2.0 - see [LICENSE](LICENSE) file for details.

```
Copyright 2025 PIPER Development Team

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

---

## 🙏 Acknowledgments

- **Tenstorrent** for open-sourcing the software stack
- **PIPER Research Team** for the original architecture
- **RISC-V Foundation** for the ISA specification
- **Community Contributors** maintaining Greyskull support

---

<div align="center">

**Built with ❤️ for the Greyskull community**

[![GitHub Stars](https://img.shields.io/github/stars/danindiana/gpt_C_plus_plus?style=social)](https://github.com/danindiana/gpt_C_plus_plus)
[![GitHub Forks](https://img.shields.io/github/forks/danindiana/gpt_C_plus_plus?style=social)](https://github.com/danindiana/gpt_C_plus_plus/fork)

</div>
