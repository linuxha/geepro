/* $Revision: 1.1.1.1 $ */
/* 
 * Copyright (C) 2007 Krzysztof Komarnicki
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
#include "checksum.h"

CHECKSUM_CRC16_STATIC_TABLE;
CHECKSUM_CRC32_STATIC_TABLE;

int checksum_calculate(
    ChecksumAlgo algo,
    int bfsize, unsigned char *buffer,
    int start, int stop,
    int start_sk1, int stop_sk1,
    int start_sk2, int stop_sk2
)
{
    int i;
    int sum = 0;
    unsigned char chr;

    if((buffer == NULL) || (bfsize < 1)) return 0;

    switch(algo){
        case CHECKSUM_ALG_LRC:   sum = 0; break;
        case CHECKSUM_ALG_CRC16: sum = 0xffff; break;
        case CHECKSUM_ALG_CRC32: sum = 0xffffffff; break;	    
    }
    
    for(i = start; i < stop; i++){
	if(
	    ((i >= start_sk1) && (i <= stop_sk1) && (start_sk1 != stop_sk1)) ||
	    ((i >= start_sk2) && (i <= stop_sk2) && (start_sk2 != stop_sk2))
	) continue;
	
	chr = buffer[i];
	
	switch(algo){
	    case CHECKSUM_ALG_LRC:   sum = CHECKSUM_LRC_UPDATE(chr, sum); break;
	    case CHECKSUM_ALG_CRC16: sum = CHECKSUM_CRC16_UPDATE(chr, sum); break;
	    case CHECKSUM_ALG_CRC32: sum = CHECKSUM_CRC32_UPDATE(chr, sum); break;
	}
    }

    switch(algo){
        case CHECKSUM_ALG_LRC:   sum = CHECKSUM_LRC_FINALIZE(sum); break;
        case CHECKSUM_ALG_CRC16: sum = CHECKSUM_CRC16_FINALIZE(sum); break;
        case CHECKSUM_ALG_CRC32: sum = CHECKSUM_CRC32_FINALIZE(sum); break;
    }

    return sum;
}
