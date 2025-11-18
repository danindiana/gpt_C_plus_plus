#!/bin/bash
#
# PIPER Installation Script for Tenstorrent Greyskull
#
# This script automates the complete installation of the Tenstorrent software
# stack and PIPER dependencies on Ubuntu 22.04 LTS.
#
# Usage:
#   sudo ./install.sh [--skip-driver] [--skip-firmware] [--skip-python]
#
# Requirements:
#   - Ubuntu 22.04 LTS
#   - Sudo privileges
#   - Internet connection
#   - Greyskull e75/e150 card installed

set -euo pipefail  # Exit on error, undefined vars, pipe failures

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default options
SKIP_DRIVER=false
SKIP_FIRMWARE=false
SKIP_PYTHON=false

# Installation paths
INSTALL_DIR="/opt"
TT_ENV_DIR="${HOME}/tt-env"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-driver)
            SKIP_DRIVER=true
            shift
            ;;
        --skip-firmware)
            SKIP_FIRMWARE=true
            shift
            ;;
        --skip-python)
            SKIP_PYTHON=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--skip-driver] [--skip-firmware] [--skip-python]"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Helper functions
print_header() {
    echo -e "${BLUE}======================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}======================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

check_requirements() {
    print_header "Checking Requirements"

    # Check OS
    if [[ ! -f /etc/os-release ]]; then
        print_error "Cannot determine OS version"
        exit 1
    fi

    source /etc/os-release
    if [[ "$ID" != "ubuntu" ]] || [[ "$VERSION_ID" != "22.04" ]]; then
        print_warning "This script is designed for Ubuntu 22.04 LTS"
        print_warning "Current OS: $ID $VERSION_ID"
        read -p "Continue anyway? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    else
        print_success "Ubuntu 22.04 LTS detected"
    fi

    # Check for sudo
    if [[ $EUID -ne 0 ]]; then
        print_error "This script must be run with sudo"
        exit 1
    fi

    # Check for internet
    if ! ping -c 1 8.8.8.8 &> /dev/null; then
        print_error "No internet connection detected"
        exit 1
    fi
    print_success "Internet connection OK"

    # Check for Greyskull card
    if lspci | grep -qi tenstorrent; then
        print_success "Tenstorrent device detected"
        lspci | grep -i tenstorrent
    else
        print_warning "No Tenstorrent device detected"
        print_warning "Make sure the card is properly installed"
        read -p "Continue anyway? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi

    echo
}

install_system_dependencies() {
    print_header "Installing System Dependencies"

    apt update
    apt install -y \
        build-essential \
        cmake \
        ninja-build \
        git \
        wget \
        curl \
        jq \
        python3.10 \
        python3-pip \
        python3-venv \
        python3-dev \
        linux-headers-$(uname -r) \
        dkms \
        libhugetlbfs-dev \
        pciutils \
        lshw

    print_success "System dependencies installed"
    echo
}

configure_hugepages() {
    print_header "Configuring HugePages"

    # Run the hugepages configuration script
    if [[ -f "$(dirname $0)/setup_hugepages.sh" ]]; then
        bash "$(dirname $0)/setup_hugepages.sh"
    else
        # Inline configuration
        echo "Allocating 16GB of HugePages..."
        echo 8192 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

        # Make permanent
        if ! grep -q "vm.nr_hugepages" /etc/sysctl.conf; then
            echo "vm.nr_hugepages = 8192" >> /etc/sysctl.conf
            sysctl -p
        fi

        print_success "HugePages configured (16GB)"
    fi

    grep HugePages /proc/meminfo
    echo
}

install_tt_kmd() {
    if $SKIP_DRIVER; then
        print_warning "Skipping TT-KMD driver installation"
        return
    fi

    print_header "Installing TT-KMD Driver"

    cd "$INSTALL_DIR"

    if [[ -d tt-kmd ]]; then
        print_warning "tt-kmd directory exists, updating..."
        cd tt-kmd
        git pull
    else
        git clone https://github.com/tenstorrent/tt-kmd.git
        cd tt-kmd
    fi

    # Build and install via DKMS
    make dkms_install

    # Load module
    modprobe tenstorrent || true

    # Verify
    if lsmod | grep -q tenstorrent; then
        print_success "TT-KMD driver installed and loaded"
    else
        print_error "TT-KMD driver not loaded"
        dmesg | tail -20
        exit 1
    fi

    echo
}

install_tt_firmware() {
    if $SKIP_FIRMWARE; then
        print_warning "Skipping firmware installation"
        return
    fi

    print_header "Installing TT-Firmware"

    cd "$INSTALL_DIR"

    if [[ -d tt-firmware ]]; then
        print_warning "tt-firmware directory exists, updating..."
        cd tt-firmware
        git pull
    else
        git clone https://github.com/tenstorrent/tt-firmware.git
        cd tt-firmware
    fi

    # Install tt-flash utility (as regular user)
    sudo -u $SUDO_USER pip install tt-flash

    # Flash firmware
    print_warning "Flashing firmware to Greyskull..."
    sudo -u $SUDO_USER tt-flash --device grayskull --fw-version latest || {
        print_error "Firmware flash failed"
        exit 1
    }

    print_success "Firmware installed"
    echo
}

install_python_environment() {
    if $SKIP_PYTHON; then
        print_warning "Skipping Python environment setup"
        return
    fi

    print_header "Setting up Python Environment"

    # Create venv as regular user
    sudo -u $SUDO_USER python3 -m venv "$TT_ENV_DIR"

    # Activate and install packages
    sudo -u $SUDO_USER bash -c "
        source $TT_ENV_DIR/bin/activate
        pip install --upgrade pip setuptools wheel

        # Install TT-SMI
        pip install git+https://github.com/tenstorrent/tt-smi.git

        # Verify
        tt-smi -l
    "

    print_success "Python environment created at $TT_ENV_DIR"
    echo
}

install_tt_metal() {
    print_header "Installing TT-Metalium SDK"

    cd "$HOME"

    if [[ -d tt-metal ]]; then
        print_warning "tt-metal directory exists, skipping clone"
    else
        sudo -u $SUDO_USER git clone https://github.com/tenstorrent/tt-metal.git
    fi

    cd tt-metal

    # Set environment variables
    export ARCH_NAME=grayskull
    export TT_METAL_HOME=$(pwd)

    # Build
    sudo -u $SUDO_USER bash -c "
        export ARCH_NAME=grayskull
        export TT_METAL_HOME=$TT_METAL_HOME
        cmake -B build -G Ninja
        cmake --build build
    "

    print_success "TT-Metalium SDK built"
    echo
}

install_tt_buda() {
    print_header "Installing TT-Buda Framework"

    cd "$HOME"

    if [[ -d tt-buda ]]; then
        print_warning "tt-buda directory exists, skipping clone"
    else
        sudo -u $SUDO_USER git clone https://github.com/tenstorrent/tt-buda.git
    fi

    cd tt-buda

    # Install in venv
    sudo -u $SUDO_USER bash -c "
        source $TT_ENV_DIR/bin/activate
        pip install -r requirements.txt
        python setup.py install
    "

    print_success "TT-Buda framework installed"
    echo
}

install_piper_dependencies() {
    print_header "Installing PIPER Dependencies"

    PIPER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

    if [[ -f "$PIPER_DIR/requirements.txt" ]]; then
        sudo -u $SUDO_USER bash -c "
            source $TT_ENV_DIR/bin/activate
            pip install -r $PIPER_DIR/requirements.txt
        "
        print_success "PIPER dependencies installed"
    else
        print_warning "requirements.txt not found"
    fi

    echo
}

setup_environment() {
    print_header "Setting up Environment"

    # Add environment variables to bashrc
    ENV_SETUP="
# Tenstorrent Environment
export TT_METAL_HOME=\$HOME/tt-metal
export ARCH_NAME=grayskull
export PYTHONPATH=\$TT_METAL_HOME:\$PYTHONPATH

# Activate TT environment
alias tt-env='source $TT_ENV_DIR/bin/activate'
"

    BASHRC="$HOME/.bashrc"
    if ! grep -q "Tenstorrent Environment" "$BASHRC"; then
        echo "$ENV_SETUP" >> "$BASHRC"
        print_success "Environment variables added to $BASHRC"
    else
        print_warning "Environment variables already in $BASHRC"
    fi

    echo
}

verify_installation() {
    print_header "Verifying Installation"

    # Run verification script if available
    if [[ -f "$(dirname $0)/verify_installation.sh" ]]; then
        bash "$(dirname $0)/verify_installation.sh"
    else
        print_warning "Verification script not found"
        print_warning "Manually verify with: tt-smi -l"
    fi

    echo
}

print_summary() {
    print_header "Installation Complete!"

    cat <<EOF

Next steps:

  1. Reboot your system:
     $ sudo reboot

  2. After reboot, activate the environment:
     $ source ~/tt-env/bin/activate

  3. Verify installation:
     $ tt-smi -l

  4. Run PIPER examples:
     $ cd PIPER/examples
     $ python neg2zero_pipeline.py --validate

  5. Check documentation:
     $ less PIPER/readme.md

Environment activation:
  - Manual: source ~/tt-env/bin/activate
  - Alias:  tt-env

For help: https://docs.tenstorrent.com/

EOF
}

# Main installation flow
main() {
    print_header "PIPER Installation for Greyskull"
    echo

    check_requirements
    install_system_dependencies
    configure_hugepages
    install_tt_kmd
    install_tt_firmware
    install_python_environment
    install_tt_metal
    install_tt_buda
    install_piper_dependencies
    setup_environment
    verify_installation
    print_summary
}

# Run main
main
