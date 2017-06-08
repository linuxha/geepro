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

#ifndef __files_h__
#define __files_h__
#include "geepro.h"

#ifdef __cplusplus
extern "C" {
#endif

// false on fail
typedef boolean (*file_ls_callback)(const char *fname, const char *error, void *arg);

extern const char *file_load(geepro *gep, const char *fname, long file_offset, long buffer_offset, long bytes_count);
extern const char *file_save(geepro *gep, const char *fname);
extern const char *file_get_time(geepro *gep, long long *time, const char *fname);
extern long file_length(const char *fname); // returns -1 on fail

// lists directory content pointed by 'path' and filtered by 'regex'
// on each match entry callback is called, 'arg' is the custom argument for callback
// 'error' have to point array string of 256 size.  It returns error message strings on error. It should be cleared by 0.
// return false on fail
extern boolean file_ls(const char *path, const char *regex, char *error, file_ls_callback, void *arg);

#ifdef __cplusplus
}
#endif

#endif
