# fish completion for jot. The candidates come from jot itself, which
# knows its commands, their options and what they take.
complete -c jot -f -a '(jot __complete --line (commandline -cp) 2>/dev/null)'
