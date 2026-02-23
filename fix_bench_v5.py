
import sys
import re
import os

def fix_sh6bench(content):
    # Add Haiku support to OS selection guards (Lines 53 and 61)
    content = content.replace(
        '|| defined(sgi) || defined(__DGUX__) || defined(__linux__)',
        '|| defined(sgi) || defined(__DGUX__) || defined(__linux__) || defined(__HAIKU__)'
    )
    content = content.replace(
        '|| defined(__DGUX__) || defined(__linux__)',
        '|| defined(__DGUX__) || defined(__linux__) || defined(__HAIKU__)'
    )

    # Haiku signal.h
    content = content.replace(
        '#include <sys/signal.h>',
        '#ifndef __HAIKU__\n#include <sys/signal.h>\n#else\n#include <signal.h>\n#endif'
    )

    # Haiku pthread
    content = content.replace(
        '#define UNIX 1\n#endif',
        '#define UNIX 1\n#endif\n#ifdef __HAIKU__\n#define HAVE_PTHREAD 1\n#endif'
    )

    # Comment out smrtheap.h
    content = content.replace('#include "smrtheap.h"', '// #include "smrtheap.h"')

    # Increase ulCallCount
    content = content.replace('unsigned long ulCallCount = 1000;', 'unsigned long ulCallCount = 10000;')

    # Main modifications for BENCH mode
    main_pattern = r'(setbuf\(stdout, NULL\);[^\n]*\n)'
    bench_main_init = """#ifdef BENCH
	fin = stdin;
	fout = stdout;
    unsigned int defaultThreadCount = GetNumProcessors();
	if (argc==2) {
		char* end;
		long l = strtol(argv[1],&end,10);
		if (l > 0) defaultThreadCount = l;
	}
#else
"""
    content = re.sub(main_pattern, r'\1' + bench_main_init, content)

    # Close #else after the second fout = stdout;
    content = re.sub(r'(fout = stdout;[^\n]*\n)\n\t(ulCallCount)', r'\1#endif\n\n\t\2', content)

    # uThreadCount and uCPUs
    content = content.replace('unsigned uCPUs = promptAndRead("CPUs (0 for all)", 0, \'u\');',
                             'unsigned uCPUs = promptAndRead("CPUs (0 for all)", PROCS, \'u\');')
    content = content.replace('uThreadCount = (int)promptAndRead("threads", GetNumProcessors(), \'u\');',
                             'uThreadCount = (int)promptAndRead("threads", defaultThreadCount, \'u\');')

    # if uThreadCount == 1 logic
    content = re.sub(
        r'(if \(uThreadCount < 1\)\s+uThreadCount = 1;)',
        r'\1\n\n\t\tif (uThreadCount==1) {\n\t\t\tstartCPU = clock();\n\t\t\tstartTime = time(NULL);\n\t\t\tdoBench(NULL);\n\t\t}\n\t\telse {',
        content
    )
    # Close the else {
    content = content.replace('if (threadArg)\n\t\t\tfree(threadArg);\n\t}', 'if (threadArg)\n\t\t\tfree(threadArg);\n\t\t}\n\t}')

    # exit(0) and other small fixes
    content = content.replace('if (fout != stdout)\n\t\tfclose(fout);', 'if (fout != stdout)\n\t\tfclose(fout);\n\texit(0);')

    # mp[-1] check
    content = re.sub(
        r'(_exit \(1\);)\s+}',
        r'\1\n\t\t\t\t}\n\t\t\t\tchar* p = mp[-1]; p[0] = 0; p[size-1] = 0;',
        content
    )

    # promptAndRead #ifndef BENCH
    content = content.replace('unsigned long result;\n\t{', 'unsigned long result;\n#ifndef BENCH\n\t{')
    content = content.replace('arg = &buf[0];\n\t}', 'arg = &buf[0];\n\t}\n#endif')

    # GetNumProcessors and pthread Haiku support
    content = content.replace('#elif defined(_POSIX_THREADS) || defined(_POSIX_REENTRANT_FUNCTIONS)',
                             '#elif defined(_POSIX_THREADS) || defined(_POSIX_REENTRANT_FUNCTIONS) || defined(__HAIKU__)')
    content = content.replace('return sysconf(_SC_NPROCESSORS_ONLN);', 'return sysconf(_SC_NPROCESSORS_ONLN);\n#elif defined(__HAIKU__)\n\treturn sysconf(_SC_NPROCESSORS_ONLN);')

    return content

