/* sh8bench.c -- SmartHeap (tm) Portable memory management benchmark.
 *
 * Copyright (C) 2005 MicroQuill Software Publishing Corporation.
 * All Rights Reserved.
 *
 * No part of this source code may be copied, modified or reproduced
 * in any form without retaining the above copyright notice.
 * This source code, or source code derived from it, may not be redistributed
 * without express written permission of the copyright owner.
 *
 *
 * Compile-time flags.  Define the following flags on the compiler command-line
 * to include the selected APIs in the benchmark.  When testing an ANSI C
 * compiler, include MALLOC_ONLY flag to avoid any SmartHeap API calls.
 * Define these symbols with the macro definition syntax for your compiler,
 * e.g. -DMALLOC_ONLY=1 or -d MALLOC_ONLY=1
 *
 *  Flag                   Meaning
 *  -----------------------------------------------------------------------
 *  SYS_MULTI_THREAD=1  Test with multiple threads (Windows, UNIX)
 *  SMARTHEAP=1         Required when compiling if linking with SmartHeap lib
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <limits.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_MT) || defined(_REENTRANT)
#define SYS_MULTI_THREAD 1
#endif

#if defined(_Windows) || defined(_WINDOWS) || defined(WIN32) || defined(__WIN32__) || defined(_WIN32)
#include <windows.h>
#define WIN32 1
#if SYS_MULTI_THREAD
#include <process.h>
typedef HANDLE ThreadID;
#define THREAD_NULL (ThreadID)-1
typedef void (*ThreadFn)(void *);
#define THREAD_FN
#define THREAD_FN_RETURN void
#endif /* SYS_MULTI_THREAD */

#elif defined(UNIX) || defined(__hpux) || defined(_AIX) || defined(__sun) \
	|| defined(sgi) || defined(__DGUX__) || defined(__linux__)
/* Unix prototypes */
#ifndef UNIX
#define UNIX 1
#endif
#include <unistd.h>
#ifdef SYS_MULTI_THREAD
#if defined(__hpux) || defined(__osf__) || defined(_AIX) || defined(sgi) \
	|| defined(__DGUX__) || defined(__linux__) || defined(__MVS__)
#define _INCLUDE_POSIX_SOURCE
#define _OPEN_SYS 1
#include <signal.h>
#include <pthread.h>
typedef pthread_t ThreadID;
#if (defined(_DECTHREADS_) && !defined(__osf__)) || defined(__MVS__)
#define THREAD_NULL ThreadNULL
#define THREAD_EQ(a,b) pthread_equal(a,b)
#ifdef __MVS__
#define _POSIX_THREADS 1
ThreadID THREAD_NULL = {"0000000"};
#define _SC_NPROCESSORS_CONF 3
typedef void (*ThreadFn)(void *);
#define THREAD_FN
#else
ThreadID ThreadNULL = {0, 0, 0};
#endif /* __MVS__ */
#elif defined(__linux__)
#include <sys/sysinfo.h>
#endif /* __hpux etc */

#elif defined(__sun)
#include <thread.h>
typedef thread_t ThreadID;
#endif /* __sun */
#endif /* SYS_MULTI_THREAD */

#endif /* end of environment-specific header files */

#ifdef SMARTHEAP
#include "smrtheap.h"
#endif

#ifndef THREAD_NULL
#define THREAD_NULL 0
#endif
#ifndef THREAD_EQ
#define THREAD_EQ(a,b) ((a)==(b))
#endif
#ifndef THREAD_FN
typedef void * (*ThreadFn)(void *);
#define THREAD_FN_RETURN void *
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef SILENT
void fprintf_silent(FILE *, ...);
void fprintf_silent(FILE *x, ...) { (void)x; }
#else
#define fprintf_silent fprintf
#endif

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#ifdef CLK_TCK
#undef CLK_TCK
#endif
#define CLK_TCK CLOCKS_PER_SEC

#define TRUE 1
#define FALSE 0
typedef int Bool;

FILE *fout, *fin;
unsigned uMaxBlockSize = 1000;
unsigned uMinBlockSize = 1;
unsigned long ulIterations   =  100000;
unsigned long ulHeapSize     = 1000000;
unsigned ulForeignAllocCount = 10000000;

