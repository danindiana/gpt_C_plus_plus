#!/bin/bash
#
# Tenstorrent Greyskull Installation Verification Script
#
# Performs comprehensive checks of the Tenstorrent software stack
# and PIPER installation.
#
# Usage:
#   ./verify_installation.sh

set -euo pipefail

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

# Counters
CHECKS_PASSED=0
CHECKS_FAILED=0
CHECKS_WARNING=0

print_header() {
    echo -e "${BLUE}======================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}======================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
    ((CHECKS_PASSED++))
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
    ((CHECKS_WARNING++))
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
    ((CHECKS_FAILED++))
}

check_system() {
    print_header "System Checks"

    # OS version
    if [[ -f /etc/os-release ]]; then
        source /etc/os-release
        if [[ "$ID" == "ubuntu" ]] && [[ "$VERSION_ID" == "22.04" ]]; then
            print_success "Ubuntu 22.04 LTS"
        else
            print_warning "OS is $ID $VERSION_ID (expected Ubuntu 22.04)"
        fi
    else
        print_error "Cannot determine OS version"
    fi

    # Kernel version
    KERNEL=$(uname -r)
    print_success "Kernel: $KERNEL"

    # Memory
    TOTAL_MEM_GB=$(free -g | awk '/^Mem:/{print $2}')
    if [[ $TOTAL_MEM_GB -ge 64 ]]; then
        print_success "System memory: ${TOTAL_MEM_GB}GB"
    elif [[ $TOTAL_MEM_GB -ge 32 ]]; then
        print_warning "System memory: ${TOTAL_MEM_GB}GB (64GB+ recommended)"
    else
        print_error "System memory: ${TOTAL_MEM_GB}GB (insufficient, 64GB minimum)"
    fi

    echo
}

check_pcie() {
    print_header "PCIe Checks"

    # Check for Tenstorrent device
    if lspci | grep -qi tenstorrent; then
        DEVICE=$(lspci | grep -i tenstorrent)
        print_success "Device found: $DEVICE"

        # Get PCIe address
        PCIE_ADDR=$(lspci | grep -i tenstorrent | cut -d' ' -f1)

        # Check link speed
        LINK_STATUS=$(sudo lspci -vvv -s "$PCIE_ADDR" | grep "LnkSta:")
        if echo "$LINK_STATUS" | grep -q "Speed 16GT/s"; then
            print_success "PCIe link: Gen 4 (16GT/s)"
        elif echo "$LINK_STATUS" | grep -q "Speed 8GT/s"; then
            print_warning "PCIe link: Gen 3 (8GT/s) - expected Gen 4"
        else
            print_warning "PCIe link speed: unknown"
        fi

        # Check link width
        if echo "$LINK_STATUS" | grep -q "Width x16"; then
            print_success "PCIe width: x16"
        else
            WIDTH=$(echo "$LINK_STATUS" | grep -oP "Width x\K\d+")
            print_warning "PCIe width: x${WIDTH} (expected x16)"
        fi
    else
        print_error "No Tenstorrent device detected"
        echo "  Run: lspci | grep -i tenstorrent"
    fi

    echo
}

check_hugepages() {
    print_header "HugePages Checks"

    HUGE_TOTAL=$(grep HugePages_Total /proc/meminfo | awk '{print $2}')
    HUGE_FREE=$(grep HugePages_Free /proc/meminfo | awk '{print $2}')
    HUGE_SIZE=$(grep Hugepagesize /proc/meminfo | awk '{print $2}')

    if [[ $HUGE_TOTAL -gt 0 ]]; then
        HUGE_MB=$((HUGE_TOTAL * HUGE_SIZE / 1024))
        print_success "HugePages allocated: ${HUGE_MB}MB (${HUGE_TOTAL} pages)"
        print_success "HugePages free: ${HUGE_FREE} pages"

        if [[ $HUGE_FREE -lt $((HUGE_TOTAL / 2)) ]]; then
            print_warning "Less than 50% hugepages free"
        fi
    else
        print_error "No HugePages allocated"
        echo "  Run: sudo ./scripts/setup_hugepages.sh"
    fi

    # Check sysctl configuration
    if grep -q "vm.nr_hugepages" /etc/sysctl.conf; then
        print_success "HugePages persistent configuration found"
    else
        print_warning "HugePages not configured in /etc/sysctl.conf"
    fi

    echo
}

check_driver() {
    print_header "TT-KMD Driver Checks"

    # Check if module is loaded
    if lsmod | grep -q tenstorrent; then
        print_success "tenstorrent kernel module loaded"

        # Check version
        MODINFO=$(modinfo tenstorrent 2>/dev/null || true)
        if [[ -n "$MODINFO" ]]; then
            VERSION=$(echo "$MODINFO" | grep "^version:" | awk '{print $2}')
            if [[ -n "$VERSION" ]]; then
                print_success "Driver version: $VERSION"
            fi
        fi
    else
        print_error "tenstorrent kernel module not loaded"
        echo "  Try: sudo modprobe tenstorrent"
    fi

    # Check DKMS
    if dkms status | grep -q tenstorrent; then
        print_success "DKMS module installed"
    else
        print_warning "DKMS module not found"
    fi

    # Check dmesg for errors
    if dmesg | grep -i tenstorrent | grep -qi error; then
        print_warning "Errors found in dmesg (check: dmesg | grep -i tenstorrent)"
    else
        print_success "No driver errors in dmesg"
    fi

    echo
}

check_firmware() {
    print_header "Firmware Checks"

    # Check if tt-flash is installed
    if command -v tt-flash &> /dev/null; then
        print_success "tt-flash utility found"

        # Get firmware version
        FW_INFO=$(tt-flash --version 2>&1 || true)
        if [[ -n "$FW_INFO" ]]; then
            echo "  $FW_INFO"
        fi
    else
        print_error "tt-flash not found"
        echo "  Install: pip install tt-flash"
    fi

    echo
}

