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

#include "modules.h"

MODULE_IMPLEMENTATION

// 24 pin
#define SIZE_2716	KB_SIZE( 2 )
#define SIZE_2732	KB_SIZE( 4 )
// 28pin
#define SIZE_2764	KB_SIZE( 8 )
#define SIZE_27128	KB_SIZE( 16 )
#define SIZE_27256	KB_SIZE( 32 )
#define SIZE_27512	KB_SIZE( 64 )
// 32pin
#define SIZE_27C010	KB_SIZE( 128 )
#define SIZE_27C020	KB_SIZE( 256 )
#define SIZE_27C040	KB_SIZE( 512 )
#define SIZE_27C080	MB_SIZE( 1 )
// 40 pin
#define SIZE_27C1024	KB_SIZE( 128 )	// 16 x 64k
#define SIZE_27C2048	KB_SIZE( 256 )  // 16 x 128k 
#define SIZE_27C4096	KB_SIZE( 512 )  // 16 x 256k
// 42pin
#define SIZE_27C400	KB_SIZE( 512 )
#define SIZE_27C800	MB_SIZE( 1 )
#define SIZE_27C160	MB_SIZE( 2 )
#define SIZE_27C322	MB_SIZE( 4 )


/* 2716 */
char read_byte_2716( int addr )
{
    char data;
    set_address( addr );
    oe(0, 1);
    data = hw_get_data();
    oe(1, 1);
    return data;
}

void read_2716_(int size, char ce)
{
    int addr;
    unsigned char data;

    start_action(0, ce);
    progress_loop(addr, size, "Reading data"){
	data = read_byte_2716( addr );
	put_buffer(addr, data);
    }    
    finish_action();
}

char test_2716_(int size, char ce, char silent)
{
    int addr;
    unsigned char rdata = 0;
    char text[256];
    
    start_action(0, ce);
    progress_loop(addr, size, "Test blank"){
	rdata = read_byte_2716( addr );
	break_if( rdata != 0xff );
    }    
    finish_action();
    if( !silent | (rdata != 0xff)){
	sprintf( text,
	    "[WN][TEXT]Chip is not empty !!!\n Address = 0x%X\nByte =0x%X%X [/TEXT][BR]OK",
	    addr,
	    to_hex(rdata, 1), to_hex(rdata, 0)
	);    
	show_message(0, (rdata == 0xff) ? "[IF][TEXT]Chip is clear.[/TEXT][BR]OK" : text, NULL, NULL);
    }
    return rdata != 0xff;
}

void verify_2716_(int size, char ce)
{
    int addr;
    unsigned char rdata = 0, wdata = 0;
    char text[256];
    
    start_action(0, ce);
    progress_loop(addr, size, "Verifying data"){
	rdata = read_byte_2716( addr );
	wdata = get_buffer( addr );
	break_if( rdata != wdata );
    }    
    finish_action();
    sprintf( text,
	"[WN][TEXT]Inconsistent chip and buffer !!!\n Address = 0x%X\nByte = 0x%X%X, should be 0x%X%X [/TEXT][BR]OK",
	addr,
	to_hex(rdata, 1), to_hex(rdata, 0),
	to_hex(wdata, 1), to_hex(wdata, 0)
    );    
    show_message(0, (rdata == wdata) ? "[IF][TEXT]Chip and buffer are consistent.[/TEXT][BR]OK" : text, NULL, NULL);
}

void write_byte_2716(int addr, char data)
{
    oe(1, 1);
    set_address( addr );
    hw_delay(100);
    set_data(data);    
    hw_delay(100);
    ce(1, 50000); // positive program pulse 50ms
    ce(0, 1000); 
}