typedef struct _BlockSizeEntry
{
	unsigned long size;
	unsigned count;
} BlockSizeEntry;

static BlockSizeEntry blockSizeHistogramDefault[] =
{ {8, 1000}, {16, 5000}, {48, 1000}, {72, 100}, {148, 100},
  {200, 100}, {520, 10}, {1056, 5}, {4096, 3}, {9162, 1}, {34562, 1},
  {168524, 1}};

unsigned blockSizes = sizeof(blockSizeHistogramDefault)
							 / sizeof(BlockSizeEntry);

static BlockSizeEntry *blockSizeHistogram = &blockSizeHistogramDefault[0];


typedef struct _InterThreadBuffer
{
	void ***buf;
	long index;
} InterThreadBuffer;


unsigned long promptAndRead(char *msg, unsigned long defaultVal, char fmtCh);

#ifdef SYS_MULTI_THREAD
unsigned long ulThreadCount = 16;
ThreadID RunThread(ThreadFn fn, void *arg);
void WaitForThreads(ThreadID[], unsigned);
int GetNumProcessors(void);
#else
unsigned long ulThreadCount = 1;
#endif

THREAD_FN_RETURN doBench(void *);


void main(int argc, char *argv[])
{
	clock_t startCPU;
	time_t startTime;
	double elapsedTime, cpuTime;
	Bool failed = TRUE;

#ifdef SMARTHEAP
	MemRegisterTask();
#endif

	setbuf(stdout, NULL);  /* turn off buffering for output */

	/* @@@ change to use flags specifying which parms are passed */

	if (argc > 1)
		fin = fopen(argv[1], "r");
	else
		fin = stdin;
	if (argc > 2)
		fout = fopen(argv[2], "w");
	else
		fout = stdout;

	if (argc > 3)
		ulHeapSize = atol(argv[3]);
	else
		ulHeapSize = promptAndRead("heap size (# of blocks)", ulHeapSize, 'u');

	if (argc > 4)
		ulIterations = atol(argv[4]);
	else
		ulIterations = promptAndRead("iterations", ulIterations, 'u');

#ifdef SYS_MULTI_THREAD
	if (argc > 5)
		ulThreadCount = atol(argv[5]);
	else
		ulThreadCount = promptAndRead("threads", ulThreadCount, 'u');

	if (argc > 6)
		ulForeignAllocCount = atol(argv[5]);
	else
		ulForeignAllocCount =
							promptAndRead("allocs freed from non-allocating thread",
											  ulForeignAllocCount, 'u');

#endif /* SYS_MULTI_THREAD */

	/* @@@
	uMinBlockSize = (unsigned)promptAndRead("min block size",uMinBlockSize,'u');
	uMaxBlockSize = (unsigned)promptAndRead("max block size",uMaxBlockSize,'u');
	*/

#ifdef SYS_MULTI_THREAD
	{
		unsigned i, thr;
		void * volatile * volatile * volatile interThreadMem = NULL;
		InterThreadBuffer *interThreadBuf = NULL;
		ThreadID *tids = NULL;
		void *threadArg = NULL;

#ifdef WIN32
		unsigned uCPUs = promptAndRead("CPUs (0 for all)", 0, 'u');

		if (uCPUs)
		{
			DWORD_PTR m1, m2;

			if (GetProcessAffinityMask(GetCurrentProcess(), &m1, &m2))
			{
				i = 0;
				m1 = 1;

				/*
				 * iterate through process affinity mask m2, counting CPUs up to
				 * the limit specified in uCPUs
				 */
				do
					if (m2 & m1)
						i++;
				while ((m1 <<= 1) && i < uCPUs);

				/* clear any extra CPUs in affinity mask */
				do
					if (m2 & m1)
						m2 &= ~m1;
				while (m1 <<= 1);

				if (SetProcessAffinityMask(GetCurrentProcess(), m2))
					fprintf(fout,
							  "\nThreads in benchmark will run on max of %u CPUs", i);
			}
		}
#endif /* WIN32 */


#ifdef __MVS__
		pthread_set_limit_np(__STL_MAX_THREADS, 0, max(ulThreadCount, 100));
#endif

		if (ulThreadCount < 1)
			ulThreadCount = 1;

		/* make sure foreign alloc count is a multiple of
		 * square of thread count
		 */
		ulForeignAllocCount /= (ulThreadCount * ulThreadCount);
		ulForeignAllocCount *= (ulThreadCount * ulThreadCount);

		if ((tids = malloc(sizeof(ThreadID) * ulThreadCount)) == NULL)
			goto outOfMemory;

		if (ulForeignAllocCount != 0)
		{
			if ((interThreadMem = malloc(sizeof(void *) * ulForeignAllocCount))
					 == NULL
				 || (interThreadBuf
					  = malloc(sizeof(InterThreadBuffer) * ulThreadCount))
						== NULL)
				goto outOfMemory;

			/* init buffer for allocations to be freed from other threads */
			memset(interThreadMem, 0, sizeof(void *) * ulForeignAllocCount);

		}

		failed = FALSE;

		startCPU = clock();
		startTime = time(NULL);

		for (thr = 0;  thr < ulThreadCount;  thr++)
		{
			if (ulForeignAllocCount != 0)
			{
				interThreadBuf[thr].index = thr;
				interThreadBuf[thr].buf = interThreadMem;

				threadArg = &interThreadBuf[thr];
			}

			if (THREAD_EQ(tids[thr] = RunThread(doBench, threadArg),
											THREAD_NULL))
			{
				fprintf(fout, "\nfailed to start thread #%d", thr);
				thr--;
				ulThreadCount--;
			}

		}

			if (ulForeignAllocCount != 0)
			{
				int i;
				void * volatile * volatile * volatile foreignAllocs =
					interThreadMem;

				for (i = 0;  i < ulForeignAllocCount;  i++)
				{
					/*	make all foreign allocs equal size to maximize collisions */
					void * volatile * volatile alloc = malloc(blockSizeHistogram[0].size);

					if (alloc)
						*alloc = foreignAllocs;

					*foreignAllocs++ = alloc;
				}
			}
		WaitForThreads(tids, ulThreadCount);

		if (tids)
			free(tids);
		if (interThreadMem)
			free(interThreadMem);
		if (interThreadBuf)
			free(interThreadBuf);
	}
#else
	startCPU = clock();
	startTime = time(NULL);
	failed = FALSE;
	doBench(NULL);
#endif

	elapsedTime = difftime(time(NULL), startTime);
	cpuTime = (double)(clock()-startCPU) / (double)CLK_TCK;

	fprintf_silent(fout, "\n");
	fprintf(fout, "\nTotal elapsed time"
#ifdef SYS_MULTI_THREAD
			  " for %d threads"
#endif
			  ": %.2f (%.4f CPU)\n",
#ifdef SYS_MULTI_THREAD
			  ulThreadCount,
#endif
			  elapsedTime, cpuTime);


outOfMemory:
	if (failed)
		printf("\nout of memory");

end:
	if (fin != stdin)
		fclose(fin);
	if (fout != stdout)
		fclose(fout);
}


