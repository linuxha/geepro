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

#ifndef __TIMER_H
#define __TIMER_H

#include <sched.h>

#define PRINT_VERSION

#define MY_PID	     0
#define MAX_PRIORITY 0xffffafff
#define MIN_PRIORITY 0xffffbfff     

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int pid;
    int policy;
    int old_priority;
    int old_policy;
    struct sched_param schd_param;
    int high_priority;
    int low_priority;
    char new_priority;
} schd;

extern int timer_init_priority(int pid, int policy, schd *);
extern int timer_new_priority(int priority, schd *);
extern int timer_restore_priority(schd *);

extern void timer_us_delay(int ticks);
extern void timer_latency(char sw, int uid); /* zmienia ustawienie planisty, wymaga root */

/* 
    jesli czas opóźninia przekracza 300ms, to pomiędzy kolejnymi 300ms wywoływana jest funkcja callback 
    ze wskaźnikiem ptr jako parametrem
*/
extern void timer_abort_func(void (*callback)(void*), void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H */
