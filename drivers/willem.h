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

#ifndef __willem_h__
#define __willem_h__
#include "hwdriver.h"
/*

#define	TA_01	10
#define	TA_02	10
#define	TA_03	20

#define TD_01	10
#define TD_02	10
#define TD_03	10
#define TD_04	10
#define TD_05	10
*/

/* czasy opóźnień*/

#define	TA_01	1
#define	TA_02	1
#define	TA_03	1

#define TD_01	1
#define TD_02	1
#define TD_03	1
#define TD_04	1
#define TD_05	1


#define DEBUG printf

extern "C" {
    extern int willem_hardware_driver(en_hw_api func, int val, void *ptr);
};
#endif