THREAD_FN_RETURN doBench(void *arg)
{
	InterThreadBuffer *interThreadBuf = arg;
	const long foreignAllocSlotSize = ulForeignAllocCount / ulThreadCount;
	const long foreignFreesPerThreadReset
		= foreignAllocSlotSize / ulThreadCount;
	long foreignAllocsLeft = arg ? foreignAllocSlotSize : 0;
	long foreignFreesLeft = arg ? foreignAllocSlotSize : 0;
	long foreignFreesPerThread = foreignFreesPerThreadReset;
	void ***foreignAllocs;
	void ***foreignFrees;
	unsigned long iterations = ulIterations / ulThreadCount;
	unsigned long heapSize = ulHeapSize / ulThreadCount;
	char **memory;
	char **mp;
	char **mpe;
	char **save_start;
	char **save_end;
	int sizeIndex = 0, count, size;

#if defined(SMARTHEAP) && defined(SYS_MULTI_THREAD)
	MemDefaultPoolThreadExclusive(MEM_THREAD_EXCLUSIVE_ON
											| MEM_FREE_ON_THREAD_TERM);
#endif

	memory = malloc(heapSize * sizeof(void *));
	mp = memory;
	mpe = memory + heapSize;
	save_start = mpe;
	save_end = mpe;

	/*
	@@@ change to:
			- do all allocs in main thread
			- pause at entry to doBench
			- do all inter-thread frees at once before any allocs
			- then do allocs which are always freed in allocating thread
			- introduce opt that, with an API call, creates thread-specific
			  default pool with serialization off, to be used only in threads
			  that exclusively free their own allocs (SH terminates pool
		     authomatically on thread termiation, so additional opt is
		     that free is optional)
			  */

	if (interThreadBuf)
	{
		/* each thread stores allocations in a thread-specific slot of the
		 * inter-thread buffer
		 */
		foreignAllocs
			= interThreadBuf->buf + (foreignAllocsLeft * interThreadBuf->index);

		/* each thread frees some allocations from each thread's slot */
		foreignFrees
			= interThreadBuf->buf + (foreignFreesPerThread*interThreadBuf->index);
	}

	if (!memory)
	{
		printf("\nout of memory");
		return;
	}

	while (iterations--)
	{
		int i;

#if 0
		/* store allocs to be freed by other threads contiguously starting
		 * at this thread's index
		 * alloc in bursts of 1000
		 */
		for (i = 0;  foreignAllocsLeft && i < 10;  i++)
		{
			/* make all foreign allocs equal size to maximize collisions */
			void **alloc = malloc(blockSizeHistogram[0].size);

			if (alloc)
				*alloc = foreignAllocs;

			*foreignAllocs++ = alloc;
			foreignAllocsLeft--;
		}
#endif

		/* free foreign allocs (those created by other threads) in bursts of 100;
		 * distribute frees across foreign allocs of each of the other threads
		 */
		for (i = 0;  foreignFreesLeft /* && i < 100 */;  i++)
		{
			if (*foreignFrees)
			{
				assert(**foreignFrees == foreignFrees);
				free(*foreignFrees++);
				foreignFreesLeft--;

				/* distribute frees evenly across all thread's allocs */
				if (--foreignFreesPerThread == 0)
				{
					foreignFreesPerThread = foreignFreesPerThreadReset;
					foreignFrees += (foreignAllocSlotSize - foreignFreesPerThread);
				}
			}
			else
				break;
		}

		/* choose a block size and count for current iteration */
/* @@@ sizeIndex = (rand() % blockSizes);		 */
		if (++sizeIndex == blockSizes)
			sizeIndex = 0;

		count = blockSizeHistogram[sizeIndex].count;
		size = blockSizeHistogram[sizeIndex].size;

		while (count--)
		{
			/* while allocating skip over that portion of the buffer that still
			 *  holds pointers from the previous cycle
			 */
			if (mp == save_start)
				mp = save_end;

			if (mp >= mpe)   /* if we've reached the end of the malloc buffer */
			{
				mp = memory;

				/* mark the next portion of the buffer */
				save_start = save_end;
				if (save_start >= mpe)
					save_start = mp;

				save_end = save_start + (heapSize / 5);
				if (save_end > mpe)
					save_end = mpe;

				/* free the bottom and top parts of the buffer.
				 * The bottom part is freed in the order of allocation.
				 * The top part is free in reverse order of allocation.
				 */
				while (mp < save_start)
					free(*mp++);

				mp = mpe;

				while (mp > save_end)
					free(*--mp);

				mp = memory;
			}
			else if (!(*mp++ = (char *)malloc(size)))
			{
				printf("Out of memory\n");
				_exit (1);
			}
		}
	}

	/* free the residual allocations */
#if 0
	/* note that we don't have to free as
	 * MEM_POOL_FREE_AT_THREAD_TERM was specified
	 */
	mpe = mp;
	mp = memory;

	while (mp < mpe)
		free(*mp++);

	if (mp < save_start)
		mp = save_start;

	while (mp < save_end)
		free(*mp++);

	free(memory);
#endif

#if 0
	MemPoolCheck(MemDefaultPool);
	MemPoolCheck(0);
#endif
}

