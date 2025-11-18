# Usage Examples

This directory contains practical examples of using the DFS search tools.

## Scripts

### find_configs.sh
Finds configuration files in system directories.

```bash
./find_configs.sh
```

### find_sources.sh
Finds all C/C++ source files in a project.

```bash
# Search current directory
./find_sources.sh

# Search specific directory with custom depth
./find_sources.sh /path/to/project 15
```

### cleanup_backups.sh
Finds and optionally removes backup files.

```bash
# Dry run (default)
./cleanup_backups.sh /path/to/search

# Actually delete files
./cleanup_backups.sh /path/to/search no
```

## Direct Command Examples

### Find Documentation Files
```bash
../build/bin/dfs-search "*.md" . 5
../build/bin/dfs-search "*README*" . 10
```

### Find Log Files
```bash
../build/bin/dfs-search "*.log" /var/log 3
../build/bin/dfs-search "*error*.log" /var/log 5 -v
```

### Find with Regex
```bash
# C++ source or header files
../build/bin/dfs-search ".*\\.(cpp|hpp|h)" . 10 -r

# Files matching date pattern
../build/bin/dfs-search "[0-9]{4}-[0-9]{2}-[0-9]{2}\\.log" /var/log 3 -r
```

### Verbose Search
```bash
../build/bin/dfs-search "*.conf" /etc 5 -v
```

## Integration Examples

### With xargs
```bash
# Count lines in all C++ files
../build/bin/dfs-search "*.cpp" . 10 | grep "Found:" | cut -d' ' -f2 | xargs wc -l

# Remove all .tmp files
../build/bin/dfs-search "*.tmp" . 5 | grep "Found:" | cut -d' ' -f2 | xargs rm
```

### With while loop
```bash
# Process each found file
../build/bin/dfs-search "*.txt" . 5 | grep "Found:" | cut -d' ' -f2 | while read file; do
    echo "Processing: $file"
    # Do something with $file
done
```

### In Makefiles
```makefile
find-sources:
    dfs-search "*.cpp" ./src 10 > sources.txt

clean-backups:
    dfs-search "*.bak" . 10 | grep "Found:" | cut -d' ' -f2 | xargs rm -f
```

## Tips

1. **Quote your patterns**: Always use quotes around patterns to prevent shell expansion
   ```bash
   dfs-search "*.txt" .    # Good
   dfs-search *.txt .      # Bad
   ```

2. **Start specific**: Begin searches from the most specific directory
   ```bash
   dfs-search "*.cpp" ./src      # Good - specific
   dfs-search "*.cpp" /          # Bad - too broad
   ```

3. **Limit depth**: Use appropriate depth limits for performance
   ```bash
   dfs-search "*.txt" . 5    # Good for most projects
   dfs-search "*.txt" . 50   # Probably excessive
   ```

4. **Test first**: Test patterns on small directories before running on large ones
   ```bash
   # Test first
   dfs-search "pattern" ./small-test-dir 3

   # Then expand
   dfs-search "pattern" ./entire-project 10
   ```

5. **Use verbose for debugging**: Enable verbose mode to see what's happening
   ```bash
   dfs-search "*.conf" /etc 5 -v
   ```
