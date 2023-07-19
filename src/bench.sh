# This script runs perf on the different version of the sytX programs.

SRC=syt
SAMPLES=5
THRESHOLD=22

echo "Benchmark for StandardYoungTableux Computation."
echo "Samples per case: "$SAMPLES
echo "Threshold       : "$THRESHOLD

for i in {0..1}
do
    name=$SRC$i
    rootdir=$(pwd)/perf/$name

    echo "Program "$name":"

    mkdir -p $rootdir

    cd $name

    for m in {3..7}
    do
        for n in {3..7}
        do
            perm=$(echo "$m*$n" | bc)
            if [[ $perm -gt $THRESHOLD ]]; then
                continue
            fi

            echo -n "- Case "$m"x"$n

            perf_file=$rootdir"/m"$m"n"$n

            if [[ ! -f "$perf_file" ]]; then
                3>$perf_file perf stat --null -r 5 --log-fd 3 obj/$name.out $m $n >/dev/null
            fi

            echo " [DONE]" 
            echo $(tail -2 $perf_file | head)

            rm -f raw/*
        done
    done 

    cd ..
done
