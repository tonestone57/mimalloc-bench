#!/bin/bash
ALLOCS="sys dh ff fg gd hd hm hml iso je lf lp lt mi mi-sec mi2 mi2-sec mi3 mi3-sec mng om mesh nomesh rp scudo sg sm sn sn-sec sn-dbg tbb tc tcg mi-dbg mi2-dbg mi3-dbg yal"
RESULTS="benchres_complete.csv"
echo "# benchmark allocator elapsed rss user sys page-faults page-reclaims" > $RESULTS
cd out/bench
for a in $ALLOCS; do
  echo "Testing $a..."
  ../../bench.sh $a cfrac --procs=4
  tail -n 1 benchres.csv >> ../../$RESULTS
done
