#!/bin/bash
# dfs-fzf: Interactive file search with fzf integration

# Check if fzf is installed
if ! command -v fzf &> /dev/null; then
    echo "Error: fzf is not installed. Please install fzf first."
    echo "Visit: https://github.com/junegunn/fzf"
    exit 1
fi

# Default values
DFS_SEARCH="../build/bin/dfs-search"
PATTERN="${1:-*}"
DIRECTORY="${2:-.}"
MAX_DEPTH="${3:-10}"

# Colors for fzf preview
export FZF_DEFAULT_OPTS="
    --ansi
    --preview 'file {} | head -1; echo; ls -lh {} 2>/dev/null || cat {} 2>/dev/null | head -100'
    --preview-window=right:60%:wrap
    --bind 'ctrl-/:toggle-preview'
    --bind 'ctrl-y:execute-silent(echo {} | pbcopy)+abort'
    --header 'CTRL-/: toggle preview | CTRL-Y: copy path | ENTER: open'
"

# Run dfs-search and pipe to fzf
selected=$(
    "$DFS_SEARCH" "$PATTERN" "$DIRECTORY" "$MAX_DEPTH" 2>/dev/null | \
    grep "^Found:" | \
    cut -d' ' -f2- | \
    fzf --multi \
        --prompt "Select file(s) > " \
        --header "Searching: $PATTERN in $DIRECTORY (depth: $MAX_DEPTH)" \
        --color "hl:#00ff00,hl+:#00ff00"
)

# Handle selection
if [ -n "$selected" ]; then
    # Check if EDITOR is set
    if [ -n "$EDITOR" ]; then
        echo "$selected" | xargs -I {} "$EDITOR" {}
    else
        # Try common editors
        if command -v vim &> /dev/null; then
            echo "$selected" | xargs -I {} vim {}
        elif command -v nano &> /dev/null; then
            echo "$selected" | xargs -I {} nano {}
        else
            # Just print the selection
            echo "Selected files:"
            echo "$selected"
        fi
    fi
else
    echo "No file selected."
fi
