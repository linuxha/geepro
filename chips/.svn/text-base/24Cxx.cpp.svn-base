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

/**********************************************************************************************
*
* VERY IMPORTANT !!!!
* It initialize and create local variables for plugin, used later by macros.
*
*/

MODULE_IMPLEMENTATION

/**********************************************************************************************
*
* Device size definition section
*
*/

#define C01_SIZE	128
#define C21_SIZE	128
#define C02_SIZE	256
#define PCF8582_SIZE	256
#define C04_SIZE	512
#define C08_SIZE	KB_SIZE( 1 )
#define C16_SIZE	KB_SIZE( 2 )
#define C32_SIZE	KB_SIZE( 4 )
#define C64_SIZE	KB_SIZE( 8 )
#define C128_SIZE	KB_SIZE( 16 )
#define C256_SIZE	KB_SIZE( 32 )
#define C512_SIZE	KB_SIZE( 64 )

/**********************************************************************************************
*
* Some defines used in code
*
*/
#define TI 			16
#define TIMEOUT			100
#define MEMO_24CXX_DEV_ADDR	0xa0	// Device internal address for 24Cxx

/**********************************************************************************************
*
* Some common function, used by action functions.
*
*/

char devsel_24Cxx( char rw, char addr )
{
    send_byte_i2c( 0xa0 | (rw & 1) | ((addr << 1) & 0x0e));
    if( wait_ack_i2c() ) return 1;
    return 0;
}

char transm_seq_hdr_24Cxx(int addr, char mode, char addr_mode) // mode = 0 -> write, mode = 1 -> read
{
    unsigned int  msb = (addr >> 8) & 0xff;
    unsigned char page_nb = addr_mode ? 0 : msb & 0x07;
    unsigned char addr_byte = addr & 0xff;
    int timeout;

    // send devsel WRITE
    timeout = TIMEOUT;
    do{
	start_i2c();
	if(--timeout == 0) return 1;
    }while(devsel_24Cxx( 0, page_nb ));

    // send address
    if( addr_mode ){
	send_byte_i2c( msb );    
	if( wait_ack_i2c() ) return 2;
    }
    send_byte_i2c( addr_byte & 0xff);
    if( wait_ack_i2c() ) return 2;

    if( mode ){ // skip for WRITE
	// repeated start + send devsel READ
	start_i2c();
	if(devsel_24Cxx( 1, page_nb )) return 3;
    }
    return 0;
}

int rd_byte_24Cxx(int addr, char addr_mode)
{
    char data;
    if( transm_seq_hdr_24Cxx( addr, 1, addr_mode) ) return -1;
    data = recv_byte_i2c();
    stop_i2c();    
    return data & 0xff;
}

char write_byte_24Cxx( int addr, char byte, char addr_mode)
{
    if( transm_seq_hdr_24Cxx( addr, 0, addr_mode) ) return 1;
    send_byte_i2c( byte );
    if( wait_ack_i2c() ) return 1;
    stop_i2c();
    return 0;
}

/**************************************************************************************
*
* Action functions. 
* Parameters are constants, defined in macro REGISTER_FUNCTION( action_name, name_from_initialisation, action_function_extension, parameters...)
*
*/

void read_24Cxx(unsigned int dev_size, char addr_mode)
{
    int i, data;

    init_i2c();
    progress_loop(i, dev_size, "Reading ..."){
	data = rd_byte_24Cxx(i, addr_mode);
	break_if( data < 0 );
	put_buffer( i, data );	
    }
    finish_action();
}

void verify_24Cxx(unsigned int dev_size, char addr_mode)
{
    int i, rdata = 0, bdata = 0;
    char text[256];

    init_i2c();
    progress_loop(i, dev_size, "Verify ..."){
	bdata = get_buffer( i );
	rdata = rd_byte_24Cxx(i, addr_mode);
	break_if( rdata < 0 || ( (bdata & 0xff) != (rdata & 0xff) ) );
    }
    finish_action();

    text[0] = 0;
    if( ERROR_VAL )
	sprintf(text, "[WN][TEXT] Memory and buffer differ !!!\n Address = 0x%X\nBuffer=0x%X, Device=0x%X[/TEXT][BR]OK", i, bdata & 0xff, rdata & 0xff);
    if( rdata >= 0 ){
	show_message(0, ERROR_VAL ? text: "[IF][TEXT] Memory and buffer are consitent[/TEXT][BR]OK", NULL, NULL);    
	ERROR_VAL = 0;
    }
}

