#!/bin/bash

SAMPLES=3
ROOTDIR=$(pwd)/perf/

mkdir -p $ROOTDIR
mkdir -p raw

echo "Benchmark for feasible syt as outer sums."
echo "Samples per case: "$SAMPLES

CASES="3x3 3x6 4x5 3x8"

make 2&>/dev/null

for c in $CASES
do
  echo -n "Case $c" 

  m=$(echo $c | cut -d"x" -f1)
  n=$(echo $c | cut -d"x" -f2)

  perf_file=$ROOTDIR"m"$m"n"$n
  if [[ ! -f "$perf_file" ]]; then
    3>$perf_file perf stat --null -r $SAMPLES --log-fd 3 ./runseq.sh $m $n
    echo $(tail -2 $perf_file | head) > $perf_file
  fi

  echo " [DONE]" 
  cat $perf_file
done
