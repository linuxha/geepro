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

// memory sizes
#define SIZE_93C06	12
#define SIZE_93C13	32
#define SIZE_93C14	128
#define SIZE_93C46	128
#define SIZE_93C56	256
#define SIZE_93C66	512
#define SIZE_93C76	1024
#define SIZE_93C86	2048

// memory address field length
#define ADDR_SIZE_93C06	7
#define ADDR_SIZE_93C13	7
#define ADDR_SIZE_93C14	7
#define ADDR_SIZE_93C46	7
#define ADDR_SIZE_93C56	8
#define ADDR_SIZE_93C66	9
#define ADDR_SIZE_93C76	10
#define ADDR_SIZE_93C86	11

#define TS 50
#define TIMEOUT 100000

/**************************************************************************************************/

void read_93Cxx(int dev_size, int org, int alen)
{
    unsigned int data, addr;
    char n = 0;

    if(org == 16) n = 1;
    alen -= n;
    
    uWire_init( org );

    progress_loop( addr, dev_size, "Reading..."){
	uWire_read_cmd( addr >> n, alen, TS);
	data = uWire_word( 0, org, TS);
	uWire_stop( TS );
	put_buffer( addr, LSB(data) );
	if(org == 16)
	    put_buffer( ++addr, MSB(data) );
    }
    finish_action();
}

void verify_93Cxx(int dev_size, int org, int alen)
{
    unsigned int addr;
    unsigned int rdata = 0, bdata = 0;
    char text[256];

    char n = 0;

    if(org == 16) n = 1;
    alen -= n;
    
    uWire_init( org );
    progress_loop( addr, dev_size, "Veryfying..."){
	uWire_read_cmd( addr >> n, alen, TS);
	rdata = uWire_word( 0, org, TS);
	uWire_stop( TS );
	bdata = get_buffer( addr ) & 0xff;
	break_if( bdata != LSB(rdata));
	if(org == 16){
	    bdata = get_buffer( ++addr ) & 0xff;	
	    break_if( bdata != MSB(rdata) );
	}
    }
    finish_action();

    text[0] = 0;
    if( ERROR_VAL )
	sprintf(text, "[WN][TEXT] Memory and buffer differ !!!\n Address = 0x%X\nBuffer=0x%X, Device=0x%X[/TEXT][BR]OK", addr, bdata & 0xff, rdata & 0xff);
    if( rdata >= 0 ){
	show_message(0, ERROR_VAL ? text: "[IF][TEXT] Memory and buffer are consitent[/TEXT][BR]OK", NULL, NULL);    
	ERROR_VAL = 0;
    }
}

void test_blank_93Cxx(int dev_size, int org, int alen)
{
    unsigned int addr;
    unsigned int rdata = 0;
    char text[256];

    char n = 0;

    if(org == 16) n = 1;
    alen -= n;
    
    uWire_init( org );
    progress_loop( addr, dev_size, "Test blank..."){
	uWire_read_cmd( addr >> n, alen, TS);
	rdata = uWire_word( 0, org, TS);
	uWire_stop( TS );
	break_if( (rdata & 0xff) != 0xff );
	if( org == 16 )
	    break_if( (rdata & 0xff00) != 0xff00 );
    }
    finish_action();

    text[0] = 0;
    if( ERROR_VAL )
	sprintf(text, "[WN][TEXT] Memory is dirty !!!\n Address = 0x%X\nDevice=0x%X[/TEXT][BR]OK", addr, rdata & 0xff);
    if( rdata >= 0 ){
	show_message(0, ERROR_VAL ? text: "[IF][TEXT] Memory is clean[/TEXT][BR]OK", NULL, NULL);    
	ERROR_VAL = 0;
    }
}

