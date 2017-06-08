/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2011 Krzysztof Komarnicki
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

#ifndef __STORINGS_H__
#define __STORINGS_H__
#include <stdio.h>

typedef struct
{
    FILE *file;
    char *sval;
    int   ival;
    float fval;
} store_str;

extern int store_constr(store_str *, const char *path, const char *file);
extern void store_destr(store_str *);
extern int store_get(store_str *, const char *key, char **value); // alloc memory for value, have to free()
extern int store_set(store_str *, const char *key, const char *string_to_store);

#endif

