#!/bin/bash
extso=".so"
localdevdir="$(pwd)/extern"

check_lib() {
  if [ -f "$1" ]; then
    echo "[OK] $1"
  else
    echo "[MISSING] $1"
  fi
}

check_lib "$localdevdir/dh/build/libdieharder$extso"
check_lib "$localdevdir/ff/libffmallocnpmt$extso"
check_lib "$localdevdir/fg/libfreeguard$extso"
check_lib "$localdevdir/gd/libguarder$extso"
check_lib "$localdevdir/hd/src/libhoard$extso"
check_lib "$localdevdir/hm/out/libhardened_malloc$extso"
check_lib "$localdevdir/hm/out-light/libhardened_malloc-light$extso"
check_lib "$localdevdir/iso/build/libisoalloc$extso"
check_lib "$localdevdir/je/lib/libjemalloc$extso"
check_lib "$localdevdir/lf/liblite-malloc-shared$extso"
check_lib "$localdevdir/lp/Source/bmalloc/libpas/build-cmake-default/Release/libpas_lib$extso"
check_lib "$localdevdir/lt/gnu.make.lib/libltalloc$extso"
check_lib "$localdevdir/mesh/build/lib/libmesh$extso"
check_lib "$localdevdir/mng/libmallocng$extso"
check_lib "$localdevdir/sc/out/Release/lib.target/libscalloc$extso"
check_lib "$localdevdir/scudo/compiler-rt/lib/scudo/standalone/libscudo$extso"
check_lib "$localdevdir/sg/libSlimGuard.so"
check_lib "$localdevdir/sm/release/lib/libsupermalloc$extso"
check_lib "$localdevdir/sn/release/libsnmallocshim$extso"
check_lib "$localdevdir/sn/release/libsnmallocshim-checks$extso"
check_lib "$localdevdir/tbb/bench_release/libtbbmalloc_proxy$extso"
check_lib "$localdevdir/tc/.libs/libtcmalloc_minimal$extso"
check_lib "$localdevdir/tcg/bazel-bin/tcmalloc/libtcmalloc$extso"
check_lib "$localdevdir/yal/yalloc$extso"
check_lib "$localdevdir/mi/out/release/libmimalloc$extso"
check_lib "$localdevdir/mi2/out/release/libmimalloc$extso"
