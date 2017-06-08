/* $Revision: 1.1.1.1 $ */
/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2006 Krzysztof Komarnicki
 * Email: krzkomar@wp.pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See the file COPYING. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "timer.h"

#define VERSION "lib timer ver 0.0.1\n"
#define CHECK(sts,msg) if((sts) == -1) { perror(msg); return -1; }

static int __sched_chg__=0;
static schd sched;
static void timer_idle(void *ptr){};
static void *__timer_call_ptr__=NULL;
static void (*__timer_call__)(void *) = timer_idle;

int set_priority(int policy, unsigned int priority, schd *sch)
{
    sch->schd_param.sched_priority = (priority == MAX_PRIORITY) ? sch->high_priority :
				(priority == MIN_PRIORITY) ? sch->low_priority : priority;
    CHECK(sched_setscheduler(sch->pid, policy, &sch->schd_param), "set sched_setscheduler()");
    return 0;
}

int new_priority(int priority, schd *sch)
{
    sch->new_priority = 1;
    return set_priority(sch->policy, priority, sch);
}

int get_maxmin_priority(schd *sch)
{
    CHECK((sch->high_priority = sched_get_priority_max(sch->policy)), "sched_getpriority_max()");
    CHECK((sch->low_priority  = sched_get_priority_min(sch->policy)), "sched_getpriority_min()");
    return (sch->high_priority == -1) ? sch->high_priority : sch->low_priority;
}

int store_actual_policy(schd *sch)
{
    sch->old_policy = sched_getscheduler(sch->pid);
    CHECK(sch->old_policy, "sched_getscheduler()");
    return sch->old_policy;
}

int store_actual_priority(schd *sch)
{
    CHECK(sched_getparam(0, &sch->schd_param), "sched_getparam()");
    sch->old_priority = sch->schd_param.sched_priority;
    return sch->old_priority;
}

int init_priority(int pid, int policy, schd *sch)
{
    int tmp;
    static char version = 1;
    sch->pid = pid;
    sch->policy = policy;
    sch->new_priority = 0;
    if(get_maxmin_priority(sch) == -1) return -1;
    if((tmp = store_actual_policy(sch)) != -1)
	tmp = store_actual_priority(sch);
#ifdef PRINT_VERSION
    if(version){
	printf(VERSION);
	version = 0;
    }
#endif
    return tmp;
}

int restore_priority(schd *sch)
{
    if(!sch->new_priority) return 0;
    sch->new_priority = 0;
    return set_priority(sch->old_policy, sch->old_priority, sch);
}

static long timer_get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

static void timer_u_sleep(int delay)
{
    long stop = timer_get_time() + delay;
    
    while(timer_get_time() < stop);
}

static int timer_usleep(long delay)
{
    struct timespec ts;
    ts.tv_sec=0;
    ts.tv_nsec = 1000 * delay;
    return nanosleep(&ts, NULL);
}

void timer_us_delay(int delay)
{

    int i, r;
    
/* 
  jesli nie przestawiony scheduler, a opóźnienie krótsze niż 50ms
*/
    if(!__sched_chg__ && delay < 50000){
	timer_u_sleep(delay);
	return;
    }

/* jesli opóźnienie większe od 300ms to musi byc mozliwość przerwania opóźnienia */
    if(delay > 300000){
	i = delay / 300000;
	r = delay - i * 300000; /* reszta czasu do odczekania */
	
	for(;i; i--){
		timer_usleep(300000);
		__timer_call__(__timer_call_ptr__);
	}
	timer_usleep(r); /* odczekaj resztę czasu */
	return;
    }

    if(timer_usleep(delay)){
	printf("{timer.c} Bład wykonania nanosleep. errno=%i\n", errno);
	return;    
    }
}

void timer_clean()
{
    if(__sched_chg__) restore_priority(&sched);
}

void timer_latency(char sw, int uid)
{
    static char flag = 0; 
    
    if(uid) return;
    
    if(!flag){
	init_priority(MY_PID, SCHED_FIFO,&sched);
	flag = 1;
	atexit(timer_clean);
    }

    if(sw){
	new_priority(MAX_PRIORITY, &sched);
	__sched_chg__ = 1;
    }else{
	restore_priority(&sched);
	__sched_chg__ = 0;
    }
}

void timer_abort_func(void (*callback)(void*), void *ptr)
{
__timer_call_ptr__ = ptr;
__timer_call__ = callback;

}