void prog_2716_(int size)
{
    unsigned long *lb;
    int addr, tries;
    unsigned char rdata = 0, wdata = 0;
    char text[256];
    
    lb = checkbox(
	"[TITLE]Chip burning[/TITLE][TYPE:QS]"
	"[FRAME:Algorithm][RADIO]"
	"[CB:4:0: Slow 50ms]"
	"[/RADIO][/FRAME]"
	"[CB:8:1: Test blank]"
	"[CB:16:1: Verify]"
    );
    if( !lb ) return;
    if( (*lb & 0x07) != 4 ) return; // missing algorithm choice
    if( *lb & 0x08 )	// test blank
	if(test_2716_(size, 0, 1)) return;

    start_action(0, 0);
    hw_sw_vpp(1);
    progress_loop(addr, size, "Writing data"){
        tries = 0;
	do{
	    wdata = get_buffer( addr );	
	    if(wdata != 0xff)
		write_byte_2716( addr, wdata );	
	    rdata = read_byte_2716( addr );
	} while( (wdata != rdata) && (++tries < 20));
	break_if( rdata != wdata );
    }    
    finish_action();
    if(rdata != wdata ){
	sprintf( text,
	    "[WN][TEXT]Write error !!!\n Address = 0x%X\nByte = 0x%X%X, should be 0x%X%X [/TEXT][BR]OK",
	    addr,
	    to_hex(rdata, 1), to_hex(rdata, 0),
	    to_hex(wdata, 1), to_hex(wdata, 0)
	);    
	show_message(0, (rdata == wdata) ? "[IF][TEXT]Chip program OK.[/TEXT][BR]OK" : text, NULL, NULL);
	return;
    }

    if( *lb & 0x10 )
	verify_2716_( size, 0 );
}

void write_byte_2732(int addr, char data)
{
    oe(0, 1);
    ce(1, 100);
    hw_sw_vpp(1);
    hw_delay( 100 );
    set_address( addr );
    set_data(data);    
    hw_delay( 5 );
    hw_delay( 100 );
    ce(0, 50000);
    ce(1, 5);
    hw_sw_vpp(0);   
    ce(0, 1);
    oe(1, 100); 
}

/**********************************************************************************************/
void write_eprom(int addr, char data, int time, char ce_pgm)
{
    oe(1, 10);
    set_address( addr );
    set_data(data);    
    hw_delay( 2 ); 	// data and address valid, oe = 1, PGM = 1
    if( ce_pgm ){
	ce(0, time);	// ce = 0
	ce(1, 100);	// ce = 1
    } else {
	pgm(0, time);	// pgm = 0
	pgm(1, 100);	// pgm = 1
    }
}

char read_eprom(int addr, char oe_vpp)
{
    char data;
    set_address(addr);
    hw_delay(5); // tPHQX + tQXGL
    oe(0, 5);
    if(oe_vpp){
        hw_sw_vpp(0);
        ce(0, 1);
    }
    data = hw_get_data();
    if(oe_vpp){
      ce(1, 1);
      hw_sw_vpp(1);
    }
    oe(1, 2);
    return data;
}

