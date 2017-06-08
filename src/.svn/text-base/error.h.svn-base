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


#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum{
    E_MSG,	// message
    E_WRN,	// warning
    E_ERR,	// normal error
    E_CRT,	// critical error
} t_error_class;

enum{
    E_OK = 0,
    E_MEM,
    E_NULL,
    E_PAR,
};

enum
{
    _F_ = 0,
    _T_ = 1,
};

typedef char t_error;
typedef char t_bool;

#define E_T_MALLOC	"malloc() == NULL"

#define ERROR(err_class, format, x...) printf("EC[%i]{%i}:%s:%s()->"format"\n", err_class, __LINE__, __FILE__,__FUNCTION__, ##x)

#endif // __ERROR_H__

