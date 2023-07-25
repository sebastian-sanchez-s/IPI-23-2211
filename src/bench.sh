# This script runs perf on the different version of the sytX programs.

SRC=syt
SAMPLES=5

echo "Benchmark for StandardYoungTableux Computation."
echo "Samples per case: "$SAMPLES

CASES="3x3 4x4 3x5 4x5"
for i in {0..1}
do
    name=$SRC$i
    rootdir=$(pwd)/perf/$name

    echo "Program "$name":"

    mkdir -p $rootdir

    cd $name

    for c in $CASES
    do
        echo -n "Case $c" 

        m=$(echo $c | cut -d"x" -f1)
        n=$(echo $c | cut -d"x" -f2)

        perf_file=$rootdir"/m"$m"n"$n
        if [[ ! -f "$perf_file" ]]; then
            make $name >/dev/null
            mkdir -p raw
            3>$perf_file perf stat --null -r $SAMPLES --log-fd 3 obj/$name.out $m $n
            echo $(tail -2 $perf_file | head) > $perf_file
        fi

        echo " [DONE]" 
        cat $perf_file

        rm -f raw/*
    done

    cd ..
done