unsigned long promptAndRead(char *msg, unsigned long defaultVal, char fmtCh)
{
	char *arg = NULL, *err;
	unsigned long result;
	{
		char buf[12];
		static char fmt[] = "\n%s [%lu]: ";
		fmt[7] = fmtCh;
		fprintf_silent(fout, fmt, msg, defaultVal);
		if (fgets(buf, 11, fin))
			arg = &buf[0];
	}
	if (arg && ((result = strtoul(arg, &err, 10)) != 0
					|| (*err == '\n' && arg != err)))
	{
		return result;
	}
	else
		return defaultVal;
}


/*** System-Specific Interfaces ***/

#ifdef SYS_MULTI_THREAD
ThreadID RunThread(ThreadFn fn, void *arg)
{
	ThreadID result = THREAD_NULL;

#if defined(__OS2__) && (defined(__IBMC__) || defined(__IBMCPP__) || defined(__WATCOMC__))
	if ((result = _beginthread(fn, NULL, 8192, arg)) == THREAD_NULL)
		return THREAD_NULL;

#elif (defined(__OS2__) && defined(__BORLANDC__)) || defined(WIN32)
	if ((result = (ThreadID)_beginthread(fn, 8192, arg)) == THREAD_NULL)
		return THREAD_NULL;

#elif defined(__sun)
/*	thr_setconcurrency(2); */
	return thr_create(NULL, 0, (void *(*)(void *))fn, arg, THR_BOUND, NULL)==0;

#elif defined(_DECTHREADS_) && !defined(__osf__)
	if (pthread_create(&result, pthread_attr_default,
							 (pthread_startroutine_t)fn, arg) == -1)
		 return THREAD_NULL;

#elif defined(_POSIX_THREADS) || defined(_POSIX_REENTRANT_FUNCTIONS)
	pthread_attr_t attr;
	pthread_attr_init(&attr);
#ifdef _AIX
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_UNDETACHED);
#endif /* _AIX */
	if (pthread_create(&result, &attr, fn, arg) == -1)
		return THREAD_NULL;