void write_93Cxx(int dev_size, int org, int alen)
{
    unsigned long *lb;
    unsigned int data, addr;
    char n = 0;

    lb = checkbox(
	"[TITLE]Writing chip[/TITLE][TYPE:QS]"
	"[CB:2:0: Are you sure ? (Tick if Yes)]"
	"[CB:1:1: Verify after process]"
    );
    if( !lb ) return; // resignation by button
    if( !(*lb & 2) ) return; // Not checked

    if(org == 16) n = 1;

    alen -= n;
    
    uWire_init( org );
    uWire_ewen_cmd( alen, TS);
    progress_loop( addr, dev_size, "Writing..."){
	uWire_write_cmd( addr >> n, alen, TS);
	data = get_buffer( addr ) & 0xff;
	if(org == 16)
	    data |= (get_buffer( ++addr ) << 8) & 0xff00;
	uWire_word( data, org, TS);
	uWire_stop( TS );	
	break_if( uWire_wait_busy( TS, TIMEOUT) ); // wait for end of write internal cycle
    }
    finish_action();
    if( (*lb & 1) && !ERROR_VAL) 
	verify_93Cxx( dev_size, org, alen);
}

void erase_93Cxx(int dev_size, int org, int alen)
{
    unsigned long *lb;

    lb = checkbox(
	"[TITLE]Erase chip[/TITLE][TYPE:QS]"
	"[CB:2:0: Are you sure ? (Tick if Yes)]"
	"[CB:1:1: Test blank]"
    );
    if( !lb ) return; // resignation by button
    if( !(*lb & 2) ) return; // Not checked

    if(org == 16) alen--;
    
    uWire_init( org );
    uWire_ewen_cmd( alen, TS);
    uWire_eral_cmd( alen, TS);
    if( uWire_wait_busy( TS, TIMEOUT) ) ERROR_VAL = 1;
    finish_action();    

    if( (*lb & 1) && !ERROR_VAL) 
	test_blank_93Cxx( dev_size, org, alen);
}

/**************************************************************************************************/
REGISTER_FUNCTION(read, 93C06,    93Cxx, SIZE_93C06, 16, ADDR_SIZE_93C06);
REGISTER_FUNCTION(read, 93C13,    93Cxx, SIZE_93C13, 16, ADDR_SIZE_93C13);
REGISTER_FUNCTION(read, 93C14,    93Cxx, SIZE_93C14, 16, ADDR_SIZE_93C14);
REGISTER_FUNCTION(read, 93C46_8,  93Cxx, SIZE_93C46, 8,  ADDR_SIZE_93C46);
REGISTER_FUNCTION(read, 93C46_16, 93Cxx, SIZE_93C46, 16, ADDR_SIZE_93C46);
REGISTER_FUNCTION(read, 93C56_8,  93Cxx, SIZE_93C56, 8,  ADDR_SIZE_93C56);
REGISTER_FUNCTION(read, 93C56_16, 93Cxx, SIZE_93C56, 16, ADDR_SIZE_93C56);
REGISTER_FUNCTION(read, 93C66_8,  93Cxx, SIZE_93C66, 8,  ADDR_SIZE_93C66);
REGISTER_FUNCTION(read, 93C66_16, 93Cxx, SIZE_93C66, 16, ADDR_SIZE_93C66);
REGISTER_FUNCTION(read, 93C76_8,  93Cxx, SIZE_93C76, 8,  ADDR_SIZE_93C76);
REGISTER_FUNCTION(read, 93C76_16, 93Cxx, SIZE_93C76, 16, ADDR_SIZE_93C76);
REGISTER_FUNCTION(read, 93C86_8,  93Cxx, SIZE_93C86, 8,  ADDR_SIZE_93C86);
REGISTER_FUNCTION(read, 93C86_16, 93Cxx, SIZE_93C86, 16, ADDR_SIZE_93C86);