void prog_eprom(int size, char ce_pgm, char oe_vpp)
{
    int addr, x;
    unsigned char rdata = 0, wdata = 0;
    unsigned long *lb;
    char text[256];
    
    lb = checkbox(
	"[TITLE]Chip burning[/TITLE][TYPE:QS]"
	"[FRAME:Algorithm][RADIO]"
	"[CB:1:0: FAST]"
	"[CB:2:0: RAPID-PULSE]"
	"[CB:4:0: Slow 50ms]"
	"[/RADIO][/FRAME]"
	"[CB:8:1: Test blank]"
	"[CB:16:1: Verify]"
    );
    if( !lb ) return;
    x = *lb & 0x07;
    if(!(( x == 1) || (x == 2) || (x == 4))) return; // missing algorithm choice
    if( *lb & 0x08 )	// test blank
	if(test_2716_(size, !ce_pgm, 1)) return;
    if( !ce_pgm ) hw_pragma( PRAGMA_CE_EQ_PGM ); // if set, ignoring ce() command for programmers like willem, PROG() will be CE
    ce(ce_pgm, 10);
    pgm(1, 10 );
    hw_set_vcc( 6 );
    hw_sw_vcc(1);
    hw_sw_vpp( 1 );
    progress_loop(addr, size, "Writing data"){
	wdata = get_buffer( addr );	
	if(wdata != 0xff){
	    if( *lb & 1 ){ // Fast
		for(x = 1; x < 26; x++){
		    write_eprom( addr, wdata, 1000, ce_pgm ); // impuls 1ms
		    rdata = read_eprom( addr, oe_vpp );
		    if( wdata == rdata ) break;
		}
		break_if( wdata != rdata );
		write_eprom( addr, wdata, 3 * x * 1000, ce_pgm ); // impuls 3xms
	    }
	    if( *lb & 2){ // rapid-pulse
		for(x = 1; x < 26; x++){
		    write_eprom( addr, wdata, 100, ce_pgm ); // impuls 100µs
		    rdata = read_eprom( addr, oe_vpp );
		    if( wdata == rdata ) break;
		}
		break_if( wdata != rdata );
	    }
	    if( *lb & 4){ // Slow 50ms
		write_eprom( addr, wdata, 50000, ce_pgm ); // impuls 50ms
		rdata = read_eprom( addr, oe_vpp );
		break_if( wdata != rdata );	    
	    }
	} else 
	    rdata = read_eprom( addr, oe_vpp );
	break_if( rdata != wdata );
    }    
    hw_sw_vpp(0);
    hw_sw_vcc(0);    
    set_address(0);
    set_data(0);
    if(rdata != wdata ){
	sprintf( text,
	    "[WN][TEXT]Write error !!!\n Address = 0x%X\nByte = 0x%X%X, should be 0x%X%X [/TEXT][BR]OK",
	    addr,
	    to_hex(rdata, 1), to_hex(rdata, 0),
	    to_hex(wdata, 1), to_hex(wdata, 0)
	);    
	show_message(0, (rdata == wdata) ? "[IF][TEXT]Chip program OK.[/TEXT][BR]OK" : text, NULL, NULL);
	hw_pragma( 0 );
	return;
    }
    if( *lb & 0x10 )
	verify_2716_( size, !ce_pgm );
    hw_pragma( 0 );
}

/*********************************************************************************************************/

