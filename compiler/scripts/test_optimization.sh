#!/usr/bin/env bash

#
# Compiler Optimization Testing Script
# Tests for optimization inconsistencies when additional information is provided
#
# Usage: ./test_optimization.sh [options]
#

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
OUTPUT_DIR="${OUTPUT_DIR:-${PROJECT_ROOT}/output}"
TEMP_DIR=$(mktemp -d)

# Compiler paths
GCC="${GCC:-/usr/bin/gcc}"
CLANG="${CLANG:-/usr/bin/clang}"

# Test configuration
OPTIMIZATION_LEVEL="${OPTIMIZATION_LEVEL:-O3}"
BINARY_SIZE_THRESHOLD="${BINARY_SIZE_THRESHOLD:-0.05}" # 5%
VERBOSE="${VERBOSE:-0}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
total_tests=0
issues_found=0

# Cleanup on exit
cleanup() {
    rm -rf "${TEMP_DIR}"
}
trap cleanup EXIT INT TERM

# Logging functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $*"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $*"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $*"
}

log_verbose() {
    if [[ "${VERBOSE}" -eq 1 ]]; then
        echo -e "[VERBOSE] $*"
    fi
}

# Check if required tools are available
check_prerequisites() {
    log_info "Checking prerequisites..."

    if [[ ! -x "${GCC}" ]]; then
        log_error "GCC not found at ${GCC}"
        return 1
    fi

    if [[ ! -x "${CLANG}" ]]; then
        log_warn "Clang not found at ${CLANG} (optional)"
    fi

    log_verbose "GCC version: $(${GCC} --version | head -n1)"
    log_verbose "Clang version: $(${CLANG} --version | head -n1 2>/dev/null || echo 'N/A')"

    return 0
}

# Compile a program and return the output binary path
compile_program() {
    local compiler="$1"
    local source_file="$2"
    local output_file="$3"
    local opt_level="${4:-${OPTIMIZATION_LEVEL}}"

    log_verbose "Compiling ${source_file} with ${compiler} -${opt_level}"

    if "${compiler}" "-${opt_level}" -o "${output_file}" "${source_file}" 2>"${TEMP_DIR}/compile_errors.txt"; then
        echo "${output_file}"
        return 0
    else
        log_error "Compilation failed for ${source_file}"
        if [[ "${VERBOSE}" -eq 1 ]]; then
            cat "${TEMP_DIR}/compile_errors.txt"
        fi
        return 1
    fi
}

# Get binary size
get_binary_size() {
    local binary="$1"

    if [[ ! -f "${binary}" ]]; then
        echo "0"
        return 1
    fi

    stat -f%z "${binary}" 2>/dev/null || stat -c%s "${binary}" 2>/dev/null || echo "0"
}

# Inject optimization hints into a program
inject_optimization_hints() {
    local source_file="$1"
    local output_file="$2"

    log_verbose "Injecting optimization hints into ${source_file}"

    # Create a refined version with additional information
    # This is a simple example - real implementation would analyze the program
    cp "${source_file}" "${output_file}"

    # Example: Add a simple hint (this would be more sophisticated in practice)
    # For demonstration, we'll just add a comment
    echo "// Refined version with optimization hints" > "${output_file}.tmp"
    cat "${source_file}" >> "${output_file}.tmp"
    mv "${output_file}.tmp" "${output_file}"

    return 0
}

# Test a single program with a compiler
test_program_with_compiler() {
    local compiler="$1"
    local compiler_name="$2"
    local source_file="$3"

    log_verbose "Testing ${source_file} with ${compiler_name}"

    ((total_tests++)) || true

    # Create refined version
    local refined_file="${TEMP_DIR}/$(basename "${source_file%.c}_refined.c")"
    inject_optimization_hints "${source_file}" "${refined_file}"

    # Compile original
    local original_binary="${TEMP_DIR}/$(basename "${source_file%.c}_original.out")"
    if ! compile_program "${compiler}" "${source_file}" "${original_binary}"; then
        log_warn "Failed to compile original program"
        return 1
    fi

    # Compile refined
    local refined_binary="${TEMP_DIR}/$(basename "${source_file%.c}_refined.out")"
    if ! compile_program "${compiler}" "${refined_file}" "${refined_binary}"; then
        log_warn "Failed to compile refined program"
        return 1
    fi

    # Compare binary sizes
    local original_size=$(get_binary_size "${original_binary}")
    local refined_size=$(get_binary_size "${refined_binary}")

    log_verbose "Original size: ${original_size} bytes"
    log_verbose "Refined size: ${refined_size} bytes"

    if [[ "${original_size}" -eq 0 ]]; then
        log_warn "Original binary has zero size"
        return 1
    fi

    # Calculate size increase
    local size_diff=$((refined_size - original_size))
    local size_increase=$(echo "scale=4; ${size_diff} / ${original_size}" | bc -l 2>/dev/null || echo "0")

    # Check if size increased beyond threshold
    local threshold_check=$(echo "${size_increase} > ${BINARY_SIZE_THRESHOLD}" | bc -l 2>/dev/null || echo "0")

    if [[ "${threshold_check}" -eq 1 ]]; then
        ((issues_found++)) || true
        log_error "ISSUE FOUND: ${compiler_name} produced larger binary for refined program!"
        log_error "  Original: ${original_size} bytes"
        log_error "  Refined:  ${refined_size} bytes"
        log_error "  Increase: $(echo "scale=2; ${size_increase} * 100" | bc)%"

        # Log the issue
        echo "[$(date)] ${compiler_name}: ${source_file} - ${size_increase}x increase" >> "${OUTPUT_DIR}/issues.log"

        return 2
    fi

    log_verbose "Test passed: size increase ${size_increase} within threshold"
    return 0
}

