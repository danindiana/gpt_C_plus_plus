# Migration Guide: Greyskull → Wormhole/Blackhole

## Overview

This guide helps migrate PIPER workloads from deprecated Greyskull (e75/e150) to current-generation Tenstorrent hardware.

## Target Hardware

### Wormhole (Recommended)

- **n150**: Entry-level, Greyskull successor
- **n300**: Dual-chip, higher performance
- **Compatibility**: Good backwards compatibility

### Blackhole (Latest)

- **Latest generation**: Maximum performance
- **Compatibility**: May require code changes

## Migration Checklist

### 1. Hardware Replacement

- [ ] Acquire Wormhole/Blackhole card
- [ ] Verify PCIe slot compatibility
- [ ] Update power supply if needed
- [ ] Install new card

### 2. Software Updates

```bash
# Update TT-KMD driver
cd /opt/tt-kmd
git pull
sudo make dkms_uninstall
sudo make dkms_install

# Update firmware
cd /opt/tt-firmware
git pull
sudo tt-flash --device wormhole --fw-version latest

# Update TT-Metal
cd ~/tt-metal
git pull
export ARCH_NAME=wormhole_b0  # Or blackhole
cmake -B build -G Ninja
cmake --build build

# Update TT-Buda
cd ~/tt-buda
git pull
pip install --upgrade -r requirements.txt
python setup.py install
```

### 3. Code Changes

#### Update Device Architecture

**Before (Greyskull)**:
```python
device = TFDevice(
    arch=BackendDevice.Grayskull,
    devtype=BackendType.Silicon
)
```

**After (Wormhole)**:
```python
device = TFDevice(
    arch=BackendDevice.Wormhole_B0,
    devtype=BackendType.Silicon
)
```

#### Update Environment Variables

```bash
# Before
export ARCH_NAME=grayskull

# After
export ARCH_NAME=wormhole_b0  # or blackhole
```

#### Update CMake (C++ kernels)

**Before**:
```cmake
set(ARCH_NAME grayskull)
```

**After**:
```cmake
set(ARCH_NAME wormhole_b0)
```

### 4. Compatibility Matrix

| Feature | Greyskull | Wormhole | Blackhole |
|---------|-----------|----------|-----------|
| TT-Buda API | ✅ | ✅ | ✅ |
| TT-Metalium | ✅ | ✅ | ⚠️ Changes |
| PyBuda | ✅ | ✅ | ✅ |
| Operator Library | Core ops | Extended ops | Full ops |

### 5. Performance Expectations

#### Wormhole n150

- **Cores**: More than Greyskull e150
- **Frequency**: Higher clock
- **Expected Speedup**: 1.5-2× over e150

#### Wormhole n300

- **Dual-chip**: 2× cores
- **Expected Speedup**: 2.5-3.5× over e150

### 6. Testing

```bash
# Verify installation
./scripts/verify_installation.sh

# Run benchmarks
cd benchmark
python criteo_preprocessing.py --device silicon

# Compare results
# Greyskull e150: ~12 GB/s stateless
# Wormhole n150: ~18-24 GB/s expected
```

### 7. Known Differences

#### Memory Architecture

- **Greyskull**: LPDDR4
- **Wormhole**: GDDR6 (higher bandwidth)
- **Impact**: 1.5-2× memory throughput

#### Core Count

- **Greyskull e150**: 120 cores
- **Wormhole n150**: More cores (check specs)
- **Impact**: Better parallelism

#### Precision Support

- **Greyskull**: FP8, FP16, BFLOAT16
- **Wormhole**: Same + additional formats
- **Blackhole**: Extended precision support

## Code Compatibility

### Fully Compatible (No Changes)

✅ PyBuda high-level API
✅ Operator definitions
✅ Data formats
✅ Model structures

### May Need Updates

⚠️ TT-Metalium kernels (architecture-specific)
⚠️ Performance tuning parameters
⚠️ SRAM utilization strategies

### Breaking Changes

❌ Direct hardware register access
❌ Architecture-specific optimizations
❌ Hardcoded core counts

## Example Migration

### Before: Greyskull-specific

```python
# examples/neg2zero_greyskull.py
device = TFDevice(arch=BackendDevice.Grayskull, ...)
expected_throughput_gbs = 12.5  # e150

if throughput < expected_throughput_gbs * 0.9:
    print("Performance below e150 spec")
```

### After: Hardware-agnostic

```python
# examples/neg2zero_unified.py
import os

# Detect architecture from environment
arch_name = os.environ.get("ARCH_NAME", "grayskull")
arch_map = {
    "grayskull": (BackendDevice.Grayskull, 12.5),
    "wormhole_b0": (BackendDevice.Wormhole_B0, 20.0),
    "blackhole": (BackendDevice.Blackhole, 30.0)
}

arch_device, expected_throughput = arch_map[arch_name]
device = TFDevice(arch=arch_device, ...)

if throughput < expected_throughput * 0.9:
    print(f"Performance below {arch_name} spec")
```

## Rollback Plan

If issues arise, reverting to Greyskull:

```bash
# Reinstall Greyskull card
# Revert software
export ARCH_NAME=grayskull
cd ~/tt-metal && git checkout <greyskull-compatible-tag>
cd ~/tt-buda && git checkout <greyskull-compatible-tag>
```

## Support Resources

- **Wormhole Docs**: https://docs.tenstorrent.com/hardware/wormhole
- **Migration Forum**: https://github.com/tenstorrent/tt-metal/discussions
- **Discord**: https://discord.gg/tenstorrent

## See Also

- [Architecture Documentation](architecture.md)
- [Performance Tuning](performance.md)
- [Main README](../readme.md)
