#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <OS.h>
#include <stdint.h>

/*
 * haiku-time.c
 *
 * A specialized 'time' wrapper for Haiku OS that collects additional
 * resource usage information (RSS, page faults) that might be missing
 * from the standard 'getrusage' implementation on older Haiku versions.
 *
 * This supports the -a, -o, and -f flags as used by mimalloc-bench.
 */

int main(int argc, char** argv) {
    char* outfile = NULL;
    char* format = NULL;
    int cmd_idx = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            // ignore: we always append if -o is specified for simplicity
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outfile = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            format = argv[++i];
        } else if (argv[i][0] == '-') {
            // ignore other flags
        } else {
            cmd_idx = i;
            break;
        }
    }

    if (cmd_idx == -1 || cmd_idx >= argc) {
        fprintf(stderr, "Usage: %s [-a] [-o outfile] [-f format] command [args...]\n", argv[0]);
        return 1;
    }

    struct rusage start_ru;
    getrusage(RUSAGE_CHILDREN, &start_ru);
    system_info start_si;
    get_system_info(&start_si);
    bigtime_t start_real = system_time();

    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[cmd_idx], &argv[cmd_idx]);
        perror("execvp");
        exit(1);
    }

    size_t peak_rss = 0;
    int status;
    while (waitpid(pid, &status, WNOHANG) == 0) {
        team_info ti;
        if (get_team_info(pid, &ti) == B_OK) {
            size_t current_rss = 0;
            ssize_t cookie = 0;
            area_info ai;
            while (get_next_area_info(pid, &cookie, &ai) == B_OK) {
                current_rss += ai.ram_size;
            }
            if (current_rss > peak_rss) peak_rss = current_rss;
        }
        usleep(20000); // Poll every 20ms
    }

    bigtime_t end_real = system_time();
    struct rusage end_ru;
    getrusage(RUSAGE_CHILDREN, &end_ru);
    system_info end_si;
    get_system_info(&end_si);

    double elapsed = (end_real - start_real) / 1000000.0;
    double user_time = (end_ru.ru_utime.tv_sec - start_ru.ru_utime.tv_sec) +
                       (end_ru.ru_utime.tv_usec - start_ru.ru_utime.tv_usec) / 1000000.0;
    double sys_time = (end_ru.ru_stime.tv_sec - start_ru.ru_stime.tv_sec) +
                      (end_ru.ru_stime.tv_usec - start_ru.ru_stime.tv_usec) / 1000000.0;

    long max_rss = end_ru.ru_maxrss;
    long minflt = end_ru.ru_minflt;
    long majflt = end_ru.ru_majflt;

    /* Fallback for older Haiku where rusage fields are 0 */
    if (max_rss <= 0) {
        max_rss = (long)(peak_rss / 1024);
    }
    if (minflt <= 0 && majflt <= 0) {
        minflt = (long)(end_si.page_faults - start_si.page_faults);
        majflt = 0;
    }

    if (outfile && format) {
        FILE* f = fopen(outfile, "a");
        if (f) {
            for (char* p = format; *p; p++) {
                if (*p == '%' && *(p+1)) {
                    p++;
                    switch (*p) {
                        case 'E': {
                            int hours = (int)elapsed / 3600;
                            int min = ((int)elapsed % 3600) / 60;
                            double sec = elapsed - hours * 3600 - min * 60;
                            if (hours > 0) fprintf(f, "%d:%02d:%05.2f", hours, min, sec);
                            else fprintf(f, "%d:%05.2f", min, sec);
                            break;
                        }
                        case 'M': fprintf(f, "%ld", max_rss); break;
                        case 'U': fprintf(f, "%.2f", user_time); break;
                        case 'S': fprintf(f, "%.2f", sys_time); break;
                        case 'F': fprintf(f, "%ld", majflt); break;
                        case 'R': fprintf(f, "%ld", minflt); break;
                        case '%': fprintf(f, "%%"); break;
                        default: fprintf(f, "%%%c", *p); break;
                    }
                } else {
                    fputc(*p, f);
                }
            }
            fputc('\n', f);
            fclose(f);
        }
    }

    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 1;
}
