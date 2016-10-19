# The following is a list of handy bash functions to help you get
# more done in less time

# Handy Aliases
alias mdc='make distclean'

# For finding usages of words in files
function fu {
    # Assume search all files if no second arg provided
    if [ $# -eq 1 ]; then
        grep -rnI "$1" .
    fi

    # Search a specific set of files
    if [ $# -gt 1 ]; then
        grep -rnI "$1" $2
    fi
}

