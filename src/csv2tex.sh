# Converts csv input to a tex document with tables.
# @parameters:
#   $1 = Number of columns in latex document
#
echo "\\documentclass{article}"
echo "\\usepackage{multicol}"
echo "\\begin{document}"
echo "\\begin{multicols}{$1}"
echo "\\noindent"

LINECOUNT=0
while read -r line
do
    n=$(echo $line | cut -d';' -f1 | tr ',' ' ' | wc -w)
    echo -n "\\begin{tabular}{"
    up2n=$(seq 1 $n)
    for i in $up2n
    do
        echo -n "|c"
    done
    echo "|}"
    for (( i=0; i<=${#line}; i++ ))
    do
        c=${line:$i:1}
        case $c in
            [0-9]) echo -n $c ;;
            ',') echo -n "&" ;;
            ';') echo "\\\\" ;;
        esac
    done
    echo "\\\\\\hline"
    echo "\\end{tabular}"
    ((LINECOUNT++))
done

echo "\\end{multicols}"
echo "TOTAL: "$LINECOUNT
echo "\\end{document}"
