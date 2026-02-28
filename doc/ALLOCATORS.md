# Memory Allocators

This document lists the memory allocators supported by the `mimalloc-bench` suite, including their versions and source repository URLs.

| ID | Allocator Name | Version | Download URL |
|:---|:---|:---|:---|
| **dh** | DieHarder | `master` | [github.com/emeryberger/DieHard](https://github.com/emeryberger/DieHard) |
| **ff** | ffmalloc | `master` | [github.com/bwickman97/ffmalloc](https://github.com/bwickman97/ffmalloc) |
| **fg** | FreeGuard | `master` | [github.com/UTSASRG/FreeGuard](https://github.com/UTSASRG/FreeGuard) |
| **gd** | Guarder | `master` | [github.com/UTSASRG/Guarder](https://github.com/UTSASRG/Guarder) |
| **hd** | Hoard | `3.13` | [github.com/emeryberger/Hoard](https://github.com/emeryberger/Hoard) |
| **hm** | Hardened Malloc | `14` | [github.com/GrapheneOS/hardened_malloc](https://github.com/GrapheneOS/hardened_malloc) |
| **iso** | isoalloc | `1.2.5` | [github.com/struct/isoalloc](https://github.com/struct/isoalloc) |
| **je** | jemalloc | `5.3.0` | [github.com/jemalloc/jemalloc](https://github.com/jemalloc/jemalloc) |
| **lf** | lockfree-malloc | `master` | [github.com/Begun/lockfree-malloc](https://github.com/Begun/lockfree-malloc) |
| **lp** | libpas | `main` | [github.com/WebKit/WebKit](https://github.com/WebKit/WebKit) |
| **lt** | ltalloc | `master` | [github.com/r-lyeh-archived/ltalloc](https://github.com/r-lyeh-archived/ltalloc) |
| **mesh** | Mesh | `master` | [github.com/plasma-umass/mesh](https://github.com/plasma-umass/mesh) |
| **mi** | mimalloc (v1.x) | `v1.9.7` | [github.com/microsoft/mimalloc](https://github.com/microsoft/mimalloc) |
| **mi2** | mimalloc (v2.x) | `v2.2.7` | [github.com/microsoft/mimalloc](https://github.com/microsoft/mimalloc) |
| **mi3** | mimalloc (v3.x) | `v3.2.8` | [github.com/microsoft/mimalloc](https://github.com/microsoft/mimalloc) |
| **mng** | mallocng | `master` | [github.com/richfelker/mallocng-draft](https://github.com/richfelker/mallocng-draft) |
| **nomesh** | Mesh (no meshing) | `master` | [github.com/plasma-umass/mesh](https://github.com/plasma-umass/mesh) |
| **pa** | PartitionAlloc | `main` | [github.com/1c3t3a/partition_alloc_builder.git](https://github.com/1c3t3a/partition_alloc_builder.git) |
| **rp** | rpmalloc | `1.4.5` | [github.com/mjansson/rpmalloc](https://github.com/mjansson/rpmalloc) |
| **sc** | scalloc | `master` | [github.com/cksystemsgroup/scalloc](https://github.com/cksystemsgroup/scalloc) |
| **scudo** | Scudo | `main` | [github.com/llvm/llvm-project](https://github.com/llvm/llvm-project) |
| **sg** | SlimGuard | `master` | [github.com/ssrg-vt/SlimGuard](https://github.com/ssrg-vt/SlimGuard) |
| **sm** | Supermalloc | `master` | [github.com/kuszmaul/SuperMalloc](https://github.com/kuszmaul/SuperMalloc) |
| **sn** | snmalloc | `0.7.3` | [github.com/Microsoft/snmalloc](https://github.com/Microsoft/snmalloc) |
| **tbb** | Intel TBB | `v2022.3.0` | [github.com/uxlfoundation/oneTBB](https://github.com/uxlfoundation/oneTBB) |
| **tc** | tcmalloc (gperftools) | `gperftools-2.18` | [github.com/gperftools/gperftools](https://github.com/gperftools/gperftools) |
| **tcg** | tcmalloc (Google) | `98fd2430` | [github.com/google/tcmalloc](https://github.com/google/tcmalloc) |
| **yal** | yalloc | `main` | [github.com/jorisgeer/yalloc](https://github.com/jorisgeer/yalloc) |
| **sys** | System Allocator | `system` | N/A (Part of the OS) |

*Note: For some allocators (like `tcg`), the specific commit hash used in the build script is provided to ensure reproducibility.*
