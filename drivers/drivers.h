#ifndef __DRIVERS_H__
#define __DRIVERS_H__
/* $Revision: 1.4 $ */
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
#include "../src/parport.h"
#include "willem.h"
#include "../src/timer.h"
#include "hwdriver.h"
#include "../src/iface.h"

#include "../src/geepro.h"

#include "../gui-gtk/gui_xml.h"

#define DRIVER_NAME(ptr)	(*(char **)ptr)

#define driver_register_begin 	extern "C" int driver_init(void *___ptr___){
#define driver_register_end 	return 0; }
#define register_api(api)	iface_prg_add((iface *)___ptr___, api, 1)


#endif