def fix_sh8bench(content):
    # Signature
    content = content.replace('void main(int argc, char *argv[])', 'int main(int argc, char *argv[])')

    # Add Haiku support to OS selection guards
    content = content.replace(
        '|| defined(sgi) || defined(__DGUX__) || defined(__linux__)',
        '|| defined(sgi) || defined(__DGUX__) || defined(__linux__) || defined(__HAIKU__)'
    )
    content = content.replace(
        '|| defined(__DGUX__) || defined(__linux__) || defined(__MVS__)',
        '|| defined(__DGUX__) || defined(__linux__) || defined(__MVS__) || defined(__HAIKU__)'
    )

    # Haiku pthread
    content = content.replace(
        '#define UNIX 1\n#endif',
        '#define UNIX 1\n#endif\n#ifdef __HAIKU__\n#define HAVE_PTHREAD 1\n#endif'
    )

    # Comment out smrtheap.h
    content = content.replace('#include "smrtheap.h"', '// #include "smrtheap.h"')

    # Increase iterations
    content = content.replace('unsigned long ulIterations   =  100000;', 'unsigned long ulIterations   =  700000;')

    # Main modifications for BENCH mode
    main_start = r'(setbuf\(stdout, NULL\);[^\n]*\n)'
    bench_main_init = """#ifdef BENCH
	fin = stdin;
	fout = stdout;
    unsigned int defaultThreadCount = GetNumProcessors();
	if (argc==2) {
		char* end;
		long l = strtol(argv[1],&end,10);
		if (l > 0) defaultThreadCount = l;
	}
#else
"""
    content = re.sub(main_start, r'\1' + bench_main_init, content)
    # Close #else
    content = re.sub(r'(fout = stdout;[^\n]*\n)\s*\n\tif \(argc > 3\)', r'\1#endif\n\n\tif (argc > 3)', content)

    # defaultThreadCount
    content = content.replace('ulThreadCount = promptAndRead("threads", ulThreadCount, \'u\');',
                             'defaultThreadCount = promptAndRead("threads", ulThreadCount, \'u\');')
    content = content.replace('unsigned uCPUs = promptAndRead("CPUs (0 for all)", 0, \'u\');',
                             'unsigned uCPUs = promptAndRead("CPUs (GetNumProcessors() for all)", 0, \'u\');')

    # mp[-1] check
    content = re.sub(
        r'(_exit \(1\);)\s+}',
        r'\1\n\t\t\t}\n\t\t\t{ char* p = mp[-1]; p[0] = 0; p[size-1] = 0; }',
        content
    )

    # promptAndRead #ifndef BENCH
    content = content.replace('unsigned long result;\n\t{', 'unsigned long result;\n#ifndef BENCH\n\t{')
    content = content.replace('arg = &buf[0];\n\t}', 'arg = &buf[0];\n\t}\n#endif')

    # GetNumProcessors and pthread Haiku support
    content = content.replace('#elif defined(_POSIX_THREADS) || defined(_POSIX_REENTRANT_FUNCTIONS)',
                             '#elif defined(_POSIX_THREADS) || defined(_POSIX_REENTRANT_FUNCTIONS) || defined(__HAIKU__)')
    content = content.replace('return sysconf(_SC_NPROCESSORS_ONLN);', 'return sysconf(_SC_NPROCESSORS_ONLN);\n#elif defined(__HAIKU__)\n\treturn sysconf(_SC_NPROCESSORS_ONLN);')

    return content

def main():
    shbench_dir = 'bench/shbench'

    with open(os.path.join(shbench_dir, 'sh6bench.c'), 'r') as f:
        sh6 = f.read()
    sh6_fixed = fix_sh6bench(sh6)
    with open(os.path.join(shbench_dir, 'sh6bench-fixed.c'), 'w') as f:
        f.write(sh6_fixed)

    with open(os.path.join(shbench_dir, 'SH8BENCH.C'), 'r') as f:
        sh8 = f.read()
    sh8_fixed = fix_sh8bench(sh8)
    with open(os.path.join(shbench_dir, 'sh8bench-fixed.c'), 'w') as f:
        f.write(sh8_fixed)

if __name__ == "__main__":
    main()
