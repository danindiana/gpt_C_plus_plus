#!/bin/bash
# dfs-fzf-enhanced: Enhanced interactive search with multiple actions

if ! command -v fzf &> /dev/null; then
    echo "Error: fzf not installed"
    exit 1
fi

DFS_SEARCH="../build/bin/dfs-enhanced"
PATTERN="${1:-*}"
DIRECTORY="${2:-.}"

# Enhanced preview with syntax highlighting if bat is available
if command -v bat &> /dev/null; then
    PREVIEW_CMD="bat --color=always --style=numbers --line-range=:100 {} 2>/dev/null || file {}"
else
    PREVIEW_CMD="head -100 {} 2>/dev/null || file {}"
fi

# Run search with colored output and pipe to fzf
selected=$(
    "$DFS_SEARCH" -f plain "$PATTERN" "$DIRECTORY" 2>/dev/null | \
    grep "^Found:" | \
    cut -d' ' -f2- | \
    fzf --multi \
        --ansi \
        --preview "$PREVIEW_CMD" \
        --preview-window=right:60%:wrap \
        --bind 'ctrl-/:toggle-preview' \
        --bind 'ctrl-o:execute(xdg-open {})' \
        --bind 'ctrl-y:execute-silent(echo {} | xclip -selection clipboard)+abort' \
        --bind 'ctrl-d:execute(rm -i {})' \
        --bind 'ctrl-e:execute($EDITOR {})' \
        --header 'CTRL-E: edit | CTRL-O: open | CTRL-Y: copy path | CTRL-D: delete | ENTER: select' \
        --prompt "🔍 Search > " \
        --color "hl:#5fff87,hl+:#ffaf5f,info:#af87ff,prompt:#5fff87,pointer:#ff87d7"
)

if [ -n "$selected" ]; then
    echo "Selected:"
    echo "$selected"
else
    echo "No selection."
fi
