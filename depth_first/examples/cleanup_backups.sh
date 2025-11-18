#!/bin/bash
# Example: Find and optionally remove backup files

DIR="${1:-.}"
DRY_RUN="${2:-yes}"

echo "Searching for backup files in: $DIR"
echo ""

# Find .bak files
echo "=== Backup files (.bak) ==="
BACKUP_FILES=$(../build/bin/dfs-search "*.bak" "$DIR" 5)

if [ -z "$BACKUP_FILES" ]; then
    echo "No backup files found."
    exit 0
fi

echo "$BACKUP_FILES"
echo ""

# Count files
NUM_FILES=$(echo "$BACKUP_FILES" | grep -c "Found:")
echo "Total backup files found: $NUM_FILES"

if [ "$DRY_RUN" = "no" ]; then
    read -p "Do you want to delete these files? (yes/no): " CONFIRM
    if [ "$CONFIRM" = "yes" ]; then
        echo "$BACKUP_FILES" | grep "Found:" | cut -d' ' -f2 | xargs rm -v
        echo "Backup files deleted."
    else
        echo "Operation cancelled."
    fi
else
    echo ""
    echo "This is a dry run. To actually delete files, run:"
    echo "  $0 $DIR no"
fi