REGISTER_FUNCTION(verify, 93C06,    93Cxx, SIZE_93C06, 16, ADDR_SIZE_93C06);
REGISTER_FUNCTION(verify, 93C13,    93Cxx, SIZE_93C13, 16, ADDR_SIZE_93C13);
REGISTER_FUNCTION(verify, 93C14,    93Cxx, SIZE_93C14, 16, ADDR_SIZE_93C14);
REGISTER_FUNCTION(verify, 93C46_8,  93Cxx, SIZE_93C46, 8,  ADDR_SIZE_93C46);
REGISTER_FUNCTION(verify, 93C46_16, 93Cxx, SIZE_93C46, 16, ADDR_SIZE_93C46);
REGISTER_FUNCTION(verify, 93C56_8,  93Cxx, SIZE_93C56, 8,  ADDR_SIZE_93C56);
REGISTER_FUNCTION(verify, 93C56_16, 93Cxx, SIZE_93C56, 16, ADDR_SIZE_93C56);
REGISTER_FUNCTION(verify, 93C66_8,  93Cxx, SIZE_93C66, 8,  ADDR_SIZE_93C66);
REGISTER_FUNCTION(verify, 93C66_16, 93Cxx, SIZE_93C66, 16, ADDR_SIZE_93C66);
REGISTER_FUNCTION(verify, 93C76_8,  93Cxx, SIZE_93C76, 8,  ADDR_SIZE_93C76);
REGISTER_FUNCTION(verify, 93C76_16, 93Cxx, SIZE_93C76, 16, ADDR_SIZE_93C76);
REGISTER_FUNCTION(verify, 93C86_8,  93Cxx, SIZE_93C86, 8,  ADDR_SIZE_93C86);
REGISTER_FUNCTION(verify, 93C86_16, 93Cxx, SIZE_93C86, 16, ADDR_SIZE_93C86);

REGISTER_FUNCTION(test_blank, 93C06,    93Cxx, SIZE_93C06, 16, ADDR_SIZE_93C06);
REGISTER_FUNCTION(test_blank, 93C13,    93Cxx, SIZE_93C13, 16, ADDR_SIZE_93C13);
REGISTER_FUNCTION(test_blank, 93C14,    93Cxx, SIZE_93C14, 16, ADDR_SIZE_93C14);
REGISTER_FUNCTION(test_blank, 93C46_8,  93Cxx, SIZE_93C46, 8,  ADDR_SIZE_93C46);
REGISTER_FUNCTION(test_blank, 93C46_16, 93Cxx, SIZE_93C46, 16, ADDR_SIZE_93C46);
REGISTER_FUNCTION(test_blank, 93C56_8,  93Cxx, SIZE_93C56, 8,  ADDR_SIZE_93C56);
REGISTER_FUNCTION(test_blank, 93C56_16, 93Cxx, SIZE_93C56, 16, ADDR_SIZE_93C56);
REGISTER_FUNCTION(test_blank, 93C66_8,  93Cxx, SIZE_93C66, 8,  ADDR_SIZE_93C66);
REGISTER_FUNCTION(test_blank, 93C66_16, 93Cxx, SIZE_93C66, 16, ADDR_SIZE_93C66);
REGISTER_FUNCTION(test_blank, 93C76_8,  93Cxx, SIZE_93C76, 8,  ADDR_SIZE_93C76);
REGISTER_FUNCTION(test_blank, 93C76_16, 93Cxx, SIZE_93C76, 16, ADDR_SIZE_93C76);
REGISTER_FUNCTION(test_blank, 93C86_8,  93Cxx, SIZE_93C86, 8,  ADDR_SIZE_93C86);
REGISTER_FUNCTION(test_blank, 93C86_16, 93Cxx, SIZE_93C86, 16, ADDR_SIZE_93C86);

REGISTER_FUNCTION(write, 93C06,    93Cxx, SIZE_93C06, 16, ADDR_SIZE_93C06);
REGISTER_FUNCTION(write, 93C13,    93Cxx, SIZE_93C13, 16, ADDR_SIZE_93C13);
REGISTER_FUNCTION(write, 93C14,    93Cxx, SIZE_93C14, 16, ADDR_SIZE_93C14);
REGISTER_FUNCTION(write, 93C46_8,  93Cxx, SIZE_93C46, 8,  ADDR_SIZE_93C46);
REGISTER_FUNCTION(write, 93C46_16, 93Cxx, SIZE_93C46, 16, ADDR_SIZE_93C46);
REGISTER_FUNCTION(write, 93C56_8,  93Cxx, SIZE_93C56, 8,  ADDR_SIZE_93C56);
REGISTER_FUNCTION(write, 93C56_16, 93Cxx, SIZE_93C56, 16, ADDR_SIZE_93C56);
REGISTER_FUNCTION(write, 93C66_8,  93Cxx, SIZE_93C66, 8,  ADDR_SIZE_93C66);
REGISTER_FUNCTION(write, 93C66_16, 93Cxx, SIZE_93C66, 16, ADDR_SIZE_93C66);
REGISTER_FUNCTION(write, 93C76_8,  93Cxx, SIZE_93C76, 8,  ADDR_SIZE_93C76);
REGISTER_FUNCTION(write, 93C76_16, 93Cxx, SIZE_93C76, 16, ADDR_SIZE_93C76);
REGISTER_FUNCTION(write, 93C86_8,  93Cxx, SIZE_93C86, 8,  ADDR_SIZE_93C86);
REGISTER_FUNCTION(write, 93C86_16, 93Cxx, SIZE_93C86, 16, ADDR_SIZE_93C86);