void write_24Cxx(int dev_size, char addr_mode, char n)
{
    int i;
    unsigned long *lb;
    
    lb = checkbox(
	"[TITLE]Writing chip[/TITLE][TYPE:QS]"
	"[CB:2:0: Are you sure ? (Tick if Yes)]"
	"[CB:1:1: Verify after process]"
    );

    if( !lb ) return; // resignation by button
    if( !(*lb & 2) ) return; // Not checked
    
    init_i2c();
    hw_set_hold( n );
    progress_loop(i, dev_size, "Writing ...")
	    break_if( write_byte_24Cxx(i, get_buffer(i), addr_mode) );
    // wait to end of last write cycle
    if( transm_seq_hdr_24Cxx( 0, 1, 0) ) ERROR_VAL = 1;
    stop_i2c();        

    finish_action();
    
    if( (*lb & 1) && !ERROR_VAL) 
	verify_24Cxx( dev_size, addr_mode);
}

/*********************************************************************************************
*
* Registering callback functions. It create callback function, that are invoked by action buttons.
* You can ommit that section and indirectly call action function, but it gives you possibility to add additional parameters, and
* assign one action function to different, but similar chips.
*
*/

REGISTER_FUNCTION( read,  24C01, 24Cxx, C01_SIZE, 0 );
REGISTER_FUNCTION( write, 24C01, 24Cxx, C01_SIZE, 0, 0 );
REGISTER_FUNCTION( verify, 24C01, 24Cxx, C01_SIZE, 0 );
REGISTER_FUNCTION( write, 24C21, 24Cxx, C01_SIZE, 0, 1 );

REGISTER_FUNCTION( read,  24C02, 24Cxx, C02_SIZE, 0 );
REGISTER_FUNCTION( write, 24C02, 24Cxx, C02_SIZE, 0, 0 );
REGISTER_FUNCTION( verify, 24C02, 24Cxx, C02_SIZE, 0 );
REGISTER_FUNCTION( write, PCF_8582, 24Cxx, C02_SIZE, 0, 1 );

REGISTER_FUNCTION( read,  24C04, 24Cxx, C04_SIZE, 0 );
REGISTER_FUNCTION( write, 24C04, 24Cxx, C04_SIZE, 0, 0 );
REGISTER_FUNCTION( verify, 24C04, 24Cxx, C04_SIZE, 0 );

REGISTER_FUNCTION( read,  24C08, 24Cxx, C08_SIZE, 0 );
REGISTER_FUNCTION( write, 24C08, 24Cxx, C08_SIZE, 0, 0 );
REGISTER_FUNCTION( verify, 24C08, 24Cxx, C08_SIZE, 0 );

REGISTER_FUNCTION( read,  24C16, 24Cxx, C16_SIZE, 0 );
REGISTER_FUNCTION( write, 24C16, 24Cxx, C16_SIZE, 0, 0 );
REGISTER_FUNCTION( verify, 24C16, 24Cxx, C16_SIZE, 0 );

REGISTER_FUNCTION( read,  24C32, 24Cxx, C32_SIZE, 1 );
REGISTER_FUNCTION( write, 24C32, 24Cxx, C32_SIZE, 1, 0 );
REGISTER_FUNCTION( verify, 24C32, 24Cxx, C32_SIZE, 1  );

REGISTER_FUNCTION( read,  24C64, 24Cxx, C64_SIZE, 1  );
REGISTER_FUNCTION( write, 24C64, 24Cxx, C64_SIZE, 1, 0 );
REGISTER_FUNCTION( verify, 24C64, 24Cxx, C64_SIZE, 1  );

REGISTER_FUNCTION( read,  24C128, 24Cxx, C128_SIZE, 1  );
REGISTER_FUNCTION( write, 24C128, 24Cxx, C128_SIZE, 1, 0 );
REGISTER_FUNCTION( verify, 24C128, 24Cxx, C128_SIZE, 1  );

REGISTER_FUNCTION( read,  24C256, 24Cxx, C256_SIZE, 1  );
REGISTER_FUNCTION( write, 24C256, 24Cxx, C256_SIZE, 1, 0 );
REGISTER_FUNCTION( verify, 24C256, 24Cxx, C256_SIZE, 1  );