#else
#error Unsupported threads package
#endif

	return result;
}

/* wait for all benchmark threads to terminate */
/* @@@ return boolean */
void WaitForThreads(ThreadID tids[], unsigned tidCnt)
{
#ifdef __OS2__
	while (tidCnt--)
		DosWaitThread(&tids[tidCnt], DCWW_WAIT);

#elif defined(WIN32)
	WaitForMultipleObjects(tidCnt, tids, TRUE, INFINITE);
	GetLastError();

#elif defined(__sun)
	int prio;
	thr_getprio(thr_self(), &prio);
	thr_setprio(thr_self(), max(0, prio-1));
	while (tidCnt--)
		thr_join(0, NULL, NULL);

#elif defined(_POSIX_THREADS) || defined(_POSIX_REENTRANT_FUNCTIONS)
	while (tidCnt--)
		pthread_join(tids[tidCnt], NULL);

#endif
}

#ifdef __hpux
#include <sys/pstat.h>
/*#include <sys/mpctl.h>*/
#elif defined(__DGUX__)
#include <sys/dg_sys_info.h>
#endif

/* return the number of processors present */
int GetNumProcessors()
{
#ifdef WIN32
	SYSTEM_INFO info;

	GetSystemInfo(&info);

	return info.dwNumberOfProcessors;

#elif defined(UNIX)

#ifdef __hpux
	struct pst_dynamic psd;
	if (pstat_getdynamic(&psd, sizeof(psd), 1, 0) != -1)
		return psd.psd_proc_cnt;
/*	shi_bSMP mpctl(MPC_GETNUMSPUS, 0, 0) > 1; */

#elif defined(sgi)
	return sysconf(_SC_NPROC_CONF);
#elif defined(__DGUX__)
	struct dg_sys_info_pm_info info;

	if (dg_sys_info((long *)&info, DG_SYS_INFO_PM_INFO_TYPE,
						 DG_SYS_INFO_PM_CURRENT_VERSION)
		 == 0)
		 return info.idle_vp_count;
	else
		return 0;
#elif defined(__linux__)
	return get_nprocs();
#elif defined(_SC_NPROCESSORS_ONLN)
	return sysconf(_SC_NPROCESSORS_ONLN);

#elif defined(_SC_NPROCESSORS_CONF)
	return sysconf(_SC_NPROCESSORS_CONF);
#endif

#endif

	return 1;
}
#endif /* SYS_MULTI_THREAD */
