/* $Revision: 1.12 $ */
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

#ifndef __modules_h__
#define __modules_h__
#include <stdio.h>
#include <string.h>
#include "../drivers/hwdriver.h"
#include "../src/chip.h"
#include "../src/protocols.h"
#include "../gui-gtk/gui.h"
#include "../src/buffer.h"
#include "../src/geepro.h"


#define ms_delay(ms)	\
    hw_delay((ms)*1000)
    
#define set_data(value)\
    hw_set_data((value) & 0xff)
    
#define set_address(a)	\
    hw_set_addr(a)

#define oe(state, delay)	\
    hw_set_oe(state);\
    hw_delay(delay)
    
#define ce(state, delay)	\
    hw_set_ce(state);\
    hw_delay(delay)

#define we(state, delay)	\
    hw_set_we(state);\
    hw_delay(delay)

#define pgm(state, delay)	\
    hw_set_pgm(state);\
    hw_delay(delay)

#define progress_loop(cn, rounds, title)		\
	for(cn = 0, gui_progress_bar_init(___geep___,title, rounds);\
	    gui_cmp_pls(___geep___,cn, rounds); cn++, gui_progress_bar_set(___geep___,cn, rounds))

#define break_if( cond )	\
	if((___error___ = (cond)) != 0){\
	    progressbar_free();\
	    break;\
	}

#define finish_action()	\
    hw_sw_vpp(0);\
    hw_set_oe(0);\
    hw_set_ce(0);\
    hw_sw_vcc(0);\
    set_address(0);\
    set_data(0);\
    buffer_checksum(___geep___);\
    gui_stat_rfsh(___geep___)

#define start_action(oe_,ce_)   \
    hw_sw_vpp(0);\
    hw_sw_vcc(1);\
    hw_set_oe(oe_);\
    hw_set_ce(ce_);\
    hw_delay(5000)

#define copy_data_to_buffer(addr)	\
    buffer_write(___geep___,addr, hw_get_data())

#define put_buffer( addr, data) buffer_write(___geep___,addr, data)

#define get_buffer(addr) buffer_read(___geep___,addr)

#define copy_data_from_buffer(addr)  set_data( get_buffer(addr) )

#define progressbar_free() gui_progress_bar_free(___geep___)

#define cmp_data_and_buffer_ploop(addr, error) \
	if(hw_get_data() != buffer_read(___geep___,addr)){\
	    error = 0;\
	    gui_progress_bar_free(___geep___);\
	    break;\
	}

#define  show_message(sw,title,msg_1,msg_2)	\
    gui_dialog_box(___geep___, title, sw ? msg_2 : msg_1, "OK", NULL)

#define show_dialog(title, msg)	\
    gui_dialog_box(___geep___, title, msg, "OK", NULL)

