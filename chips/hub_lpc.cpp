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

#define LF002_SIZE	KB_SIZE( 256 )
#define LF003_SIZE	KB_SIZE( 384 )
#define LF004_SIZE	KB_SIZE( 512 )
#define LF008_SIZE	MB_SIZE( 1 )

#define LF002_START	0x00000
#define LF003_START	0x20000
#define LF004_START	0x00000
#define LF008_START	0x00000

#define TT			50	// cycle time
#define TIMEOUT			500

#define WE_HUB( state )		hw_set_ce( state )
#define OE_HUB( state )		hw_set_oe( state )
#define RC_HUB( state )		hw_sw_vcc( state ) // as RC signal in adapter
#define VCC_HUB( state )	hw_sw_vpp( state ) // as VCC in adapter

/********************************* LOW LEVEL OPERATIONS ******************************************************/
void init_HUB()
{
    hw_set_vcc( 500  );
    hw_set_vpp( 1200 );
    WE_HUB( 1 );
    OE_HUB( 1 );    
    RC_HUB( 1 );    
    VCC_HUB( 1 );    
    hw_ms_delay(200); // time for reset
}

void write_data_HUB( unsigned int addr, unsigned char data)
{
    hw_set_addr( addr & 0x7ff );
    hw_us_delay( TT );    
    RC_HUB( 0 );	 // store low 11 bits of address
    OE_HUB( 1 );    
    WE_HUB( 0 );
    hw_us_delay( TT );
    hw_set_addr( (addr >> 11) & 0x7ff );
    RC_HUB( 1 );	 // store high 11 bits of address    
    hw_set_data( data ); // set data
    hw_us_delay( TT );
    WE_HUB( 1 );	// store data
    hw_us_delay( TT );
}

unsigned char read_data_HUB( unsigned int addr)
{
    unsigned char data = 0;

    RC_HUB( 1 );	
    OE_HUB( 1 );    
    WE_HUB( 1 );
    hw_set_addr( addr & 0x7ff ); // low 11 bits
    RC_HUB( 0 );	 	 // store low 11 bits of address
    hw_us_delay( TT );
    hw_set_addr( (addr >> 11) & 0x7ff );
    RC_HUB( 1 );	 // store high 11 bits of address    
    hw_us_delay( TT );
    OE_HUB( 0 );    
    hw_us_delay( TT );
    data = hw_get_data();
    hw_us_delay( TT );
    OE_HUB( 1 );        
    return data;
}

unsigned char poll_data_HUB()
{
    unsigned char data;
    hw_us_delay( TT );
    OE_HUB( 0 );    
    hw_us_delay( TT );
    data = hw_get_data();
    hw_us_delay( TT );
    OE_HUB( 1 );        
    return data;
}

/*********************************************************************************************/

void read_HUB(unsigned int dev_size, unsigned int start)
{
    unsigned int addr;

    init_HUB();
    progress_loop(addr, dev_size, "Reading...")
	put_buffer( addr, read_data_HUB( start + addr ) );
    finish_action();    
}