REGISTER_FUNCTION(erase, 93C06,    93Cxx, SIZE_93C06, 16, ADDR_SIZE_93C06);
REGISTER_FUNCTION(erase, 93C13,    93Cxx, SIZE_93C13, 16, ADDR_SIZE_93C13);
REGISTER_FUNCTION(erase, 93C14,    93Cxx, SIZE_93C14, 16, ADDR_SIZE_93C14);
REGISTER_FUNCTION(erase, 93C46_8,  93Cxx, SIZE_93C46, 8,  ADDR_SIZE_93C46);
REGISTER_FUNCTION(erase, 93C46_16, 93Cxx, SIZE_93C46, 16, ADDR_SIZE_93C46);
REGISTER_FUNCTION(erase, 93C56_8,  93Cxx, SIZE_93C56, 8,  ADDR_SIZE_93C56);
REGISTER_FUNCTION(erase, 93C56_16, 93Cxx, SIZE_93C56, 16, ADDR_SIZE_93C56);
REGISTER_FUNCTION(erase, 93C66_8,  93Cxx, SIZE_93C66, 8,  ADDR_SIZE_93C66);
REGISTER_FUNCTION(erase, 93C66_16, 93Cxx, SIZE_93C66, 16, ADDR_SIZE_93C66);
REGISTER_FUNCTION(erase, 93C76_8,  93Cxx, SIZE_93C76, 8,  ADDR_SIZE_93C76);
REGISTER_FUNCTION(erase, 93C76_16, 93Cxx, SIZE_93C76, 16, ADDR_SIZE_93C76);
REGISTER_FUNCTION(erase, 93C86_8,  93Cxx, SIZE_93C86, 8,  ADDR_SIZE_93C86);
REGISTER_FUNCTION(erase, 93C86_16, 93Cxx, SIZE_93C86, 16, ADDR_SIZE_93C86);