#define REGISTER_MODULE_BEGIN(name)	\
    extern "C" int init_module(geepro *gep___)\
    {\
	int __id__=0, __i__ =0 ;\
	___geep___ = gep___;\
	chip_desc __init_struct__; \
	__i__++;\
	printf("Init " #name " module.\n");\
	{

#define REGISTER_MODULE_END	\
	} __id__ = 0;\
	__i__ = 0;\
	return 0;\
    }

#define MODULE_IMPLEMENTATION	\
    static geepro *___geep___ = ((geepro*)0);\
    static int ___error___ = 0;\

#define VOID

#define REG_FUNC_BEGIN(name)	\
    static int name (geepro *gep___)\
    {\
	___geep___ = gep___;\
	___error___ = 0;

#define SET_ERROR	___error___ = -1;	

#define ERROR_VAL	___error___

#define REG_FUNC_END	\
	___geep___ = ((geepro*)0);\
	return ___error___;\
    }

#define to_hex(value, digit)	((value >> (digit * 4)) & 0x0f)

#define ERROR	\
    return -1

#define register_chip_begin(path, name, family, size)	\
    memset(&__init_struct__, 0, sizeof(chip_desc));\
    __init_struct__.chip_path = (char *)path;\
    __init_struct__.chip_name = (char *)name;\
    __init_struct__.chip_family = (char *)family;\
    __init_struct__.chip_id = ++__id__;\
    __init_struct__.dev_size = size

#define register_chip_end	\
    chip_register_chip(___geep___->ifc->plugins, &__init_struct__)

#define add_action(bt_name, callback)	\
    chip_add_action(&__init_struct__, bt_name, (int (*)(void *))(callback))

#define add_autostart(callback)	\
    __init_struct__.autostart = callback

#define MODULE_WRITE_ACTION MODULE_PROG_ACTION
#define MODULE_TEST_BLANK_ACTION MODULE_TEST_ACTION

#define checkbox(fmt)	gui_checkbox(___geep___, fmt)

#define MODULE_READ_ACTION		"geepro-read-action", "Read data from chip"
#define MODULE_READ_FLASH_ACTION	"geepro-read-action", "Read data from chip"
#define MODULE_READ_EEPROM_ACTION 	"geepro-read-eeprom-action", "Read EEPROM data from chip"
#define MODULE_SIGN_ACTION		"geepro-sign-action", "Read signature from chip"
#define MODULE_PROG_ACTION		"geepro-write-action", "Write data to chip"
#define MODULE_PROG_FLASH_ACTION 	"geepro-write-action", "Write data to chip"
#define MODULE_PROG_EEPROM_ACTION	"geepro-write-eeprom-action", "Write EEPROM data to chip"
#define MODULE_ERASE_ACTION		"geepro-erase-action", "Erase memory"
#define MODULE_TEST_ACTION		"geepro-testblank-action", "Test blank memory"
#define MODULE_VERIFY_ACTION		"geepro-verify-action", "Verify chip memory with buffer"
#define MODULE_VERIFY_FLASH_ACTION 	"geepro-verify-action", "Verify chip memory with buffer"
#define MODULE_VERIFY_EEPROM_ACTION 	"geepro-verify-eeprom-action", "Verify EEPROM chip memory with buffer"
#define MODULE_LOCKBIT_ACTION		"geepro-lockbit-action", "Set lock-bits and fuses"

#define SET_Vcc_VOLTAGE( x )	hw_set_vcc( (int)((x) * 100.00) )
#define SET_Vpp_VOLTAGE( x )	hw_set_vpp( (int)((x) * 100.00) )

#define REGISTER_FUNCTION_( registered_func, exec_func, call_parameters... )	\
    REG_FUNC_BEGIN( registered_func )	\
	exec_func(call_parameters);	\
    REG_FUNC_END

#define REGISTER_FUNCTION( action, registered_name, exec_name, call_parameters... )	\
    REGISTER_FUNCTION_( action##_##registered_name, action##_##exec_name, call_parameters)

#define BYTE_POSITION( value, position )	((value) << ( 8 * (position)))
#define MSB( i )	(( (i) >> 8) & 0xff)
#define LSB( i )	((i) & 0xff)

#define KB_SIZE( x )	(1024 * (x))
#define MB_SIZE( x ) 	(1024 * KB_SIZE(x))
#define GB_SIZE( x ) 	(1024 * MB_SIZE(x))
#define TB_SIZE( x ) 	(1024 * GB_SIZE(x))
#define PB_SIZE( x ) 	(1024 * TB_SIZE(x))

#define Bb_SIZE( x )	(( x ) * 8)
#define Kb_SIZE( x )	(KB_SIZE( x ) * 8)
#define Mb_SIZE( x ) 	(MB_SIZE( x ) * 8)
#define Gb_SIZE( x ) 	(GB_SIZE( x ) * 8)
#define Tb_SIZE( x ) 	(TB_SIZE( x ) * 8)
#define Pb_SIZE( x ) 	(PB_SIZE( x ) * 8)

#define loockup_signature( root, vend_id, chip_id, ret_vend_name, ret_chip_name)	\
			loockup_jedec_signature( root, vend_id, chip_id, ret_vend_name, ret_chip_name )

#define BIT_POS( x )	( 1 << (x) )

#define RANGE_2		BIT_POS( 0 )
#define RANGE_4		BIT_POS( 1 )
#define RANGE_8		BIT_POS( 2 )
#define RANGE_16	BIT_POS( 3 )
#define RANGE_32	BIT_POS( 4 )
#define RANGE_64	BIT_POS( 5 )
#define RANGE_128	BIT_POS( 6 )
#define RANGE_256	BIT_POS( 7 )
#define RANGE_512	BIT_POS( 8 )
#define RANGE_1K	BIT_POS( 9 )
#define RANGE_2K	BIT_POS( 10 )
#define RANGE_4K	BIT_POS( 11 )
#define RANGE_8K	BIT_POS( 12 )
#define RANGE_16K	BIT_POS( 13 )
#define RANGE_32K	BIT_POS( 14 )
#define RANGE_64K	BIT_POS( 15 )
#define RANGE_128K	BIT_POS( 16 )
#define RANGE_256K	BIT_POS( 17 )
#define RANGE_512K	BIT_POS( 18 )
#define RANGE_1M	BIT_POS( 19 )
#define RANGE_2M	BIT_POS( 20 )
#define RANGE_4M	BIT_POS( 21 )
#define RANGE_8M	BIT_POS( 22 )
#define RANGE_16M	BIT_POS( 23 )

#endif





