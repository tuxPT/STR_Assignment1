/**********************************************************
 * Paulo Pedreiras, Oct/2020
 * DETI/UA/IT - Real-Time and Embedded Systems course
 *
 * sched_deadline linux example
 * Note: implementation of SCHED_DEADLINE is not standard
 * See man sched_setattr: 
 	...
 	CONFORMING TO
       These system calls are nonstandard Linux extensions.

 * struct sched_attr and methods sched_getattr() / sched_setattr() are still missing !
 *    
 * Working example can be found in 
 * https://www.kernel.org/doc/Documentation/scheduler/sched-deadline.txt
 *
 * The example code below is an adaptation of example code above mentioned
 ***********************************************************/

#define _GNU_SOURCE
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include <linux/unistd.h>
 #include <linux/kernel.h>
 #include <linux/types.h>
 #include <sys/syscall.h>
 #include <pthread.h>

 #define gettid() syscall(__NR_gettid)

 #define SCHED_DEADLINE	6

 /* Use the proper syscall numbers */
 #ifdef __x86_64__
 #define __NR_sched_setattr		314
 #define __NR_sched_getattr		315
 #endif

 #ifdef __i386__
 #define __NR_sched_setattr		351
 #define __NR_sched_getattr		352
 #endif

 #ifdef __arm__
 #define __NR_sched_setattr		380
 #define __NR_sched_getattr		381
 #endif

/* Declare structure sched_attr */
 struct sched_attr {
	__u32 size;

	__u32 sched_policy;
	__u64 sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	__s32 sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	__u32 sched_priority;

	/* SCHED_DEADLINE (nsec) */
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
 };

/* Implementation of sched_{get,set}attr API functions */
 int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
 {
	return syscall(__NR_sched_setattr, pid, attr, flags);
 }

 int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags)
 {
	return syscall(__NR_sched_getattr, pid, attr, size, flags);
 }


/*
 * Application code
 */ 

 void *edfTask(void *data)
 {
	struct timespec ta;
	struct sched_attr attr;
	unsigned int i, flags=0;
	int ret;

	printf("Deadline thread started [%ld]\n", gettid());

	/* Set SCHED_DEALINE attributes structure */
	attr.size = sizeof(attr);
	attr.sched_flags = 0;
	attr.sched_nice = 0;
	attr.sched_priority = 0; // Should be zero for all scheduling policies except RR and FIFO

	/* Creates a 10ms/30ms reservation */
	attr.sched_policy = SCHED_DEADLINE;
	attr.sched_runtime = 10 * 1000 * 1000; // Note that time specification fields are in ns
	attr.sched_period = attr.sched_deadline = 30 * 1000 * 1000;
	
	/* Set scheduling policy for thread*/
	ret = sched_setattr(0, &attr, flags);
	if (ret < 0) {		
		perror("sched_setattr");
		exit(-1);
	}

	while (1) {
		/* Do some processing ...*/
		clock_gettime(CLOCK_MONOTONIC, &ta);
		printf("Activation %4d\n",i++);
		printf("ta (s:ns) = %6lu:%6lu \n\r ",ta.tv_sec,ta.tv_nsec);
		
		/* Wait for next period */
		sched_yield();
	}

 }

 int main (int argc, char **argv)
 {
	pthread_t thread;

	printf("Executing EDF+CBS thread ...\n");

	pthread_create(&thread, NULL, edfTask, NULL);

	pthread_exit(NULL);

	printf("Done\n");
	return 0;
 }
