cmake_minimum_required(VERSION 3.0)
project(mimalloc-bench CXX C)
set(CMAKE_CXX_STANDARD 17)

# Haiku OS detection
if (CMAKE_SYSTEM_NAME MATCHES "Haiku")
  set(HAIKU 1)
  message(STATUS "Configuring for Haiku OS")
  # On Haiku, common POSIX functions (like pthreads) are in libroot.
  # Explicitly linking -lpthread might not be needed or could even cause issues
  # if libroot provides them and there's a separate, possibly incomplete, libpthread.
  # For now, we'll assume libroot is sufficient and remove explicit -lpthread.
  # If specific pthread functionality is missing, we might need to add it back
  # or find Haiku-specific alternatives.
  set(CMAKE_THREAD_LIBS_INIT "") # Try clearing CMake's default pthread detection
  set(THREADS_PREFER_PTHREAD_FLAG OFF)
else()
  set(HAIKU 0)
endif()

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to *** Release ***")
  set(CMAKE_BUILD_TYPE "Release")
endif()

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

set(cfrac_sources
    cfrac.c
    pops.c pconst.c pio.c
    pabs.c pneg.c pcmp.c podd.c phalf.c
    padd.c psub.c pmul.c pdivmod.c psqrt.c ppowmod.c
    atop.c ptoa.c itop.c utop.c ptou.c errorp.c
    pfloat.c pidiv.c pimod.c picmp.c
    primes.c pcfrac.c pgcd.c)
PREPEND(cfrac_sources cfrac/ ${cfrac_sources})

set(espresso_sources
    cofactor.c cols.c compl.c contain.c cubestr.c cvrin.c cvrm.c cvrmisc.c cvrout.c
    dominate.c equiv.c espresso.c essen.c exact.c expand.c gasp.c getopt.c gimpel.c
    globals.c hack.c indep.c irred.c main.c map.c matrix.c mincov.c opo.c pair.c part.c
    primes.c reduce.c rows.c set.c setc.c sharp.c sminterf.c solution.c sparse.c unate.c
    utility.c verify.c)
PREPEND(espresso_sources espresso/ ${espresso_sources})

set(barnes_sources
    code.c code_io.c load.c grav.c getparam.c util.c)
PREPEND(barnes_sources barnes/ ${barnes_sources})

# turn off warnings..
message(STATUS "${CMAKE_C_COMPILER_ID}")
if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
  set(COMMON_WARNING_FLAGS " -w -Wno-implicit-function-declaration -Wno-implicit-int -Wno-int-conversion -Wno-return-mismatch -Wno-incompatible-pointer-types")
  string(APPEND CMAKE_C_FLAGS ${COMMON_WARNING_FLAGS})
  string(APPEND CMAKE_CXX_FLAGS ${COMMON_WARNING_FLAGS})
endif()

# Common libraries for most executables
set(COMMON_LIBS m)
if(NOT HAIKU)
  list(APPEND COMMON_LIBS pthread)
endif()

add_executable(cfrac ${cfrac_sources})
target_compile_options(cfrac PRIVATE $<$<C_COMPILER_ID:GNU>:-std=gnu89>)
target_compile_definitions(cfrac PRIVATE NOMEMOPT=1)
target_link_libraries(cfrac m) # Only m, pthread handled by COMMON_LIBS if needed

add_executable(espresso ${espresso_sources})
target_compile_options(espresso PRIVATE $<$<C_COMPILER_ID:GNU>:-std=gnu89>)
target_link_libraries(espresso m) # Only m

add_executable(barnes ${barnes_sources})
target_link_libraries(barnes m) # Only m

add_executable(larson larson/larson.cpp)
target_compile_options(larson PRIVATE -Wno-unused-result)
target_compile_definitions(larson PRIVATE CPP=1)
target_link_libraries(larson ${COMMON_LIBS})

add_executable(larson-sized larson/larson.cpp)
target_compile_options(larson-sized PRIVATE -Wno-unused-result -fsized-deallocation)
target_compile_definitions(larson-sized PRIVATE CPP=1 SIZED=1)
target_link_libraries(larson-sized ${COMMON_LIBS})

add_executable(alloc-test alloc-test/test_common.cpp alloc-test/allocator_tester.cpp)
target_compile_definitions(alloc-test PRIVATE BENCH=4)
target_link_libraries(alloc-test ${COMMON_LIBS})


if(NOT APPLE AND NOT HAIKU)  # Keep shbench disabled for Haiku for now, simplify first pass
  add_executable(sh6bench shbench/sh6bench-new.c)
  target_compile_definitions(sh6bench PRIVATE BENCH=1 SYS_MULTI_THREAD=1)
  target_link_libraries(sh6bench ${COMMON_LIBS})

  add_executable(sh8bench shbench/sh8bench-new.c)
  target_compile_definitions(sh8bench PRIVATE BENCH=1 SYS_MULTI_THREAD=1)
  target_link_libraries(sh8bench ${COMMON_LIBS})
endif()

add_executable(cache-scratch cache-scratch/cache-scratch.cpp)
target_link_libraries(cache-scratch ${COMMON_LIBS})

add_executable(cache-thrash cache-thrash/cache-thrash.cpp)
target_link_libraries(cache-thrash ${COMMON_LIBS})

add_executable(xmalloc-test xmalloc-test/xmalloc-test.c)
target_link_libraries(xmalloc-test ${COMMON_LIBS})

add_executable(malloc-large-old malloc-large/malloc-large-old.cpp)
target_link_libraries(malloc-large-old ${COMMON_LIBS})

add_executable(malloc-large malloc-large/malloc-large.cpp)
target_link_libraries(malloc-large ${COMMON_LIBS})

add_executable(mstress mstress/mstress.c)
target_link_libraries(mstress ${COMMON_LIBS})

add_executable(mleak mleak/mleak.c)
target_link_libraries(mleak ${COMMON_LIBS})

add_executable(rptest rptest/rptest.c rptest/thread.c rptest/timer.c)
target_compile_options(rptest PRIVATE -fpermissive)
target_include_directories(rptest PRIVATE rptest)
target_link_libraries(rptest ${COMMON_LIBS}) # m is already in COMMON_LIBS

if(NOT HAIKU)
  add_executable(glibc-simple glibc-bench/bench-malloc-simple.c)
  target_link_libraries(glibc-simple ${COMMON_LIBS})

  add_executable(glibc-thread glibc-bench/bench-malloc-thread.c)
  target_link_libraries(glibc-thread ${COMMON_LIBS})
endif()

add_subdirectory(security)
