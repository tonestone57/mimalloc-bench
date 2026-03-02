#!/bin/bash
ALLOCS="sn-dbg tbb tc tcg mi-dbg mi2-dbg mi3-dbg yal"
RESULTS="benchres_complete.csv"
cd out/bench
for a in $ALLOCS; do
  echo "Testing $a..."
  ../../bench.sh $a cfrac --procs=4
  tail -n 1 benchres.csv >> ../../$RESULTS
done
