/* $Revision: 1.3 $ */
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
#include <string.h>

extern "C" {
    #include "buffer.h"
}
#include "../intl/lang.h"
#include "main.h"
#include "../gui-gtk/gui.h"

int buffer_alloc(chip_desc *chip)
{	
    if(!chip){
	printf("{buffer.c} buffer_alloc() --> Error: chip == NULL\n");
	return -4;
    }
    if(chip->buffer){
	printf("{buffer.c} buffer_alloc() --> Error: chip->buffer != NULL\n");
	return -3;
    }
    if(!chip->dev_size){
	printf("{buffer.c} buffer_alloc() --> Error: chip->dev_size == 0\n");
	return -2;
    }
    
    chip->buffer = (char *)malloc(sizeof(char) * chip->dev_size);

    if(!chip->buffer){
	printf("{buffer.c} buffer_alloc() --> Error: Out of memory\n");
	return -1;
    }
    
    memset(chip->buffer, 0, sizeof(char) * chip->dev_size);
    return 0;
}

void buffer_free(chip_desc *chip)
{
    if(!chip){
	printf("{buffer.c} buffer_alloc() --> Error: chip == NULL\n");
	return;
    }
    if(!chip->buffer) return;
    free(chip->buffer);
    chip->buffer = NULL;
}

void buffer_clear(geepro *gep)
{
    if(!gep->chp) return;
    memset( gep->chp->buffer, 0, gep->chp->dev_size );
}

long buffer_checksum(geepro *gep)
{
    unsigned int i;
    long tmp = 0;
    
    for( i = 0; i < gep->chp->dev_size; i++ ) tmp += gep->chp->buffer[i];
    return tmp;
}


char buffer_write(geepro *gep, unsigned int addr, unsigned char byte)
{
    if(addr >= gep->chp->dev_size) return -1;
    gep->chp->buffer[addr] = byte;
    return 0;
}

int buffer_read(geepro *gep, unsigned int addr)
{
    if(addr >= gep->chp->dev_size) return -1;
    return gep->chp->buffer[addr];
}

char *buffer_get_buffer_ptr(geepro *gep)
{
    return gep->chp->buffer;
}



