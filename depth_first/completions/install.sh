#!/bin/bash
# Install shell completion scripts

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Detect shell
if [ -n "$BASH_VERSION" ]; then
    # Bash completion
    BASH_COMPLETION_DIR="${HOME}/.local/share/bash-completion/completions"
    mkdir -p "$BASH_COMPLETION_DIR"

    cp "$SCRIPT_DIR/dfs-search.bash" "$BASH_COMPLETION_DIR/dfs-search"
    echo "✓ Bash completion installed to $BASH_COMPLETION_DIR/dfs-search"
    echo "  Restart your shell or run: source $BASH_COMPLETION_DIR/dfs-search"

elif [ -n "$ZSH_VERSION" ]; then
    # Zsh completion
    ZSH_COMPLETION_DIR="${HOME}/.local/share/zsh/site-functions"
    mkdir -p "$ZSH_COMPLETION_DIR"

    cp "$SCRIPT_DIR/dfs-search.zsh" "$ZSH_COMPLETION_DIR/_dfs-search"
    echo "✓ Zsh completion installed to $ZSH_COMPLETION_DIR/_dfs-search"
    echo "  Add this to your ~/.zshrc if not already present:"
    echo "    fpath=(~/.local/share/zsh/site-functions \$fpath)"
    echo "    autoload -Uz compinit && compinit"
    echo "  Then restart your shell"

else
    echo "Unknown shell. Please install manually:"
    echo "  Bash: Copy dfs-search.bash to /etc/bash_completion.d/ or ~/.local/share/bash-completion/completions/"
    echo "  Zsh: Copy dfs-search.zsh to a directory in your \$fpath as _dfs-search"
fi