void verify_HUB(unsigned int dev_size, unsigned int start)
{
    char text[256];
    unsigned int addr;
    unsigned char rdata = 0, bdata = 0;

    init_HUB();
    progress_loop(addr, dev_size, "Veryfying..."){
	rdata = read_data_HUB( start + addr );
	bdata = get_buffer( addr );
	break_if( rdata != bdata );
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

void test_blank_HUB(unsigned int dev_size, unsigned int start)
{
    char text[256];
    unsigned int addr;
    unsigned char rdata = 0;

    init_HUB();
    progress_loop(addr, dev_size, "Test Blank..."){
	rdata = read_data_HUB( start + addr );
	break_if( rdata != 0xff );
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

void sign_HUB()
{
    unsigned char man, id, tbl;
    char chip[256], vendor[256], text[1024];

    init_HUB();

    write_data_HUB( 0x5555, 0xaa);
    write_data_HUB( 0x2aaa, 0x55);    
    write_data_HUB( 0x5555, 0x90);

    man = read_data_HUB( 0 );
    id  = read_data_HUB( 1 );    
    tbl = read_data_HUB( 2 ) & 1;   // if 0 boot block is locked for writing

    finish_action();        

    loockup_signature( "HUB/LPC", man, id, vendor, chip);

    sprintf(text, "[IF][TEXT]"
	"Vendor ( 0x%x ): %s\n\n"
	"Chip   ( 0x%x ): %s\n\n"
	"Boot sector protected: %s\n"
	"[/TEXT][BR]OK", 
	man, vendor, 
	id, chip, 
	tbl ? "No" : "Yes"
    );
    show_message(0, text, NULL, NULL);
}

void lock_HUB()
{
    unsigned long *lb;
    int i;
    
    lb = checkbox(
	"[TITLE]Boot Block Lock[/TITLE][TYPE:WN]"
	"[CB:2:0: Are you ABSOLUTLY sure ? (Tick if Yes)]"
	"[CB:1:1: Once locked, cannot be unlocked (Untick if Yes)]"
    );
    if( !lb ) return; // resignation by button
    if( !(*lb & 2) ) return; // Not checked
    if( *lb & 1 ) return;

    init_HUB();

    // take actual flag
    write_data_HUB( 0x5555, 0xaa);
    write_data_HUB( 0x2aaa, 0x55);    
    write_data_HUB( 0x5555, 0x80);
    write_data_HUB( 0x5555, 0xaa);
    write_data_HUB( 0x2aaa, 0x55);    
    write_data_HUB( 0x5555, 0x40);

    progress_loop(i, 50, "Chip erasing...") hw_ms_delay(20);

    finish_action();        
    hw_ms_delay(200);

    show_dialog("[IF][TEXT]BOOT BLOCK LOCKED[/TEXT][BR]OK","");
}


void erase_HUB(unsigned int dev_size, unsigned int start)
{
    unsigned long *lb;
    int i;
    
    lb = checkbox(
	"[TITLE]Erasing chip[/TITLE][TYPE:QS]"
	"[CB:2:0: Are you sure ? (Tick if Yes)]"
	"[CB:1:1: Test blank]"
    );

    if( !lb ) return; // resignation by button
    if( !(*lb & 2) ) return; // Not checked

    init_HUB();
    write_data_HUB( 0x5555, 0xaa);
    write_data_HUB( 0x2aaa, 0x55);    
    write_data_HUB( 0x5555, 0x80);
    write_data_HUB( 0x5555, 0xaa);    
    write_data_HUB( 0x2aaa, 0x55);    
    write_data_HUB( 0x5555, 0x10);    
    progress_loop(i, 100, "Chip erasing...") hw_ms_delay(20);
    finish_action();        
    hw_ms_delay(200);
    
    if( (*lb & 1) && !ERROR_VAL) 
	test_blank_HUB( dev_size, start);
}

char wait_HUB( unsigned char d, char mode) // 0 for polling, 1 for toggle
{
    int i;
    for(i = 0; i < TIMEOUT; i++){
	if((poll_data_HUB() & 0x80) == (d & 0x80)) return 0;
	hw_us_delay(5 * TT);
    }
    return 1;
}

void prog_HUB(unsigned int dev_size, unsigned int start)
{
    unsigned long *lb;
    unsigned char bdata;
    int addr;
    
    lb = checkbox(
	"[TITLE]Writing chip[/TITLE][TYPE:QS]"
	"[CB:2:0: Are you sure ? (Tick if Yes)]"
	"[CB:1:1: Verify]"
    );

    if( !lb ) return; // resignation by button
    if( !(*lb & 2) ) return; // Not checked

    init_HUB();

    progress_loop(addr, dev_size, "Writing chip..."){
	write_data_HUB( 0x5555, 0xaa);
        write_data_HUB( 0x2aaa, 0x55);    
    	write_data_HUB( 0x5555, 0xa0);
	bdata = get_buffer( addr);
	write_data_HUB( addr + start, bdata);        
	break_if( wait_HUB( bdata, 0 ) );	
    }
    
    finish_action();        
    hw_ms_delay(200);
    
    if( (*lb & 1) && !ERROR_VAL) 
	verify_HUB( dev_size, start);
}


/*********************************************************************************************/

REGISTER_FUNCTION( sign,  HUB_, HUB);
REGISTER_FUNCTION( lock,  HUB_, HUB);

REGISTER_FUNCTION( read,  LF002, HUB, LF002_SIZE, LF002_START);
REGISTER_FUNCTION( read,  LF003, HUB, LF003_SIZE, LF003_START);
REGISTER_FUNCTION( read,  LF004, HUB, LF004_SIZE, LF004_START);
REGISTER_FUNCTION( read,  LF008, HUB, LF008_SIZE, LF008_START);

REGISTER_FUNCTION( verify,  LF002, HUB, LF002_SIZE, LF002_START);
REGISTER_FUNCTION( verify,  LF003, HUB, LF003_SIZE, LF003_START);
REGISTER_FUNCTION( verify,  LF004, HUB, LF004_SIZE, LF004_START);
REGISTER_FUNCTION( verify,  LF008, HUB, LF008_SIZE, LF008_START);

REGISTER_FUNCTION( test_blank,  LF002, HUB, LF002_SIZE, LF002_START);
REGISTER_FUNCTION( test_blank,  LF003, HUB, LF003_SIZE, LF003_START);
REGISTER_FUNCTION( test_blank,  LF004, HUB, LF004_SIZE, LF004_START);
REGISTER_FUNCTION( test_blank,  LF008, HUB, LF008_SIZE, LF008_START);

REGISTER_FUNCTION( erase,  LF002, HUB, LF002_SIZE, LF002_START);
REGISTER_FUNCTION( erase,  LF003, HUB, LF003_SIZE, LF003_START);
REGISTER_FUNCTION( erase,  LF004, HUB, LF004_SIZE, LF004_START);
REGISTER_FUNCTION( erase,  LF008, HUB, LF008_SIZE, LF008_START);

REGISTER_FUNCTION( prog,  LF002, HUB, LF002_SIZE, LF002_START);
REGISTER_FUNCTION( prog,  LF003, HUB, LF003_SIZE, LF003_START);
REGISTER_FUNCTION( prog,  LF004, HUB, LF004_SIZE, LF004_START);
REGISTER_FUNCTION( prog,  LF008, HUB, LF008_SIZE, LF008_START);

REGISTER_MODULE_BEGIN(HUB_HUB)

    register_chip_begin("/HUB LPC/SST49LFxxx", "SST49LF002, SST49LF020", "HUB_LPC", LF002_SIZE);
	add_action(MODULE_READ_ACTION, read_LF002);
	add_action(MODULE_VERIFY_ACTION, verify_LF002);
	add_action(MODULE_TEST_ACTION, test_blank_LF002);
	add_action(MODULE_SIGN_ACTION, sign_HUB_);
	add_action(MODULE_ERASE_ACTION, erase_LF002);
	add_action(MODULE_PROG_ACTION, prog_LF002);
    register_chip_end;

    register_chip_begin("/HUB LPC/SST49LFxxx", "SST49LF003", "HUB_LPC", LF003_SIZE);
	add_action(MODULE_READ_ACTION, read_LF003);
	add_action(MODULE_VERIFY_ACTION, verify_LF003);
	add_action(MODULE_TEST_ACTION, test_blank_LF003);
	add_action(MODULE_SIGN_ACTION, sign_HUB_);
	add_action(MODULE_ERASE_ACTION, erase_LF003);
	add_action(MODULE_PROG_ACTION, prog_LF003);
    register_chip_end;

    register_chip_begin("/HUB LPC/SST49LFxxx", "SST49LF004, SST49LF040", "HUB_LPC", LF004_SIZE);
	add_action(MODULE_READ_ACTION, read_LF004);
	add_action(MODULE_VERIFY_ACTION, verify_LF004);
	add_action(MODULE_TEST_ACTION, test_blank_LF004);
	add_action(MODULE_SIGN_ACTION, sign_HUB_);
	add_action(MODULE_ERASE_ACTION, erase_LF004);
	add_action(MODULE_PROG_ACTION, prog_LF004);
    register_chip_end;

    register_chip_begin("/HUB LPC/SST49LFxxx", "SST49LF008", "HUB_LPC", LF008_SIZE);
	add_action(MODULE_READ_ACTION, read_LF008);
	add_action(MODULE_VERIFY_ACTION, verify_LF008);
	add_action(MODULE_TEST_ACTION, test_blank_LF008);
	add_action(MODULE_SIGN_ACTION, sign_HUB_);
	add_action(MODULE_ERASE_ACTION, erase_LF008);
	add_action(MODULE_PROG_ACTION, prog_LF008);
    register_chip_end;

    register_chip_begin("/HUB LPC/W49Vxxx", "W49V002", "HUB_LPC", LF002_SIZE);
	add_action(MODULE_READ_ACTION, read_LF002);
	add_action(MODULE_VERIFY_ACTION, verify_LF002);
	add_action(MODULE_TEST_ACTION, test_blank_LF002);
	add_action(MODULE_SIGN_ACTION, sign_HUB_);
	add_action(MODULE_ERASE_ACTION, erase_LF002);
	add_action(MODULE_PROG_ACTION, prog_LF002);
	add_action(MODULE_LOCKBIT_ACTION, lock_HUB_);	
    register_chip_end;

REGISTER_MODULE_END

