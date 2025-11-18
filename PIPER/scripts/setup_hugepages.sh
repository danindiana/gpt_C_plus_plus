#!/bin/bash
#
# HugePages Configuration Script for Tenstorrent Greyskull
#
# This script configures Linux HugePages for optimal Tenstorrent performance.
# HugePages reduce TLB misses and improve memory access performance.
#
# Usage:
#   sudo ./setup_hugepages.sh [SIZE_GB]
#
# Default: 16GB

set -euo pipefail

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Default size: 16GB
DEFAULT_SIZE_GB=16
SIZE_GB=${1:-$DEFAULT_SIZE_GB}

# Calculate number of 2MB pages needed
PAGES_NEEDED=$((SIZE_GB * 1024 / 2))

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Check if running as root
if [[ $EUID -ne 0 ]]; then
    print_error "This script must be run with sudo"
    exit 1
fi

echo "========================================"
echo "HugePages Configuration"
echo "========================================"
echo "Requested size: ${SIZE_GB}GB"
echo "Pages needed: ${PAGES_NEEDED} x 2MB"
echo

# Check current memory
TOTAL_MEM_KB=$(grep MemTotal /proc/meminfo | awk '{print $2}')
TOTAL_MEM_GB=$((TOTAL_MEM_KB / 1024 / 1024))
REQUESTED_MB=$((SIZE_GB * 1024))

echo "System memory: ${TOTAL_MEM_GB}GB"

if [[ $SIZE_GB -gt $((TOTAL_MEM_GB / 2)) ]]; then
    print_warning "Requesting more than 50% of system memory"
    print_warning "This may cause system instability"
    read -p "Continue? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Show current hugepages
echo
echo "Current HugePages configuration:"
grep -E "HugePages|Hugepagesize" /proc/meminfo

# Allocate hugepages
echo
echo "Allocating ${PAGES_NEEDED} hugepages..."

# Try to allocate
echo $PAGES_NEEDED > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# Verify allocation
ALLOCATED=$(cat /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages)

if [[ $ALLOCATED -eq $PAGES_NEEDED ]]; then
    print_success "Successfully allocated ${PAGES_NEEDED} hugepages"
elif [[ $ALLOCATED -gt 0 ]]; then
    print_warning "Partially allocated: ${ALLOCATED}/${PAGES_NEEDED} pages"
    print_warning "System may not have enough free memory"
    print_warning "Try:"
    print_warning "  1. Closing applications"
    print_warning "  2. Reducing allocation size"
    print_warning "  3. Rebooting and running this script early"
else
    print_error "Failed to allocate any hugepages"
    exit 1
fi

# Make permanent
echo
echo "Making configuration permanent..."

SYSCTL_CONF="/etc/sysctl.conf"
BACKUP="${SYSCTL_CONF}.backup.$(date +%Y%m%d_%H%M%S)"

# Backup sysctl.conf
cp "$SYSCTL_CONF" "$BACKUP"
print_success "Backed up $SYSCTL_CONF to $BACKUP"

# Remove old hugepages settings
sed -i '/vm.nr_hugepages/d' "$SYSCTL_CONF"
sed -i '/vm.hugetlb_shm_group/d' "$SYSCTL_CONF"

# Add new settings
cat >> "$SYSCTL_CONF" <<EOF

# Tenstorrent HugePages Configuration
# Configured by setup_hugepages.sh on $(date)
vm.nr_hugepages = $PAGES_NEEDED
vm.hugetlb_shm_group = 0
EOF

# Apply immediately
sysctl -p > /dev/null 2>&1 || true

print_success "Added to $SYSCTL_CONF"

# Configure systemd service for early allocation
SYSTEMD_SERVICE="/etc/systemd/system/hugepages.service"

if [[ ! -f "$SYSTEMD_SERVICE" ]]; then
    cat > "$SYSTEMD_SERVICE" <<EOF
[Unit]
Description=Allocate HugePages for Tenstorrent
DefaultDependencies=no
Before=sysinit.target

[Service]
Type=oneshot
ExecStart=/bin/sh -c 'echo $PAGES_NEEDED > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages'

[Install]
WantedBy=sysinit.target
EOF

    systemctl daemon-reload
    systemctl enable hugepages.service
    print_success "Created systemd service for early allocation"
fi

# Final status
echo
echo "========================================"
echo "Final HugePages Status"
echo "========================================"
grep -E "HugePages|Hugepagesize" /proc/meminfo

HUGE_TOTAL=$(grep HugePages_Total /proc/meminfo | awk '{print $2}')
HUGE_FREE=$(grep HugePages_Free /proc/meminfo | awk '{print $2}')
HUGE_SIZE_MB=$((HUGE_TOTAL * 2))

echo
print_success "Configuration complete!"
echo
echo "Total allocated: ${HUGE_SIZE_MB}MB (${HUGE_TOTAL} pages)"
echo "Free: ${HUGE_FREE} pages"
echo
echo "Notes:"
echo "  - Changes are persistent across reboots"
echo "  - To modify allocation, run this script again"
echo "  - To disable, set vm.nr_hugepages = 0 in /etc/sysctl.conf"
echo

exit 0
