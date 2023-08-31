#!/bin/bash
#/****************************************************************************
# * Copyright (C) 2023 by Sebastián Sánchez                                                      
# *                                                                          
# *  Permission is hereby granted, free of charge, to any person obtaining
# *  a copy of this software and associated documentation files (the
# *  "Software"), to deal in the Software without restriction, including
# *  without limitation the rights to use, copy, modify, merge, publish,
# *  distribute, sublicense, and/or sell copies of the Software, and to
# *  permit persons to whom the Software is furnished to do so, subject to
# *  the following conditions:
# *  
# *  The above copyright notice and this permission notice shall be
# *  included in all copies or substantial portions of the Software.
# *  
# *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# ****************************************************************************/


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
