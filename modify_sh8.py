
import os

def main():
    with open('bench/shbench/SH8BENCH.C', 'r') as f:
        lines = f.readlines()

    def find_line(pattern, start_index=0):
        for i in range(start_index, len(lines)):
            if pattern in lines[i]:
                return i
        return -1

    # Apply changes

    # 1. Header Haiku
    idx = find_line('|| defined(sgi) || defined(__DGUX__) || defined(__linux__)')
    if idx != -1:
        lines[idx] = '\t|| defined(sgi) || defined(__DGUX__) || defined(__linux__) || defined(__HAIKU__)\n'

    idx = find_line('#ifndef UNIX', idx)
    if idx != -1:
        idx_next = find_line('#endif', idx)
        if idx_next != -1:
            lines.insert(idx_next + 1, '#ifdef __HAIKU__\n#define HAVE_PTHREAD 1\n#endif\n')

    idx = find_line('|| defined(__DGUX__) || defined(__linux__) || defined(__MVS__)', idx)
    if idx != -1:
        lines[idx] = '\t|| defined(__DGUX__) || defined(__linux__) || defined(__MVS__) || defined(__HAIKU__)\n'

    # 2. smartheap.h
    idx = find_line('#include "smrtheap.h"')
    if idx != -1:
        lines[idx] = '// #include "smrtheap.h"\n'

    # 3. iterations
    idx = find_line('unsigned long ulIterations   =  100000;')
    if idx != -1:
        lines[idx] = 'unsigned long ulIterations   =  700000;\n'

    # 4. main
    idx = find_line('void main(int argc, char *argv[])')
    if idx != -1:
        lines[idx] = 'unsigned int defaultThreadCount = 1;\nint main(int argc, char *argv[])\n'

    # 5. BENCH setup
    idx = find_line('setbuf(stdout, NULL);  /* turn off buffering for output */')
    if idx != -1:
        lines.insert(idx + 1, '#ifdef BENCH\n\tfin = stdin;\n\tfout = stdout;\n\tdefaultThreadCount = GetNumProcessors();\n\tif (argc==2) {\n\t\tchar* end;\n\t\tlong l = strtol(argv[1],&end,10);\n\t\tif (l > 0) defaultThreadCount = l;\n\t}\n#else\n')

    idx = find_line('fout = stdout;', idx)
    if idx != -1:
        lines.insert(idx + 1, '#endif\n\n')

    # 7. args
    idx = find_line('ulThreadCount = promptAndRead("threads", ulThreadCount, \'u\');')
    if idx != -1:
        lines[idx] = '\t\tulThreadCount = promptAndRead("threads", defaultThreadCount, \'u\');\n'

    idx = find_line('ulForeignAllocCount = atol(argv[5]);', idx)
    if idx != -1:
        lines[idx] = '\t\tulForeignAllocCount = atol(argv[6]);\n'

    idx = find_line('promptAndRead("allocs freed from non-allocating thread"', idx)
    if idx != -1:
        lines[idx-1] = '\t\tulForeignAllocCount = promptAndRead("foreign alloc count", ulForeignAllocCount, \'u\');\n'
        # We need to remove the next few lines of the original prompt
        del lines[idx:idx+2]

    # 8. touch memory
    idx = find_line('else if (!(*mp++ = (char *)malloc(size)))')
    if idx != -1:
        lines[idx] = '\t\t\t\telse if (!(*mp = (char *)malloc(size)))\n'
        lines.insert(idx + 1, '\t\t\t\t{\n\t\t\t\t\tprintf("Out of memory\\n");\n\t\t\t\t\t_exit (1);\n\t\t\t\t}\n\t\t\t\telse\n\t\t\t\t{\n\t\t\t\t\tchar* p = *mp;\n\t\t\t\t\tp[0] = 0; p[size-1] = 0;\n\t\t\t\t\tmp++;\n\t\t\t\t}\n')

    # 10. promptAndRead guard
    idx = find_line('char buf[12];')
    if idx != -1:
        # Search backwards for the {
        j = idx
        while j > 0 and '{' not in lines[j]:
            j -= 1
        lines.insert(j + 1, '#ifndef BENCH\n')

        idx = find_line('arg = &buf[0];', j)
        if idx != -1:
            # Search forwards for the }
            j = idx
            while j < len(lines) and '}' not in lines[j]:
                j += 1
            lines.insert(j, '#endif\n') # Put #endif BEFORE the }

    # 12 & 13. POSIX threads
    posix_indices = []
    for i, line in enumerate(lines):
        if '#elif defined(_POSIX_THREADS) || defined(_POSIX_REENTRANT_FUNCTIONS)' in line:
            posix_indices.append(i)
    for idx in reversed(posix_indices):
        lines[idx] = '#elif defined(_POSIX_THREADS) || defined(_POSIX_REENTRANT_FUNCTIONS) || defined(__HAIKU__)\n'

    # 14. GetNumProcessors
    idx = find_line('return sysconf(_SC_NPROCESSORS_ONLN);')
    if idx != -1:
        lines.insert(idx + 1, '#elif defined(__HAIKU__)\n\treturn sysconf(_SC_NPROCESSORS_ONLN);\n')

    with open('bench/shbench/sh8bench-new.c', 'w') as f:
        f.writelines(lines)

if __name__ == "__main__":
    main()
