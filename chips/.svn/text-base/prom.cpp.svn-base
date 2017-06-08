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

#include "modules.h"

MODULE_IMPLEMENTATION

#define ADDR_MASK_474	0x600
#define SIZE_PROM_474	0x200

REG_FUNC_BEGIN(read_474)
    int addr;

    hw_sw_vpp(0);
    hw_sw_vcc(1);
    hw_set_addr(0);
    hw_set_data(0);
    hw_delay(1000);
    progress_loop(addr, SIZE_PROM_474, "Reading..."){
	hw_set_oe(1);
	hw_set_ce(1);
	hw_set_addr(addr | ADDR_MASK_474);	
	hw_delay(200);
	buffer_write(___geep___,addr,hw_get_data());
	hw_delay(200);
    }
    finish_action();
REG_FUNC_END

REG_FUNC_BEGIN(verify_474)
    int addr;

    hw_sw_vpp(0);
    hw_sw_vcc(1);
    hw_set_addr(0);
    hw_set_data(0);
    hw_delay(1000);
    progress_loop(addr, SIZE_PROM_474, "Veryfying..."){
	hw_set_oe(1);
	hw_set_ce(1);
	hw_set_addr(addr | ADDR_MASK_474);	
	hw_delay(200);
	if(buffer_read(___geep___,addr) != hw_get_data() ){
// ?????
	}
	hw_delay(200);
    }
    finish_action();
REG_FUNC_END

REGISTER_MODULE_BEGIN( PROM )

    register_chip_begin("/PROM","74S474", "74S47x", SIZE_PROM_474);
	add_action(MODULE_READ_ACTION, read_474);
	add_action(MODULE_VERIFY_ACTION, verify_474);
    register_chip_end;

    register_chip_begin("/PROM","74S475", "74S47x", SIZE_PROM_474);
	add_action(MODULE_READ_ACTION, read_474);
	add_action(MODULE_VERIFY_ACTION, verify_474);
    register_chip_end;

    register_chip_begin("/PROM","74S475", "74S47x", SIZE_PROM_474);
	add_action(MODULE_READ_ACTION, read_474);
	add_action(MODULE_VERIFY_ACTION, verify_474);
    register_chip_end;

    register_chip_begin("/PROM","KR556RT5", "74S47x", SIZE_PROM_474);
	add_action(MODULE_READ_ACTION, read_474);
	add_action(MODULE_VERIFY_ACTION, verify_474);
    register_chip_end;

REGISTER_MODULE_END