/* 2716 */
REGISTER_FUNCTION( read,   2716,  2716_, SIZE_2716, 0 );
REGISTER_FUNCTION( verify, 2716,  2716_, SIZE_2716, 0 );
REGISTER_FUNCTION( test,   2716,  2716_, SIZE_2716, 0, 0 );
REGISTER_FUNCTION( prog,   2716,  2716_, SIZE_2716 );
/* 2732 */
REGISTER_FUNCTION( read,   2732,  2716_, SIZE_2732, 0 );
REGISTER_FUNCTION( verify, 2732,  2716_, SIZE_2732, 0 );
REGISTER_FUNCTION( test,   2732,  2716_, SIZE_2732, 0, 0 );
REGISTER_FUNCTION( prog,   2732,  eprom, SIZE_2732, 1, 1 );
/* 2764 */
REGISTER_FUNCTION( read,   2764,  2716_, SIZE_2764, 1 );
REGISTER_FUNCTION( verify, 2764,  2716_, SIZE_2764, 1 );
REGISTER_FUNCTION( test,   2764,  2716_, SIZE_2764, 1, 0 );
REGISTER_FUNCTION( prog,   2764,  eprom, SIZE_2764, 0, 0 );
/* 27128 */
REGISTER_FUNCTION( read,   27128, 2716_, SIZE_27128, 1 );
REGISTER_FUNCTION( verify, 27128, 2716_, SIZE_27128, 1 );
REGISTER_FUNCTION( test,   27128, 2716_, SIZE_27128, 1, 0 );
REGISTER_FUNCTION( prog,   27128, eprom, SIZE_27128, 0, 0 );
/* 27256 */
REGISTER_FUNCTION( read,   27256, 2716_, SIZE_27256, 0 );
REGISTER_FUNCTION( verify, 27256, 2716_, SIZE_27256, 0 );
REGISTER_FUNCTION( test,   27256, 2716_, SIZE_27256, 0, 0 );
REGISTER_FUNCTION( prog,   27256, eprom, SIZE_27256, 1, 0 );
/* 27512 */
REGISTER_FUNCTION( read,   27512, 2716_, SIZE_27512, 0 );
REGISTER_FUNCTION( verify, 27512, 2716_, SIZE_27512, 0 );
REGISTER_FUNCTION( test,   27512, 2716_, SIZE_27512, 0, 0 );
REGISTER_FUNCTION( prog,   27512, eprom, SIZE_27512, 1, 1 );
//REGISTER_FUNCTION( erase,  27512, eprom, SIZE_27512, 1, 1 );
/* 27C010 */
REGISTER_FUNCTION( read,   27C010, 2716_, SIZE_27C010, 0 );
REGISTER_FUNCTION( verify, 27C010, 2716_, SIZE_27C010, 0 );
REGISTER_FUNCTION( test,   27C010, 2716_, SIZE_27C010, 1, 0 );
REGISTER_FUNCTION( prog,   27C010, eprom, SIZE_27C010, 0, 0 );
/* 27C020 */
REGISTER_FUNCTION( read,   27C020, 2716_, SIZE_27C020, 0 );
REGISTER_FUNCTION( verify, 27C020, 2716_, SIZE_27C020, 0 );
REGISTER_FUNCTION( test,   27C020, 2716_, SIZE_27C020, 1, 0 );
REGISTER_FUNCTION( prog,   27C020, eprom, SIZE_27C020, 0, 0 );
/* 27C040 */
REGISTER_FUNCTION( read,   27C040, 2716_, SIZE_27C040, 0 );
REGISTER_FUNCTION( verify, 27C040, 2716_, SIZE_27C040, 0 );
REGISTER_FUNCTION( test,   27C040, 2716_, SIZE_27C040, 0, 0 );
REGISTER_FUNCTION( prog,   27C040, eprom, SIZE_27C040, 1, 0 );
/* 27C080 */
REGISTER_FUNCTION( read,   27C080, 2716_, SIZE_27C080, 0 );
REGISTER_FUNCTION( verify, 27C080, 2716_, SIZE_27C080, 0 );
REGISTER_FUNCTION( test,   27C080, 2716_, SIZE_27C080, 0, 0 );
REGISTER_FUNCTION( prog,   27C080, eprom, SIZE_27C080, 1, 1 );

/* 27C400 */
REGISTER_FUNCTION( read,   27C400, 2716_, SIZE_27C400, 0 );
REGISTER_FUNCTION( verify, 27C400, 2716_, SIZE_27C400, 0 );
REGISTER_FUNCTION( test,   27C400, 2716_, SIZE_27C400, 0, 0);
REGISTER_FUNCTION( prog,   27C400, eprom, SIZE_27C400, 1, 0);
/* 27C800 */
REGISTER_FUNCTION( read,   27C800, 2716_, SIZE_27C800, 0 );
REGISTER_FUNCTION( verify, 27C800, 2716_, SIZE_27C800, 0 );
REGISTER_FUNCTION( test,   27C800, 2716_, SIZE_27C800, 0, 0 );
REGISTER_FUNCTION( prog,   27C800, eprom, SIZE_27C800, 1, 1 );
/* 27C160 */
REGISTER_FUNCTION( read,   27C160, 2716_, SIZE_27C160, 0 );
REGISTER_FUNCTION( verify, 27C160, 2716_, SIZE_27C160, 0 );
REGISTER_FUNCTION( test,   27C160, 2716_, SIZE_27C160, 0, 0 );
REGISTER_FUNCTION( prog,   27C160, eprom, SIZE_27C160, 1, 1 );
/* 27C322 */
REGISTER_FUNCTION( read,   27C322, 2716_, SIZE_27C322, 0 );
REGISTER_FUNCTION( verify, 27C322, 2716_, SIZE_27C322, 0 );
REGISTER_FUNCTION( test,   27C322, 2716_, SIZE_27C322, 0, 0 );
REGISTER_FUNCTION( prog,   27C322, eprom, SIZE_27C322, 1, 1 );

