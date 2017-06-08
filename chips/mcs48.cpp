/* $Revision: 1.5 $ */
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

/* MEM SIZE */
#define SIZE_8041	1024
#define SIZE_8042	2048
#define SIZE_8048	1024
#define SIZE_8049	2048
#define SIZE_8050	4096

/* MASK for T0 */
#define MCS48_READ_MODE		0x10
#define MCS48_WRITE_MODE	0x10

void set_address_mcs48(int addr, char mode)
{
    hw_delay(100);
    set_address( ((addr >> 8) & 0x0f) | mode);
    oe(0, 1500);  // ustawienie RESET = 0
    set_data(addr & 0xff);
    oe(1, 1500);  // zapamietanie dolnej polowki adresu
}

void read_mcs48(int size)
{
    int addr;

    ce(0, 1);
    oe(1, 1);
    hw_sw_vcc(1);
    hw_delay(3000);    
    oe(0, 3000);  // RESET = 0, VPP = VCC
    hw_sw_vpp(1); // VPP = Uvpp
    oe(1, 3000);  // RESET = 1, VPP = VCC    
    progress_loop(addr, size, "Reading"){
	set_address_mcs48( addr, MCS48_READ_MODE );
	copy_data_to_buffer( addr );
    }
    set_address(0);
    set_data(0);
    finish_action();
}

void test_blank_mcs48(int size)
{
    int addr;
    char data = 0, text[256];

    ce(0, 1);
    oe(1, 1);
    hw_sw_vcc(1);
    hw_delay(3000);    
    oe(0, 3000);  // RESET = 0, VPP = VCC
    hw_sw_vpp(1); // VPP = Uvpp
    oe(1, 3000);  // RESET = 1, VPP = VCC    
    progress_loop(addr, size, "Test blank"){
	set_address_mcs48( addr, MCS48_READ_MODE );
	data = hw_get_data();
	break_if( data != 0xff );
    }
    set_address(0);
    set_data(0);
    finish_action();
    if(data != 0xff){
	sprintf(text, "[IF][TEXT] Memory is not clear\nByte 0x%x at address 0x%x [/TEXT][BR]OK", data & 0xff, addr & 0xffff);
	show_message(0, text, NULL, NULL);    
    }
}

void verify_mcs48(int size)
{
    int addr;
    unsigned char rdata = 0, wdata = 0;
    char text[256];

    ce(0, 1);
    oe(1, 1);
    hw_sw_vcc(1);
    hw_delay(3000);    
    oe(0, 3000);  // RESET = 0, VPP = VCC
    hw_sw_vpp(1); // VPP = Uvpp
    oe(1, 3000);  // RESET = 1, VPP = VCC    
    progress_loop(addr, size, "Verification"){
	set_address_mcs48( addr, MCS48_READ_MODE );
	rdata = hw_get_data();
	wdata = get_buffer( addr );
	break_if( rdata != wdata );
    }
    set_address(0);
    set_data(0);
    finish_action();
    if(rdata != wdata){
	sprintf(text, 
	    "[IF][TEXT] Buffer and chip inconsistent\n"
	    "0x%x(chip) != 0x%x(buffer) at address 0x%x [/TEXT][BR]OK", 
	    rdata & 0xff, wdata & 0xff, addr & 0xffff
	);
	show_message(0, text, NULL, NULL);    
    }
}

void prog_pulse_mcs48()
{
    ce(0, 50000);
    ce(1, 100);
}

void prog_mcs48(int size)
{
    unsigned int addr;
    unsigned char rdata = 0, wdata = 0;
    char text[256];

    ce(0, 1);
    oe(1, 1);
    hw_sw_vcc(1);
    hw_delay(3000);    
    oe(0, 3000);  // RESET = 0, VPP = VCC
    hw_sw_vpp(1); // VPP = Uvpp
    oe(1, 3000);  // RESET = 1, VPP = VCC    
    progress_loop(addr, size, "Programm"){
	set_address_mcs48( addr, MCS48_WRITE_MODE );
	wdata = get_buffer( addr );
	set_data( wdata );
	prog_pulse_mcs48();	
	set_address_mcs48( addr, MCS48_READ_MODE );
	rdata = hw_get_data();
	break_if( rdata != wdata );
    }
    set_address(0);
    set_data(0);
    finish_action();
    if(rdata != wdata){
	sprintf(text, 
	    "[IF][TEXT] Program error\n"
	    "0x%x(chip) != 0x%x(buffer) at address 0x%x [/TEXT][BR]OK", 
	    rdata & 0xff, wdata & 0xff, addr & 0xffff
	);
	show_message(0, text, NULL, NULL);    
    }
}

