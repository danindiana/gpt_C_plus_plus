# Detailed Usage Guide

## Table of Contents

- [Basic Usage](#basic-usage)
- [Pattern Matching](#pattern-matching)
- [Advanced Options](#advanced-options)
- [Real-World Examples](#real-world-examples)
- [Performance Tuning](#performance-tuning)
- [Troubleshooting](#troubleshooting)

## Basic Usage

### Command Syntax

```bash
dfs-search <pattern> <directory> [max_depth] [options]
```

### Parameters

- **pattern**: File pattern to search for (required)
- **directory**: Starting directory (required)
- **max_depth**: Maximum recursion depth (optional, default: 10)
- **options**: Additional flags (optional)

### Quick Examples

```bash
# Search current directory
./build/bin/dfs-search "*.txt" .

# Search with depth limit
./build/bin/dfs-search "*.cpp" /home/user 5

# Verbose search
./build/bin/dfs-search "*.log" /var/log 3 -v
```

## Pattern Matching

### Wildcard Patterns

#### Extension Matching
```bash
# All text files
dfs-search "*.txt" .

# All C++ files
dfs-search "*.cpp" ./src

# All headers
dfs-search "*.h" ./include
```

#### Prefix Matching
```bash
# Files starting with 'test'
dfs-search "test*" .

# Files starting with 'config'
dfs-search "config*" /etc
```

#### Contains Matching
```bash
# Files containing 'temp'
dfs-search "*temp*" /tmp

# Files containing 'backup'
dfs-search "*backup*" /home
```

### Regex Patterns

Enable regex with `-r` flag:

```bash
# C++ source or header files
dfs-search ".*\\.(cpp|hpp|h)" ./src 10 -r

# Files matching specific pattern
dfs-search "test_[0-9]+\\.txt" ./tests 5 -r

# Complex patterns
dfs-search "^[A-Z].*\\.log$" /var/log 3 -r
```

## Advanced Options

### Verbose Mode (`-v`)

Shows detailed search progress:

```bash
dfs-search "*.conf" /etc 3 -v
```

Output includes:
- Directories being scanned
- Paths being skipped
- Permission errors
- Detailed statistics

### Combining Options

```bash
# Regex + Verbose
dfs-search "test.*\\.cpp" ./src 10 -r -v

# All options
dfs-search ".*\\.txt" /home/user 5 -r -v
```

## Real-World Examples

### Example 1: Finding Configuration Files

```bash
# System configurations
dfs-search "*.conf" /etc 5

# User configurations
dfs-search ".config" /home/user 3

# Application configs
dfs-search "*config*.json" . 10
```

### Example 2: Source Code Search

```bash
# Find all C++ source files
dfs-search "*.cpp" ./src 10

# Find specific test files
dfs-search "test_*.cpp" ./tests 5

# Find headers in project
dfs-search "*.hpp" ./include 3
```

### Example 3: Log File Analysis

```bash
# Recent logs
dfs-search "*.log" /var/log 2

# Error logs
dfs-search "*error*.log" /var/log 5

# Application logs with regex
dfs-search "app_[0-9]{4}-[0-9]{2}-[0-9]{2}\\.log" /var/log 3 -r
```

### Example 4: Documentation Search

```bash
# README files
dfs-search "*README*" . 5

# Markdown documentation
dfs-search "*.md" ./docs 10

# All documentation
dfs-search ".*\\.(md|txt|rst)" . 8 -r
```

### Example 5: Cleanup Operations

```bash
# Find temporary files
dfs-search "*.tmp" /tmp 3

# Find backup files
dfs-search "*.bak" /home/user 10

# Find cache files
dfs-search "*cache*" . 5
```

## Performance Tuning

### Limiting Search Depth

Shallower searches are faster:

```bash
# Fast: depth 3
dfs-search "*.txt" /home/user 3

# Medium: depth 10
dfs-search "*.txt" /home/user 10

# Slow: depth 20
dfs-search "*.txt" /home/user 20
```

### Specific Starting Points

Start from the most specific directory:

```bash
# Better: specific directory
dfs-search "*.cpp" ./src/core 5

# Worse: broad directory
dfs-search "*.cpp" / 20
```

### Using Wildcards vs Regex

Wildcards are faster than regex:

```bash
# Fast: wildcard
dfs-search "*.txt" . 10

# Slower: regex
dfs-search ".*\\.txt" . 10 -r
```

## Troubleshooting

### Permission Denied Errors

If you encounter permission errors:

```bash
# Use sudo (be careful!)
sudo dfs-search "*.conf" /etc 5

# Add verbose to see which directories fail
dfs-search "*.conf" /etc 5 -v
```

### No Results Found

Check these common issues:

1. **Wrong pattern**: Verify pattern syntax
   ```bash
   # Wrong: missing quotes
   dfs-search *.txt .

   # Right: quoted pattern
   dfs-search "*.txt" .
   ```

2. **Insufficient depth**: Increase max_depth
   ```bash
   # May not find files in deep directories
   dfs-search "*.txt" /home/user 2

   # Better
   dfs-search "*.txt" /home/user 10
   ```

3. **Wrong directory**: Verify starting path
   ```bash
   # Check directory exists
   ls /path/to/search

   # Then search
   dfs-search "*.txt" /path/to/search 5
   ```

### Slow Performance

If searches are too slow:

1. **Reduce depth**:
   ```bash
   dfs-search "*.txt" /home/user 5  # Instead of 20
   ```

2. **Be more specific**:
   ```bash
   dfs-search "*.cpp" ./src/core  # Instead of ./
   ```

3. **Use wildcards instead of regex**:
   ```bash
   dfs-search "*.txt"  # Instead of ".*\\.txt" -r
   ```

### Regex Not Working

Ensure proper escaping:

```bash
# Wrong: unescaped dots
dfs-search "test.txt" . -r

# Right: escaped dots
dfs-search "test\\.txt" . -r

# Right: double-escaped in shell
dfs-search "test\\\\.txt" . -r
```

## Integration Examples

### Using in Scripts

```bash
#!/bin/bash

# Find and process all log files
dfs-search "*.log" /var/log 5 | while read -r file; do
    echo "Processing: $file"
    gzip "$file"
done
```

### Pipeline Usage

```bash
# Count found files
dfs-search "*.txt" . 5 | wc -l

# Filter results
dfs-search "*.cpp" ./src 10 | grep "test"

# Process with xargs
dfs-search "*.bak" . 10 | xargs rm
```

### Makefile Integration

```makefile
clean-logs:
    dfs-search "*.log" ./build 5 | xargs rm -f

find-sources:
    dfs-search "*.cpp" ./src 10 > sources.txt
```

## Best Practices

1. **Always quote patterns**:
   ```bash
   dfs-search "*.txt" .  # Good
   dfs-search *.txt .    # Bad: shell expansion
   ```

2. **Start from specific directories**:
   ```bash
   dfs-search "*.cpp" ./src      # Good
   dfs-search "*.cpp" /          # Bad: too broad
   ```

3. **Use appropriate depth limits**:
   ```bash
   dfs-search "*.txt" . 5   # Good for projects
   dfs-search "*.txt" . 50  # Usually excessive
   ```

4. **Enable verbose mode when debugging**:
   ```bash
   dfs-search "*.conf" /etc 5 -v
   ```

5. **Test patterns on small directories first**:
   ```bash
   # Test first
   dfs-search "pattern" ./small-dir 3

   # Then expand
   dfs-search "pattern" ./large-dir 10
   ```

## Additional Resources

- [Main README](../README.md) - Project overview
- [Architecture](ARCHITECTURE.md) - Technical details
- [Examples](../examples/) - More example scripts
- [GitHub Issues](https://github.com/danindiana/gpt_C_plus_plus/issues) - Report problems