/********************************************************************************************/
REGISTER_MODULE_BEGIN( 27xx )
/* 24 PIN EPROM */
    register_chip_begin("/EPROM/24 pin", "2716", "2716", SIZE_2716);
	add_action(MODULE_READ_ACTION, read_2716);
	add_action(MODULE_PROG_ACTION, prog_2716);
	add_action(MODULE_VERIFY_ACTION, verify_2716);
	add_action(MODULE_TEST_ACTION, test_2716);
    register_chip_end;
    register_chip_begin("/EPROM/24 pin", "2732", "2732", SIZE_2732);
	add_action(MODULE_READ_ACTION, read_2732);
	add_action(MODULE_PROG_ACTION, prog_2732);
	add_action(MODULE_VERIFY_ACTION, verify_2732);
	add_action(MODULE_TEST_ACTION, test_2732);
    register_chip_end;
/* 28 PIN EPROM */
    register_chip_begin("/EPROM/28 pin", "2764", "2764_128", SIZE_2764);
	add_action(MODULE_READ_ACTION, read_2764);
	add_action(MODULE_PROG_ACTION, prog_2764);
	add_action(MODULE_VERIFY_ACTION, verify_2764);
	add_action(MODULE_TEST_ACTION, test_2764);
    register_chip_end;
    register_chip_begin("/EPROM/28 pin", "27128", "2764_128", SIZE_27128);
	add_action(MODULE_READ_ACTION, read_27128);
	add_action(MODULE_PROG_ACTION, prog_27128);
	add_action(MODULE_VERIFY_ACTION, verify_27128);
	add_action(MODULE_TEST_ACTION, test_27128);
    register_chip_end;
    register_chip_begin("/EPROM/28 pin", "27256", "27256", SIZE_27256);
	add_action(MODULE_READ_ACTION, read_27256);
	add_action(MODULE_PROG_ACTION, prog_27256);
	add_action(MODULE_VERIFY_ACTION, verify_27256);
	add_action(MODULE_TEST_ACTION, test_27256);
    register_chip_end;
    register_chip_begin("/EPROM/28 pin", "27512", "27512", SIZE_27512);
	add_action(MODULE_READ_ACTION, read_27512);
	add_action(MODULE_PROG_ACTION, prog_27512);
	add_action(MODULE_VERIFY_ACTION, verify_27512);
	add_action(MODULE_TEST_ACTION, test_27512);
    register_chip_end;
//    register_chip_begin("/EPROM Electrically Erasable/Winbond", "W27x512", "27512", SIZE_27512);
//	add_action(MODULE_READ_ACTION, read_27512);
//	add_action(MODULE_VERIFY_ACTION, verify_27512);
//	add_action(MODULE_TEST_ACTION, test_27512);
//	add_action(MODULE_ERASE_ACTION, test_27512);
//	add_action(MODULE_PROG_ACTION, erase_27512);
//    register_chip_end;
/* 32 PIN EPROM */
    register_chip_begin("/EPROM/32 pin", "27C010,27C1000,27C1001", "27C010", SIZE_27C010);
	add_action(MODULE_READ_ACTION, read_27C010);
	add_action(MODULE_PROG_ACTION, prog_27C010);
	add_action(MODULE_VERIFY_ACTION, verify_27C010);
	add_action(MODULE_TEST_ACTION, test_27C010);
    register_chip_end;
    register_chip_begin("/EPROM/32 pin", "27C020,27C2000,27C2001", "27C010", SIZE_27C020);
	add_action(MODULE_READ_ACTION, read_27C020);
	add_action(MODULE_PROG_ACTION, prog_27C020);
	add_action(MODULE_VERIFY_ACTION, verify_27C020);
	add_action(MODULE_TEST_ACTION, test_27C020);
    register_chip_end;
    register_chip_begin("/EPROM/32 pin", "27C040,27C4000,27C4001", "27C040", SIZE_27C040);
	add_action(MODULE_READ_ACTION, read_27C040);
	add_action(MODULE_PROG_ACTION, prog_27C040);
	add_action(MODULE_VERIFY_ACTION, verify_27C040);
	add_action(MODULE_TEST_ACTION, test_27C040);
    register_chip_end;
    register_chip_begin("/EPROM/32 pin", "27C080,27C8000,27C8001", "27C080", SIZE_27C080);
	add_action(MODULE_READ_ACTION, read_27C080);
	add_action(MODULE_PROG_ACTION, prog_27C080);
	add_action(MODULE_VERIFY_ACTION, verify_27C080);
	add_action(MODULE_TEST_ACTION, test_27C080);
    register_chip_end;
