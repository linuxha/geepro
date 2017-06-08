/* $Revision: 1.4 $ */
/* parport - user space wrapper for LPT port using ppdev v 0.0.2
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
#include <sys/io.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

#include "parport.h"

#define PARPORT_VERSION	"lib parport version 0.0.3\n"

#ifndef __PARPORT_CPP_CLASS__
void parport_message(int, void *, const char *, ...);

/* zmienne globalne */
unsigned char parport_mirror[3];
static int parport_ppdev_fd=0;
static char allow=0;
message_type parport_message_handler = parport_message;
void *parport_msgh_ptr = NULL;
static int parport_init_lvl=0;

#endif

/* wirtualna funkcja obsługi błędu */
void PARPORT(message)(int lvl, void *ptr, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("%s(%i): ", lvl > 3 ? (lvl > 6 ? "ERROR":"WARNING"): "MSG", lvl);
    vprintf(fmt, ap);
    va_end(ap);
}

/***************************************************************************************************************************/
/* niskopoziomowe IO */

/* do zmiany */

int PARPORT(cleanup)(void)
{
    int err = 0;
    if(PARPORT_M(init_lvl) == 0) return 0;
    PARPORT_M(message)(1, PARPORT_EM, "Cleanup parport device.\n");
    if(PARPORT_M(init_lvl) > 1)
	if(ioctl(PARPORT_M(ppdev_fd), PPRELEASE) == -1){
	    PARPORT_M(message)(7, PARPORT_EM, "ioctl(%d, PPRELEASE): %s\n", PARPORT_M(ppdev_fd), strerror(errno));
	    err = PP_ERROR;
	}
    if(PARPORT_M(init_lvl))
	if(close(PARPORT_M(ppdev_fd)) == -1){
	    PARPORT_M(message)(7, PARPORT_EM, "close(%d): %s\n", PARPORT_M(ppdev_fd), strerror(errno));
	    err = PP_ERROR;
	}
    if(!err) PARPORT_M(init_lvl) = 0;
    return err;
}

int PARPORT(init)(const char *dev_path, int dev_flags)
{
    static char first_run=1;
    allow = 1;
    if(first_run){
	PARPORT_M(message)(0, PARPORT_EM, PARPORT_VERSION);
	first_run=0;
    } else{
	if(PARPORT_M(cleanup)() == PP_ERROR) return PP_ERROR;
    }
    PARPORT_M(message)(1, PARPORT_EM, "Opening device %s\n", dev_path);
    if((PARPORT_M(ppdev_fd) = open(dev_path, O_RDWR | dev_flags)) == -1 ){
	PARPORT_M(message)(7, PARPORT_EM, "open(\"%s\", O_RDWR): %s\n", dev_path, strerror(errno));    
	return PP_ERROR;
    }
    PARPORT_M(init_lvl) = 1;
    PARPORT_M(message)(1, PARPORT_EM, "Device %s opened with handler=%d\n", dev_path, PARPORT_M(ppdev_fd));
    if(ioctl(PARPORT_M(ppdev_fd), PPCLAIM) == -1){
	PARPORT_M(message)(7, PARPORT_EM, "ioctl(%d, PPCLAIM): %s\n", PARPORT_M(ppdev_fd), strerror(errno));
	PARPORT_M(cleanup)();
	return PP_ERROR;
    }
    PARPORT_M(init_lvl) = 2;
    PARPORT_M(mirror)[0] = PARPORT_M(mirror)[1] = PARPORT_M(mirror)[2] = 0;      

    PARPORT_M(reset)();
    allow = 0;
    return 0;
}

int PARPORT(w_data)(unsigned char data)
{
    if(allow) return 0;
    if(ioctl(PARPORT_M(ppdev_fd), PPWDATA, &data)){
	PARPORT_M(message)(7, PARPORT_EM, "ioctl(%d, PPWDATA, %d): %s\n", PARPORT_M(ppdev_fd), data, strerror(errno));	
	PARPORT_M(cleanup)();
	return PP_ERROR;
    }    
    return 0;
}

int PARPORT(w_ctrl)(unsigned char data)
{
    if(allow) return 0;
    data ^= 0x0b; /* negacja bitów sprzetowo negowanych */
    if(ioctl(PARPORT_M(ppdev_fd), PPWCONTROL, &data)){
	PARPORT_M(message)(7, PARPORT_EM, "ioctl(%d, PPWCONTROL, %d): %s\n", PARPORT_M(ppdev_fd), data, strerror(errno));	
	PARPORT_M(cleanup)();
	return PP_ERROR;
    }    
    return 0;
}

int PARPORT(r_stat)(void)
{ 
    if(allow) return 0;
    unsigned char data;
    data = 0;
    if(ioctl(PARPORT_M(ppdev_fd), PPRSTATUS, &data)){
	PARPORT_M(message)(7, PARPORT_EM, "ioctl(%d, PPRSTATUS): %s\n", PARPORT_M(ppdev_fd), strerror(errno));
	PARPORT_M(cleanup)();
	return PP_ERROR;
    }    
    data ^= 0x80;
    return (int)data;
}

/***************************************************************************************************************************/
/* Operacje IO */

int PARPORT(set)(unsigned char port_idx, unsigned char data)
{
    if(allow) return 0;
    if(port_idx > 2) return PP_ERROR;
    PARPORT_M(mirror)[port_idx] = data;
    if(port_idx == PA)
	if(PARPORT_M(w_data)(data) == PP_ERROR) return PP_ERROR;
    if(port_idx == PC)
	if(PARPORT_M(w_ctrl)(data) == PP_ERROR) return PP_ERROR;
    return 0;
}

int PARPORT(get)(unsigned char port_idx)
{
    if(allow) return 0;
    if(port_idx > 2) return 0;
    if(port_idx == PB) PARPORT_M(mirror)[port_idx] = PARPORT_M(r_stat)();
    return PARPORT_M(mirror)[port_idx];
}

int PARPORT(reset)(void)
{
    if(allow) return 0;
    if(PARPORT_M(set)(PA, 0) == PP_ERROR) return PP_ERROR;
    if(PARPORT_M(set)(PB, 0) == PP_ERROR) return PP_ERROR;
    if(PARPORT_M(set)(PC, 0) == PP_ERROR) return PP_ERROR;
    return 0;
}

/***************************************************************************************************************************/
/* operacje bitowe na portach */

int PARPORT(set_bit)(unsigned char idx, unsigned char mask)
{
    return PARPORT_M(set)(idx, PARPORT_M(mirror)[idx] | mask);
}

int PARPORT(clr_bit)(unsigned char idx, unsigned char mask)
{
    return PARPORT_M(set)(idx, PARPORT_M(mirror)[idx] & ~mask);
}

int PARPORT(get_bit)(unsigned char idx, unsigned char mask)
{
    register int x = PARPORT_M(get)(idx) & mask;
    if(x == -1) return PP_ERROR;
    return x ? 1:0;
}

#ifdef __PARPORT_CPP_CLASS__
parport::parport(const char *dev_path, int flags)
{
    if(init(dev_path, flags) == PP_ERROR) throw "parport.c";
}

parport::~parport()
{
    if(cleanup() == PP_ERROR) throw "parport.c";
}

#endif

