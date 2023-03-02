REPORTFILE=./cppcheck_report.txt
WARNINGSFILE=./warnings.txt
if test -f "$REPORTFILE"; then
    # Filter innocuous warnings and write the remaining ones to another file.
    # Note that you can add more warnings by adding it in the parenthesis,
    # with "\|" in front of it. For example, "(missingIncludeSystem\|useStlAlgorithm\)"
    sed 's/\[\(missingIncludeSystem\|useStlAlgorithm\)\]//g' "$REPORTFILE" > "$WARNINGSFILE"
    # are there any remaining warnings?
    if grep -qP '\[[a-zA-Z0-9]+\]' "$WARNINGSFILE"; then
        # print the remaining warnings
        echo Warnings detected:
        echo ==================
        cat "$WARNINGSFILE" | grep -P '\[[a-zA-Z0-9]+\]'
        # fail the job
        exit 1
    else
        echo No warnings detected
    fi
else
    echo "$REPORTFILE" not found
fi