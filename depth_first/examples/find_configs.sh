#!/bin/bash
# Example: Find all configuration files in /etc

echo "Finding configuration files in /etc..."
../build/bin/dfs-search "*.conf" /etc 3

echo ""
echo "Finding .config directories in home..."
../build/bin/dfs-search ".config" "$HOME" 2