/* 40 PIN 16bit EPROM */
    register_chip_begin("/EPROM/40 pin", "27C1024,27C210", "27C1024", SIZE_27C1024);
	add_action(MODULE_READ_ACTION, read_27C010);
	add_action(MODULE_PROG_ACTION, prog_27C010);
	add_action(MODULE_VERIFY_ACTION, verify_27C010);
	add_action(MODULE_TEST_ACTION, test_27C010);
    register_chip_end;
    register_chip_begin("/EPROM/40 pin", "27C2048,27C220", "27C1024", SIZE_27C2048);
	add_action(MODULE_READ_ACTION, read_27C020);
	add_action(MODULE_PROG_ACTION, prog_27C020);
	add_action(MODULE_VERIFY_ACTION, verify_27C020);
	add_action(MODULE_TEST_ACTION, test_27C020);
    register_chip_end;
    register_chip_begin("/EPROM/40 pin", "27C4096,27C240,27C4002,27C042", "27C4096", SIZE_27C4096);
	add_action(MODULE_READ_ACTION, read_27C040);
	add_action(MODULE_PROG_ACTION, prog_27C040);
	add_action(MODULE_VERIFY_ACTION, verify_27C040);
	add_action(MODULE_TEST_ACTION, test_27C040);
    register_chip_end;
/* 42 PIN 16bit EPROM */
    register_chip_begin("/EPROM/42 pin", "27C400", "27C400", SIZE_27C400);
	add_action(MODULE_READ_ACTION, read_27C400);
	add_action(MODULE_PROG_ACTION, prog_27C400);
	add_action(MODULE_VERIFY_ACTION, verify_27C400);
	add_action(MODULE_TEST_ACTION, test_27C400);
    register_chip_end;
    register_chip_begin("/EPROM/42 pin", "27C800", "27C400", SIZE_27C800);
	add_action(MODULE_READ_ACTION, read_27C800);
	add_action(MODULE_PROG_ACTION, prog_27C800);
	add_action(MODULE_VERIFY_ACTION, verify_27C800);
	add_action(MODULE_TEST_ACTION, test_27C800);
    register_chip_end;
    register_chip_begin("/EPROM/42 pin", "27C160", "27C400", SIZE_27C160);
	add_action(MODULE_READ_ACTION, read_27C160);
	add_action(MODULE_PROG_ACTION, prog_27C160);
	add_action(MODULE_VERIFY_ACTION, verify_27C160);
	add_action(MODULE_TEST_ACTION, test_27C160);
    register_chip_end;
    register_chip_begin("/EPROM/42 pin", "27C322", "27C322", SIZE_27C322);
	add_action(MODULE_READ_ACTION, read_27C322);
	add_action(MODULE_PROG_ACTION, prog_27C322);
	add_action(MODULE_VERIFY_ACTION, verify_27C322);
	add_action(MODULE_TEST_ACTION, test_27C322);
    register_chip_end;
REGISTER_MODULE_END
