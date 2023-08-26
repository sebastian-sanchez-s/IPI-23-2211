SAMPLES=3
ROOTDIR=$(pwd)/perf/

mkdir -p $ROOTDIR

echo "Benchmark for StandardYoungTableux Computation."
echo "Samples per case: "$SAMPLES

CASES="3x3 3x6 4x5"

make 2&>/dev/null

for c in $CASES
do
    echo -n "Case $c" 

    m=$(echo $c | cut -d"x" -f1)
    n=$(echo $c | cut -d"x" -f2)

    # date -d @$(date -u +%s) +'%H:%m-%d/%m/%Y'
    perf_file=$ROOTDIR"m"$m"n"$n
    if [[ ! -f "$perf_file" ]]; then
        mkdir -p raw
        mv banned/c$m"r"$n banned/tmp
        3>$perf_file perf stat --null -r $SAMPLES --log-fd 3 ./obj/a.out $m $n
        echo $(tail -2 $perf_file | head) > $perf_file
        mv banned/tmp banned/c$m"r"$n
    fi

    echo " [DONE]" 
    cat $perf_file
done
