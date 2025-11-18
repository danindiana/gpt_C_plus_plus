#compdef dfs-search dfs-enhanced

_dfs_search() {
    local curcontext="$curcontext" state line
    typeset -A opt_args

    _arguments -C \
        '(-d --depth)'{-d,--depth}'[Maximum search depth]:depth:' \
        '(-r --regex)'{-r,--regex}'[Use regex pattern matching]' \
        '(-v --verbose)'{-v,--verbose}'[Verbose output]' \
        '(-p --parallel)'{-p,--parallel}'[Enable parallel search]' \
        '(-t --threads)'{-t,--threads}'[Number of threads]:threads:' \
        '(-f --format)'{-f,--format}'[Output format]:format:(plain colored json xml csv)' \
        '(-D --duplicates)'{-D,--duplicates}'[Find duplicate files]' \
        '(-c --content)'{-c,--content}'[Search file contents]:pattern:' \
        '(-C --config)'{-C,--config}'[Configuration file]:file:_files' \
        '(-e --exclude)'{-e,--exclude}'[Exclude path]:path:_directories' \
        '(-h --help)'{-h,--help}'[Show help]' \
        '1:pattern:' \
        '2:directory:_directories'
}

_dfs_search "$@"