REGISTER_MODULE_BEGIN( 93Cxx )
    register_chip_begin("/Serial EEPROM/93Cxx/16bit", "93C06", "93Cxx", SIZE_93C06);
	add_action(MODULE_READ_ACTION, read_93C06);
	add_action(MODULE_VERIFY_ACTION, verify_93C06);
	add_action(MODULE_TEST_ACTION, test_blank_93C06);
	add_action(MODULE_PROG_ACTION, write_93C06);
	add_action(MODULE_ERASE_ACTION, erase_93C06);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/16bit", "93C13", "93Cxx", SIZE_93C13);
	add_action(MODULE_READ_ACTION, read_93C13);
	add_action(MODULE_VERIFY_ACTION, verify_93C13);
	add_action(MODULE_TEST_ACTION, test_blank_93C13);
	add_action(MODULE_PROG_ACTION, write_93C13);
	add_action(MODULE_ERASE_ACTION, erase_93C13);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/8bit", "93C46 (8bit)", "93Cxx", SIZE_93C46);
	add_action(MODULE_READ_ACTION, read_93C46_8);
	add_action(MODULE_VERIFY_ACTION, verify_93C46_8);
	add_action(MODULE_TEST_ACTION, test_blank_93C46_8);
	add_action(MODULE_PROG_ACTION, write_93C46_8);
	add_action(MODULE_ERASE_ACTION, erase_93C46_8);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/16bit", "93C14", "93Cxx", SIZE_93C14);
	add_action(MODULE_READ_ACTION, read_93C14);
	add_action(MODULE_VERIFY_ACTION, verify_93C14);
	add_action(MODULE_TEST_ACTION, test_blank_93C14);
	add_action(MODULE_PROG_ACTION, write_93C14);
	add_action(MODULE_ERASE_ACTION, erase_93C14);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/16bit", "93C46 (16bit)", "93Cxx", SIZE_93C46);
	add_action(MODULE_READ_ACTION, read_93C46_16);
	add_action(MODULE_VERIFY_ACTION, verify_93C46_16);
	add_action(MODULE_TEST_ACTION, test_blank_93C46_16);
	add_action(MODULE_PROG_ACTION, write_93C46_16);
	add_action(MODULE_ERASE_ACTION, erase_93C46_16);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/8bit", "93C56 (8bit)", "93Cxx", SIZE_93C56);
	add_action(MODULE_READ_ACTION, read_93C56_8);
	add_action(MODULE_VERIFY_ACTION, verify_93C56_8);
	add_action(MODULE_TEST_ACTION, test_blank_93C56_8);
	add_action(MODULE_PROG_ACTION, write_93C56_8);
	add_action(MODULE_ERASE_ACTION, erase_93C56_8);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/16bit", "93C56 (16bit)", "93Cxx", SIZE_93C56);
	add_action(MODULE_READ_ACTION, read_93C56_16);
	add_action(MODULE_VERIFY_ACTION, verify_93C56_16);
	add_action(MODULE_TEST_ACTION, test_blank_93C56_16);
	add_action(MODULE_PROG_ACTION, write_93C56_16);
	add_action(MODULE_ERASE_ACTION, erase_93C56_16);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/8bit", "93C66 (8bit)", "93Cxx", SIZE_93C66);
	add_action(MODULE_READ_ACTION, read_93C66_8);
	add_action(MODULE_VERIFY_ACTION, verify_93C66_8);
	add_action(MODULE_TEST_ACTION, test_blank_93C66_8);
	add_action(MODULE_PROG_ACTION, write_93C66_8);
	add_action(MODULE_ERASE_ACTION, erase_93C66_8);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/16bit", "93C66 (16bit)", "93Cxx", SIZE_93C66);
	add_action(MODULE_READ_ACTION, read_93C66_16);
	add_action(MODULE_VERIFY_ACTION, verify_93C66_16);
	add_action(MODULE_TEST_ACTION, test_blank_93C66_16);
	add_action(MODULE_PROG_ACTION, write_93C66_16);
	add_action(MODULE_ERASE_ACTION, erase_93C66_16);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/8bit", "93C76 (8bit)", "93Cxx", SIZE_93C76);
	add_action(MODULE_READ_ACTION, read_93C76_8);
	add_action(MODULE_VERIFY_ACTION, verify_93C76_8);
	add_action(MODULE_TEST_ACTION, test_blank_93C76_8);
	add_action(MODULE_PROG_ACTION, write_93C76_8);
	add_action(MODULE_ERASE_ACTION, erase_93C76_8);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/16bit", "93C76 (16bit)", "93Cxx", SIZE_93C76);
	add_action(MODULE_READ_ACTION, read_93C76_16);
	add_action(MODULE_VERIFY_ACTION, verify_93C76_16);
	add_action(MODULE_TEST_ACTION, test_blank_93C76_16);
	add_action(MODULE_PROG_ACTION, write_93C76_16);
	add_action(MODULE_ERASE_ACTION, erase_93C76_16);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/8bit", "93C86 (8bit)", "93Cxx", SIZE_93C86);
	add_action(MODULE_READ_ACTION, read_93C86_8);
	add_action(MODULE_VERIFY_ACTION, verify_93C86_8);
	add_action(MODULE_TEST_ACTION, test_blank_93C86_8);
	add_action(MODULE_PROG_ACTION, write_93C86_8);
	add_action(MODULE_ERASE_ACTION, erase_93C86_8);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/93Cxx/16bit", "93C86 (16bit)", "93Cxx", SIZE_93C86);
	add_action(MODULE_READ_ACTION, read_93C86_16);
	add_action(MODULE_VERIFY_ACTION, verify_93C86_16);
	add_action(MODULE_TEST_ACTION, test_blank_93C86_16);
	add_action(MODULE_PROG_ACTION, write_93C86_16);
	add_action(MODULE_ERASE_ACTION, erase_93C86_16);
    register_chip_end;

REGISTER_MODULE_END