check_python() {
    print_header "Python Environment Checks"

    # Check Python version
    if command -v python3 &> /dev/null; then
        PY_VERSION=$(python3 --version | awk '{print $2}')
        if [[ "$PY_VERSION" =~ ^3\.([0-9]+) ]]; then
            MINOR=${BASH_REMATCH[1]}
            if [[ $MINOR -ge 8 ]]; then
                print_success "Python version: $PY_VERSION"
            else
                print_warning "Python version: $PY_VERSION (3.8+ recommended)"
            fi
        fi
    else
        print_error "python3 not found"
    fi

    # Check for virtual environment
    if [[ -d "$HOME/tt-env" ]]; then
        print_success "TT virtual environment exists"
    else
        print_warning "TT virtual environment not found at ~/tt-env"
    fi

    # Check TT-SMI (if venv activated)
    if [[ -n "${VIRTUAL_ENV:-}" ]]; then
        if command -v tt-smi &> /dev/null; then
            print_success "tt-smi found"

            # Try to run tt-smi
            if tt-smi -l &> /dev/null; then
                print_success "tt-smi can communicate with device"
            else
                print_warning "tt-smi cannot communicate with device"
            fi
        else
            print_warning "tt-smi not found (install: pip install tt-smi)"
        fi
    else
        print_warning "Virtual environment not activated"
        echo "  Run: source ~/tt-env/bin/activate"
    fi

    echo
}

check_tt_metal() {
    print_header "TT-Metalium Checks"

    # Check if TT_METAL_HOME is set
    if [[ -n "${TT_METAL_HOME:-}" ]]; then
        print_success "TT_METAL_HOME set: $TT_METAL_HOME"

        if [[ -d "$TT_METAL_HOME" ]]; then
            print_success "TT-Metal directory exists"

            # Check for build artifacts
            if [[ -d "$TT_METAL_HOME/build" ]]; then
                print_success "Build directory found"

                # Check for key binaries
                if [[ -f "$TT_METAL_HOME/build/lib/libtt_metal.so" ]]; then
                    print_success "libtt_metal.so built"
                else
                    print_warning "libtt_metal.so not found"
                fi
            else
                print_warning "Build directory not found (run: cmake --build build)"
            fi
        else
            print_error "TT_METAL_HOME directory does not exist"
        fi
    else
        print_warning "TT_METAL_HOME not set"
        echo "  Add to ~/.bashrc: export TT_METAL_HOME=\$HOME/tt-metal"
    fi

    echo
}

check_tt_buda() {
    print_header "TT-Buda Checks"

    # Check if activated venv has pybuda
    if [[ -n "${VIRTUAL_ENV:-}" ]]; then
        if python3 -c "import pybuda" 2>/dev/null; then
            PB_VERSION=$(python3 -c "import pybuda; print(pybuda.__version__)" 2>/dev/null || echo "unknown")
            print_success "PyBuda installed (version: $PB_VERSION)"
        else
            print_warning "PyBuda not importable"
        fi
    else
        print_warning "Virtual environment not activated (cannot check PyBuda)"
    fi

    echo
}

check_piper() {
    print_header "PIPER Checks"

    PIPER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

    # Check directory structure
    if [[ -d "$PIPER_DIR/examples" ]]; then
        print_success "examples/ directory found"
    fi

    if [[ -d "$PIPER_DIR/benchmark" ]]; then
        print_success "benchmark/ directory found"
    fi

    if [[ -d "$PIPER_DIR/scripts" ]]; then
        print_success "scripts/ directory found"
    fi

    # Check key files
    if [[ -f "$PIPER_DIR/readme.md" ]]; then
        print_success "README found"
    fi

    if [[ -f "$PIPER_DIR/requirements.txt" ]]; then
        print_success "requirements.txt found"
    fi

    # Check if Python examples are executable
    if [[ -x "$PIPER_DIR/examples/neg2zero_pipeline.py" ]]; then
        print_success "neg2zero_pipeline.py is executable"
    elif [[ -f "$PIPER_DIR/examples/neg2zero_pipeline.py" ]]; then
        print_warning "neg2zero_pipeline.py exists but not executable"
        echo "  Run: chmod +x $PIPER_DIR/examples/neg2zero_pipeline.py"
    fi

    echo
}

print_summary() {
    print_header "Verification Summary"

    echo "Checks passed:  $CHECKS_PASSED"
    echo "Warnings:       $CHECKS_WARNING"
    echo "Checks failed:  $CHECKS_FAILED"
    echo

    if [[ $CHECKS_FAILED -eq 0 ]] && [[ $CHECKS_WARNING -eq 0 ]]; then
        echo -e "${GREEN}✓ All checks passed! System is ready.${NC}"
        echo
        echo "Next steps:"
        echo "  1. Activate environment: source ~/tt-env/bin/activate"
        echo "  2. Run examples: cd PIPER/examples && python neg2zero_pipeline.py"
        return 0
    elif [[ $CHECKS_FAILED -eq 0 ]]; then
        echo -e "${YELLOW}⚠ Installation complete with warnings${NC}"
        echo
        echo "Review warnings above and address if needed."
        return 0
    else
        echo -e "${RED}✗ Installation incomplete or errors detected${NC}"
        echo
        echo "Address the failed checks above before proceeding."
        return 1
    fi
}

# Main
main() {
    echo
    print_header "Tenstorrent Greyskull Verification"
    echo

    check_system
    check_pcie
    check_hugepages
    check_driver
    check_firmware
    check_python
    check_tt_metal
    check_tt_buda
    check_piper

    print_summary
}

main
exit $?
