#!/bin/bash
# Bash completion for dfs-search

_dfs_search_completions() {
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    # Available options
    opts="--depth --regex --verbose --parallel --threads --format --duplicates --content --config --exclude --help -d -r -v -p -t -f -D -c -C -e -h"

    # Format options
    formats="plain colored json xml csv"

    case "${prev}" in
        -f|--format)
            COMPREPLY=( $(compgen -W "${formats}" -- "${cur}") )
            return 0
            ;;
        -d|--depth|-t|--threads)
            # Numeric input, no completion
            return 0
            ;;
        -C|--config)
            # File completion
            COMPREPLY=( $(compgen -f -- "${cur}") )
            return 0
            ;;
        -e|--exclude)
            # Directory completion
            COMPREPLY=( $(compgen -d -- "${cur}") )
            return 0
            ;;
        -c|--content)
            # No completion for pattern
            return 0
            ;;
        *)
            ;;
    esac

    # Check if we're at the pattern or directory argument
    local num_args=0
    for ((i=1; i<COMP_CWORD; i++)); do
        if [[ "${COMP_WORDS[i]}" != -* ]]; then
            ((num_args++))
        elif [[ "${COMP_WORDS[i]}" =~ ^-(d|t|f|C|e|c)$ ]] || \
             [[ "${COMP_WORDS[i]}" =~ ^--(depth|threads|format|config|exclude|content)$ ]]; then
            ((i++))  # Skip option argument
        fi
    done

    if [[ ${num_args} -eq 0 ]]; then
        # First non-option argument: pattern (no completion)
        if [[ ${cur} == -* ]]; then
            COMPREPLY=( $(compgen -W "${opts}" -- "${cur}") )
        fi
        return 0
    elif [[ ${num_args} -eq 1 ]]; then
        # Second non-option argument: directory
        if [[ ${cur} == -* ]]; then
            COMPREPLY=( $(compgen -W "${opts}" -- "${cur}") )
        else
            COMPREPLY=( $(compgen -d -- "${cur}") )
        fi
        return 0
    fi

    # Default: complete options
    if [[ ${cur} == -* ]]; then
        COMPREPLY=( $(compgen -W "${opts}" -- "${cur}") )
        return 0
    fi
}

complete -F _dfs_search_completions dfs-search
complete -F _dfs_search_completions dfs-enhanced
