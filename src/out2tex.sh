# Converts csv input to a tex document with tables.
# @parameters:
#   $1 = Number of columns in latex document
#
echo "\\documentclass{article}"
echo "\\usepackage{multicol}"
echo "\\begin{document}"
echo "\\begin{multicols}{4}"

while read -r line
do
    nsq=$(echo $line | tr ',' ' ' | wc -w)
    n=$(echo "sqrt($nsq)" | bc)
    echo "\\begin{displaymath}"
    echo -n "\\begin{array}{"
    up2n=$(seq 1 $n)
    for i in $up2n
    do
        echo -n "|c"
    done
    echo "|}"
    nlc=0 #newlinecounter
    for (( i=0; i<=${#line}; i++ ))
    do
        c=${line:$i:1}
        if [[ "$c" == "," ]]; then
            ((nlc++))
            if [[ $(($nlc % $n)) -eq 0 ]]; then
                echo "\\\\"
            else
                echo -n "&"
            fi
        else
            echo -n $c
        fi
    done
    echo ""
    echo "\\end{array}"
    echo "\\end{displaymath}"
done

echo "\\end{multicols}"
echo "\\end{document}"