# Test all programs in a directory
test_programs() {
    local program_dir="$1"

    if [[ ! -d "${program_dir}" ]]; then
        log_error "Program directory ${program_dir} does not exist"
        return 1
    fi

    log_info "Testing programs in ${program_dir}"

    # Find all C source files
    local c_files=()
    while IFS= read -r -d '' file; do
        c_files+=("${file}")
    done < <(find "${program_dir}" -name "*.c" -print0)

    if [[ ${#c_files[@]} -eq 0 ]]; then
        log_warn "No C source files found in ${program_dir}"
        return 0
    fi

    log_info "Found ${#c_files[@]} program(s) to test"

    # Test each program
    for source_file in "${c_files[@]}"; do
        log_info "Testing: $(basename "${source_file}")"

        # Test with GCC
        if [[ -x "${GCC}" ]]; then
            test_program_with_compiler "${GCC}" "GCC" "${source_file}" || true
        fi

        # Test with Clang
        if [[ -x "${CLANG}" ]]; then
            test_program_with_compiler "${CLANG}" "Clang" "${source_file}" || true
        fi
    done

    return 0
}

# Print usage
print_usage() {
    cat << EOF
Usage: $0 [options] [program_directory]

Options:
    -h, --help              Show this help message
    -v, --verbose           Enable verbose output
    -o, --optimization LEVEL Set optimization level (O0, O1, O2, O3, Os, Ofast)
    -t, --threshold VALUE   Set binary size threshold (default: 0.05)
    --gcc PATH              Path to GCC compiler
    --clang PATH            Path to Clang compiler
    --output DIR            Output directory for results

Environment Variables:
    GCC                     Path to GCC compiler (default: /usr/bin/gcc)
    CLANG                   Path to Clang compiler (default: /usr/bin/clang)
    OPTIMIZATION_LEVEL      Optimization level (default: O3)
    BINARY_SIZE_THRESHOLD   Binary size threshold (default: 0.05)
    VERBOSE                 Enable verbose output (0 or 1)

Examples:
    $0 ./examples/seed_programs
    $0 -v -o O2 ./examples/seed_programs
    VERBOSE=1 $0 ./examples/seed_programs

EOF
}

# Main function
main() {
    local program_dir="${PROJECT_ROOT}/examples/seed_programs"

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -h|--help)
                print_usage
                exit 0
                ;;
            -v|--verbose)
                VERBOSE=1
                shift
                ;;
            -o|--optimization)
                OPTIMIZATION_LEVEL="$2"
                shift 2
                ;;
            -t|--threshold)
                BINARY_SIZE_THRESHOLD="$2"
                shift 2
                ;;
            --gcc)
                GCC="$2"
                shift 2
                ;;
            --clang)
                CLANG="$2"
                shift 2
                ;;
            --output)
                OUTPUT_DIR="$2"
                shift 2
                ;;
            -*)
                log_error "Unknown option: $1"
                print_usage
                exit 1
                ;;
            *)
                program_dir="$1"
                shift
                ;;
        esac
    done

    # Create output directory
    mkdir -p "${OUTPUT_DIR}"

    log_info "Compiler Optimization Testing Script"
    log_info "======================================"

    # Check prerequisites
    if ! check_prerequisites; then
        exit 1
    fi

    # Run tests
    test_programs "${program_dir}"

    # Print summary
    log_info ""
    log_info "======================================"
    log_info "Testing Complete"
    log_info "======================================"
    log_info "Total tests:   ${total_tests}"
    log_info "Issues found:  ${issues_found}"

    if [[ ${issues_found} -gt 0 ]]; then
        log_warn "Issues were found! See ${OUTPUT_DIR}/issues.log for details"
        exit 2
    else
        log_info "All tests passed!"
        exit 0
    fi
}

# Run main function
main "$@"