REGISTER_FUNCTION( read,  24C512, 24Cxx, C512_SIZE, 1  );
REGISTER_FUNCTION( write, 24C512, 24Cxx, C512_SIZE, 1, 0 );
REGISTER_FUNCTION( verify, 24C512, 24Cxx, C512_SIZE, 1  );

/******************************************************************************************************
*
* Chip plugin initialisation. It defines actions possible on the chip, buffer size and callback funcion for action on given chip. It also sets
* menu path, chip name and chip family. Chip family is an identifier used by device driver to identify chip (e.g. show proper switch settings, picture etc).
* Device actions for family is described in <device>.xml file.
*
*/
REGISTER_MODULE_BEGIN(24Cxx)

    register_chip_begin("/Serial EEPROM/24Cxx", "24C01", "24Cxx", C01_SIZE);
	add_action(MODULE_READ_ACTION, read_24C01);
	add_action(MODULE_PROG_ACTION, write_24C01);
	add_action(MODULE_VERIFY_ACTION, verify_24C01);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/24Cxx", "24C21", "24Cxx", C21_SIZE);
	add_action(MODULE_READ_ACTION, read_24C01);
	add_action(MODULE_PROG_ACTION, write_24C21);
	add_action(MODULE_VERIFY_ACTION, verify_24C01);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/24Cxx", "24C02", "24Cxx", C02_SIZE);
	add_action(MODULE_READ_ACTION, read_24C02);
	add_action(MODULE_PROG_ACTION, write_24C02);
	add_action(MODULE_VERIFY_ACTION, verify_24C02);
    register_chip_end;
    
    register_chip_begin("/Serial EEPROM/24Cxx", "24C04", "24Cxx", C04_SIZE);
	add_action(MODULE_READ_ACTION, read_24C04);
	add_action(MODULE_PROG_ACTION, write_24C04);
	add_action(MODULE_VERIFY_ACTION, verify_24C04);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/24Cxx", "24C08", "24Cxx", C08_SIZE);
	add_action(MODULE_READ_ACTION, read_24C08);
	add_action(MODULE_PROG_ACTION, write_24C08);
	add_action(MODULE_VERIFY_ACTION, verify_24C08);
    register_chip_end;
    
    register_chip_begin("/Serial EEPROM/24Cxx", "24C16", "24Cxx", C16_SIZE);
	add_action(MODULE_READ_ACTION, read_24C16);
	add_action(MODULE_PROG_ACTION, write_24C16);
	add_action(MODULE_VERIFY_ACTION, verify_24C16);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/24Cxx", "24C32", "24Cxx", C32_SIZE);
	add_action(MODULE_READ_ACTION, read_24C32);
	add_action(MODULE_PROG_ACTION, write_24C32);
	add_action(MODULE_VERIFY_ACTION, verify_24C32);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/24Cxx", "24C64", "24Cxx", C64_SIZE);
	add_action(MODULE_READ_ACTION, read_24C64);
	add_action(MODULE_PROG_ACTION, write_24C64);
	add_action(MODULE_VERIFY_ACTION, verify_24C64);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/24Cxx", "24C128", "24Cxx", C128_SIZE);
	add_action(MODULE_READ_ACTION, read_24C128);
	add_action(MODULE_PROG_ACTION, write_24C128);
	add_action(MODULE_VERIFY_ACTION, verify_24C128);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/24Cxx", "24C256", "24Cxx", C256_SIZE);
	add_action(MODULE_READ_ACTION, read_24C256);
	add_action(MODULE_PROG_ACTION, write_24C256);
	add_action(MODULE_VERIFY_ACTION, verify_24C256);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/24Cxx", "24C512", "24Cxx", C512_SIZE);
	add_action(MODULE_READ_ACTION, read_24C512);
	add_action(MODULE_PROG_ACTION, write_24C512);
	add_action(MODULE_VERIFY_ACTION, verify_24C512);
    register_chip_end;

    register_chip_begin("/Serial EEPROM/PCF85xx", "PCF8582", "24Cxx", PCF8582_SIZE);
	add_action(MODULE_READ_ACTION, read_24C02);
	add_action(MODULE_PROG_ACTION, write_PCF_8582);
	add_action(MODULE_VERIFY_ACTION, verify_24C02);
    register_chip_end;

REGISTER_MODULE_END
