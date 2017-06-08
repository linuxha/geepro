/* $Revision: 1.2 $ */
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

#ifndef __main_h__
#define __main_h__
#include "chip.h"
#include "geepro.h"
#include "storings.h"

#ifdef __cplusplus
extern "C" {
#endif

extern store_str store;
extern void error(void);
extern char test_hw(void *, geepro*);
extern int  test_uid(geepro *gep);
extern void rem_all_modl( modules* );

#ifdef __cplusplus
}
#endif

#endif