/********************************************************
  ROM
*/
REGISTER_FUNCTION( read, 8048, mcs48, SIZE_8048 );
REGISTER_FUNCTION( verify, 8048, mcs48, SIZE_8048 );
REGISTER_FUNCTION( read, 8049, mcs48, SIZE_8049 );
REGISTER_FUNCTION( verify, 8049, mcs48, SIZE_8049 );
REGISTER_FUNCTION( read, 8050, mcs48, SIZE_8050 );
REGISTER_FUNCTION( verify, 8050, mcs48, SIZE_8050 );
/********************************************************
 EPROM
*/
//REGISTER_FUNCTION( read, 8041, mcs48, SIZE_8041 );
//REGISTER_FUNCTION( verify, 8041, mcs48, SIZE_8041 );
//REGISTER_FUNCTION( prog, 8741, mcs48, SIZE_8041 );
//REGISTER_FUNCTION( test_blank, 8041, mcs48, SIZE_8041 );
//REGISTER_FUNCTION( read, 8042, mcs48, SIZE_8042 );
//REGISTER_FUNCTION( verify, 8042, mcs48, SIZE_8042 );
//REGISTER_FUNCTION( prog, 8742, mcs48, SIZE_8042 );
//REGISTER_FUNCTION( test_blank, 8042, mcs48, SIZE_8042 );
REGISTER_FUNCTION( prog, 8748, mcs48, SIZE_8048 );
REGISTER_FUNCTION( test_blank, 8048, mcs48, SIZE_8048 );
REGISTER_FUNCTION( prog, 8749, mcs48, SIZE_8049 );
REGISTER_FUNCTION( test_blank, 8049, mcs48, SIZE_8049 );

/*************************************************************************/
REGISTER_MODULE_BEGIN( MCS-48 )
// ROM Version
    register_chip_begin("/uk/MCS-48/ROM", "i8048", "MCS-48", SIZE_8048);
	add_action(MODULE_READ_ACTION, read_8048);
	add_action(MODULE_VERIFY_ACTION, verify_8048);
    register_chip_end;
    register_chip_begin("/uk/MCS-48/ROM", "i8049", "MCS-48", SIZE_8049);
	add_action(MODULE_READ_ACTION, read_8049);
	add_action(MODULE_VERIFY_ACTION, verify_8049);
    register_chip_end;
    register_chip_begin("/uk/MCS-48/ROM", "i8050", "MCS-48", SIZE_8050);
	add_action(MODULE_READ_ACTION, read_8050);
	add_action(MODULE_VERIFY_ACTION, verify_8050);
    register_chip_end;
// EPROM/OTP Version
//    register_chip_begin("/uk/MCS-48/EPROM", "i8741", "MCS-48", SIZE_8041);
//	add_action(MODULE_READ_ACTION, read_8041);
//	add_action(MODULE_VERIFY_ACTION, verify_8041);
//	add_action(MODULE_PROG_ACTION, prog_8741);
//	add_action(MODULE_TEST_BLANK_ACTION, test_blank_8041);
//    register_chip_end;
//    register_chip_begin("/uk/MCS-48/EPROM", "i8742", "MCS-48", SIZE_8042);
//	add_action(MODULE_READ_ACTION, read_8042);
//	add_action(MODULE_VERIFY_ACTION, verify_8042);
//	add_action(MODULE_PROG_ACTION, prog_8742);
//	add_action(MODULE_TEST_BLANK_ACTION, test_blank_8042);
//    register_chip_end;
    register_chip_begin("/uk/MCS-48/EPROM", "i8748", "MCS-48", SIZE_8048);
	add_action(MODULE_READ_ACTION, read_8048);
	add_action(MODULE_VERIFY_ACTION, verify_8048);
	add_action(MODULE_PROG_ACTION, prog_8748);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_8048);
    register_chip_end;
    register_chip_begin("/uk/MCS-48/EPROM", "i8749", "MCS-48", SIZE_8049);
	add_action(MODULE_READ_ACTION, read_8049);
	add_action(MODULE_VERIFY_ACTION, verify_8049);
	add_action(MODULE_PROG_ACTION, prog_8749);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_8049);
    register_chip_end;
REGISTER_MODULE_END
