SAMPLES=5
ROOTDIR=$(pwd)/perf/

mkdir -p $ROOTDIR

echo "Benchmark for StandardYoungTableux Computation."
echo "Samples per case: "$SAMPLES

CASES="3x4 3x6 4x5"

for c in $CASES
do
    echo -n "Case $c" 

    m=$(echo $c | cut -d"x" -f1)
    n=$(echo $c | cut -d"x" -f2)

    perf_file=$ROOTDIR"m"$m"n"$n
    if [[ ! -f "$perf_file" ]]; then
        mkdir -p raw
        3>$perf_file perf stat --null -r $SAMPLES --log-fd 3 obj/a.out $m $n
        echo $(tail -2 $perf_file | head) > $perf_file
    fi

    echo " [DONE]" 
    cat $perf_file

    rm -f raw/*
done
