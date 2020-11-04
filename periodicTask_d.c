/**********************************************************
 * Paulo Pedreiras, Oct/2020
 * DETI/UA/IT - Real-Time Systems course
 *
 * Periodic activity with the use of timers
 *
 *    
 *
 ***********************************************************/
#define _GNU_SOURCE /* Required by sched_setaffinity */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <sched.h> //sched_setscheduler
#include <pthread.h>
#include <errno.h>
#include <signal.h> // Timers
#include <stdint.h>
#include <unistd.h>


/* ***********************************************
* App specific defines
* ***********************************************/
#define NS_IN_SEC 1000000000L

#define PERIOD_NS (100*1000*1000) 	// Period (ns component)
#define PERIOD_S (0)			// Period (seconds component)

// States of the thread (relative to number of activations)
#define BOOT 0
#define FIRST 1
#define OTHERS 2

/* ***********************************************
* Prototypes
* ***********************************************/
struct  timespec TsAdd(struct  timespec  ts1, struct  timespec  ts2);
struct  timespec TsSub(struct  timespec  ts1, struct  timespec  ts2);

/* ***********************************************
* Global variables
* ***********************************************/
uint64_t min_iat, max_iat;
cpu_set_t cpuset;


/* *************************
* Thread_1 code 
* **************************/

void * Thread_1_code(void *arg)
{
    	/* Timespec variables to manage time */
	struct timespec ts,tr,ta, ta_ant, tp;
	
	/* Other variables */
	static int state = BOOT;
	static int update; 
	
	/* Set absolute activation time of first instance */
	clock_gettime(CLOCK_MONOTONIC, &ts);
	tp.tv_nsec = PERIOD_NS;
	tp.tv_sec = PERIOD_S;	

	ts = TsAdd(ts,tp);	
	
	/* Periodic jobs ...*/ 
	while(1) {

		/* Wait until next cycle */
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,&ts,&tr);
		clock_gettime(CLOCK_MONOTONIC, &ta);		
		ts = TsAdd(ts,tp);			

		/* Compute latency and jitter */
		switch (state) {
		  case BOOT:
		    ta_ant = ta;
		    state = FIRST;
		    break;
		    
		  case FIRST:
		    tr=TsSub(ta,ta_ant);
		    min_iat = tr.tv_nsec;
		    max_iat = tr.tv_nsec;
		    ta_ant = ta;
		    update = 1;
		    state = OTHERS;
		    break;
		    
		  default:
		    tr=TsSub(ta,ta_ant);
		    if(tr.tv_nsec < min_iat) {
		      min_iat = tr.tv_nsec;
		      update = 1;
		    }
		    if(tr.tv_nsec > max_iat) {
		      max_iat = tr.tv_nsec;
		      update = 0;
		    }
		    ta_ant = ta;
		}
		  
  		/* Print maximum/minimum inter-arrival time */
		if(update) {
		  printf("Task %s inter-arrival time: min: %lu / max: %lu\n\r",(char *) arg, min_iat, max_iat);
		  update = 0;
		}
		/* Do processing */
		/* Consume time  */
        int milli_seconds = 20;
        clock_t start_time = clock();
        while (clock() < start_time + milli_seconds);
		// printf("Wake at secs:%d, nsec:%d\n\r", ta.tv_sec, ta.tv_nsec);
		// printf("Next activation secs:%d, nsec:%d\n\r", ts.tv_sec, ts.tv_nsec);
		
	}  
  
    return NULL;
}


/* *************************
* main()
* **************************/

int main(int argc, char *argv[])
{

	//  Force all processes to execute on the same CPU
	CPU_ZERO(&cpuset);
	CPU_SET(0,&cpuset);
	if(sched_setaffinity(0, sizeof(cpuset), &cpuset)) {
		printf("\n Lock of process to CPU0 failed!!!");
		return(1);
	}

	int err;
	pthread_t threadid;
	char procname[40]; 
	

	/* Process input args */
	if(argc != 2) {
	  printf("Usage: %s PROCNAME, where PROCNAME is a string\n\r", argv[0]);
	  return -1; 
	}
	
	strcpy(procname, argv[1]);
	
	//Defines a fixed real time priority
	struct sched_param parm;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr,
	PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	parm.sched_priority = 42;
	pthread_attr_setschedparam(&attr, &parm);

	/* Create periodic thread/task */
	err=pthread_create(&threadid, &attr, Thread_1_code, &procname);
 	if(err != 0) {
		printf("\n\r Error creating Thread [%s]", strerror(err));
		return -1;
	}
	else 
		while(1); // Ok. Thread shall run
	
	/* Last thing that main() should do */
    	pthread_exit(NULL);
		
	return 0;
}




/* ***********************************************
* Auxiliary functions 
* ************************************************/
struct  timespec  TsAdd(struct  timespec  ts1, struct  timespec  ts2){
	
    struct  timespec  tr;
	
	// Add the two timespec variables
    	tr.tv_sec = ts1.tv_sec + ts2.tv_sec ;
    	tr.tv_nsec = ts1.tv_nsec + ts2.tv_nsec ;
	// Check for nsec overflow	
	if (tr.tv_nsec >= NS_IN_SEC) {
        	tr.tv_sec++ ;
		tr.tv_nsec = tr.tv_nsec - NS_IN_SEC ;
    	}

    return (tr) ;
}

struct  timespec  TsSub (struct  timespec  ts1, struct  timespec  ts2) {
  struct  timespec  tr;

  // Subtract second arg from first one 
  if ((ts1.tv_sec < ts2.tv_sec) || ((ts1.tv_sec == ts2.tv_sec) && (ts1.tv_nsec <= ts2.tv_nsec))) {
    // Result would be negative. Return 0
    tr.tv_sec = tr.tv_nsec = 0 ;  
  } else {						
	// If T1 > T2, proceed 
        tr.tv_sec = ts1.tv_sec - ts2.tv_sec ;
        if (ts1.tv_nsec < ts2.tv_nsec) {
            tr.tv_nsec = ts1.tv_nsec + NS_IN_SEC - ts2.tv_nsec ;
            tr.tv_sec-- ;				
        } else {
            tr.tv_nsec = ts1.tv_nsec - ts2.tv_nsec ;
        }
    }

    return (tr) ;

}
