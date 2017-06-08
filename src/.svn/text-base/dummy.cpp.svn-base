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

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "dummy.h"
#include "../drivers/hwdriver.h"
#include "iface.h"
}

int dummy_hardware_driver(en_hw_api funct, int val, void *ptr)
{
    printf("Dummy module\n");
    switch(funct){
	/* ogólne */
	case HW_NAME:	  *(char **)ptr = (char *)"Dummy prog";
	case HW_IFACE:	  return IFACE_USB;
	case HW_GINIT:    return 0;
	case HW_TEST:	  return 1; /* zawsze obecny */
	case HW_RESET:    return 0;
	case HW_OPEN:     return 0;
	case HW_CLOSE:    return 0;
	case HW_SET_VCC:  return 0;
	case HW_SET_VPP:  return 0;
	case HW_SW_VCC:	  return 0;
	case HW_SW_VPP:	  return 0;

	case HW_SW_DPSW:  return 0;
	/* funkcje gniazda eprom */
	case HW_SET_DATA: return 0;
	case HW_SET_ADDR: return 0;
	case HW_INC_ADDR: return 0;
	case HW_DEC_ADDR: return 0;
	case HW_RST_ADDR: return 0;
	case HW_GET_DATA: return 0;
	case HW_SET_WE:   return 0;
	case HW_SET_OE:   return 0;
	case HW_SET_CE:   return 0;
	/* Serial SPI jak 93Cxx, 25Cxx*/
	case HW_SET_CS:	  return 0;
	case HW_SET_CLK:  return 0;
	case HW_SET_DI:	  return 0;
	case HW_GET_DO:	  return 0;
	/* Serial I2C jak 24Cxx, PIC */
	case HW_SET_HOLD: return 0;
	case HW_SET_SCL:  return 0;
	case HW_SET_SDA:  return 0;
	case HW_GET_SDA:  return 0;
	case HW_DELAY:	  return 0;
	case HW_LATENCY:  return 0;
	default:  	  return HW_ERROR;
    }
    return -2;
}

