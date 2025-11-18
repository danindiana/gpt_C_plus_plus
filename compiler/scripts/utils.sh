#!/usr/bin/env bash

#
# Utility functions for compiler testing
#

# Detect the operating system
detect_os() {
    case "$(uname -s)" in
        Linux*)     echo "Linux";;
        Darwin*)    echo "macOS";;
        CYGWIN*)    echo "Windows";;
        MINGW*)     echo "Windows";;
        *)          echo "Unknown";;
    esac
}

# Check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Find available compilers
find_compilers() {
    local compilers=()

    if command_exists gcc; then
        compilers+=("gcc:$(which gcc)")
    fi

    if command_exists clang; then
        compilers+=("clang:$(which clang)")
    fi

    if command_exists g++; then
        compilers+=("g++:$(which g++)")
    fi

    if command_exists clang++; then
        compilers+=("clang++:$(which clang++)")
    fi

    printf '%s\n' "${compilers[@]}"
}

# Get compiler version
get_compiler_version() {
    local compiler="$1"

    if [[ ! -x "${compiler}" ]]; then
        echo "unknown"
        return 1
    fi

    "${compiler}" --version 2>/dev/null | head -n1 || echo "unknown"
}

# Compare two binaries
compare_binaries() {
    local file1="$1"
    local file2="$2"

    if [[ ! -f "${file1}" ]] || [[ ! -f "${file2}" ]]; then
        return 1
    fi

    cmp -s "${file1}" "${file2}"
}

# Calculate file size difference percentage
size_difference_percent() {
    local size1="$1"
    local size2="$2"

    if [[ "${size1}" -eq 0 ]]; then
        echo "0"
        return
    fi

    echo "scale=4; (${size2} - ${size1}) / ${size1}" | bc -l 2>/dev/null || echo "0"
}

# Extract function names from C source
extract_functions() {
    local source_file="$1"

    grep -E '^[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\(' "${source_file}" | \
        sed 's/^\([a-zA-Z_][a-zA-Z0-9_]*\).*/\1/' || true
}

# Check if Csmith is available
check_csmith() {
    if command_exists csmith; then
        return 0
    elif [[ -x "/usr/bin/csmith" ]]; then
        return 0
    elif [[ -x "/usr/local/bin/csmith" ]]; then
        return 0
    fi

    return 1
}

# Check if YarpGen is available
check_yarpgen() {
    if command_exists yarpgen; then
        return 0
    elif [[ -x "/usr/bin/yarpgen" ]]; then
        return 0
    elif [[ -x "/usr/local/bin/yarpgen" ]]; then
        return 0
    fi

    return 1
}

# Generate a random C program using Csmith
generate_with_csmith() {
    local output_file="$1"
    local seed="${2:-$(date +%s)}"

    if ! check_csmith; then
        return 1
    fi

    csmith --seed "${seed}" > "${output_file}" 2>/dev/null
}

# Parse compiler output for warnings
parse_warnings() {
    local compile_output="$1"

    grep -E '(warning|error):' <<< "${compile_output}" || true
}

# Create a minimized version of a test case (requires creduce)
minimize_test_case() {
    local source_file="$1"
    local test_script="$2"
    local output_file="$3"

    if ! command_exists creduce; then
        echo "creduce not found"
        return 1
    fi

    cp "${source_file}" "${output_file}"
    creduce "${test_script}" "${output_file}" 2>/dev/null || true
}

# Run git bisect (requires git repository)
git_bisect_helper() {
    local repo_path="$1"
    local test_script="$2"
    local good_commit="${3:-HEAD~100}"
    local bad_commit="${4:-HEAD}"

    if [[ ! -d "${repo_path}/.git" ]]; then
        echo "Not a git repository: ${repo_path}"
        return 1
    fi

    (
        cd "${repo_path}" || return 1
        git bisect start
        git bisect bad "${bad_commit}"
        git bisect good "${good_commit}"
        git bisect run "${test_script}"
        git bisect reset
    )
}

# Export all functions
export -f detect_os
export -f command_exists
export -f find_compilers
export -f get_compiler_version
export -f compare_binaries
export -f size_difference_percent
export -f extract_functions
export -f check_csmith
export -f check_yarpgen
export -f generate_with_csmith
export -f parse_warnings
export -f minimize_test_case
export -f git_bisect_helper
