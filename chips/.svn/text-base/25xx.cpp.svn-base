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

#define L2005_SIZE	KB_SIZE( 256 )

#define CMD_WREN	0x06
#define CMD_WRDI	0x04
#define CMD_RDID	0x9f
#define CMD_RDSR	0x05
#define CMD_WRSR	0x01
#define CMD_READ	0x03
#define CMD_ERASE	0x60
#define CMD_PP		0x02

#define WIP	1

/**************************************************************************************
*
* Action functions. 
* Parameters are constants, defined in macro REGISTER_FUNCTION( action_name, name_from_initialisation, action_function_extension, parameters...)
*
*/
/*

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
*/

// Low level
void spi_start(int voltage)
{
    hw_set_clk(0);
    hw_set_cs(1);
    hw_set_di(1);
    hw_set_vpp(0);
    hw_delay(1000);
    hw_set_vcc( voltage ); 
    hw_sw_vcc(1);
    hw_delay(1000);
    hw_set_cs(0);        
    hw_delay(100);
}

void spi_stop()
{
    hw_set_clk(0);
    hw_set_cs(1);
    hw_set_di(1);
    hw_set_vpp(0);
    hw_set_vcc( 0 ); 
    hw_delay(1000);
    hw_sw_vcc(0);
    hw_delay(1000);
}

void spi_send_seq( int time, unsigned int bits, char *data)
{
    unsigned int i;
    
    for(i = 0; i < bits; i++){
	hw_delay( time / 2);
	if( (0x80 >> (i & 0x07)) & data[ i / 8 ]) // get bit
	    hw_set_di(1);
	else
	    hw_set_di(0);	
	hw_delay( time / 2);
	hw_set_clk(1);
	hw_delay( time );
	hw_set_clk(0);
    }
    hw_delay( time );
}

void spi_recv_seq( int time, unsigned int bits, char *data)
{
    unsigned int i;
    char bit;
    
    for(i = 0; i < bits; i++){
	hw_set_clk(1);	
	hw_delay( time );
	bit = 0x80 >> (i & 0x07);
	data[ i / 8] &= ~bit; // clear bit
	if( hw_get_do() ) // set bit if get '1'
	    data[ i / 8] |= bit;
	hw_set_clk(0);	
	hw_delay( time );
    }
}

void send_cmd( char cmd )
{
    hw_delay( 2 );
    hw_set_cs( 0 );
    spi_send_seq(2, 8, &cmd);
    hw_set_cs( 1 );    
    hw_delay( 2 );
}

inline char get_status()
{
    char x;
    x = CMD_RDSR;
    spi_send_seq( 2, 8, &x);
    spi_recv_seq( 2, 8, &x);
    return x;
}

inline void write_page(char *buffer, int page_nb)
{
    // data dor commad Page Program
    buffer[0] = CMD_PP;
    buffer[1] = (page_nb >> 8) & 0xff; // MSB
    buffer[2] = page_nb & 0xff; // LSB
    buffer[3] = 0; // no offset in page
    send_cmd( CMD_WREN ); // send Write Enable command
    hw_set_cs( 0 );        
    spi_send_seq( 2, 8 * 260, buffer);
    hw_set_cs( 1 ); // finish command       
    hw_delay( 100 );
}

void read_page(char *buffer, int page_nb)
{
    // data command for read page
    buffer[0] = CMD_READ;
    buffer[1] = (page_nb >> 8) & 0xff; // MSB
    buffer[2] = page_nb & 0xff; // LSB
    buffer[3] = 0; // no offset in page
    hw_set_cs( 0 );        
    spi_send_seq( 2, 8 * 4, buffer);
    spi_recv_seq( 2, 8 * 256, buffer + 4);
    hw_set_cs( 1 ); // finish command       
    hw_delay( 2 );
}

/**************************************************************************************/

void sign_25lcxx()
{
    char text[1024];
    char cmd[3];    

    spi_start( 33 );
    cmd[0] = CMD_RDID;
    spi_send_seq( 100, 8, cmd);
    cmd[0] = 0; cmd[1] = 0; cmd[2] = 0;
    spi_recv_seq( 10, 24, cmd);
    spi_stop();
    finish_action();    

    sprintf(
	text, "[IF][TEXT]Chip signature: 0x%X%X%X%X%X%X\nDevice name:%s[/TEXT][BR]OK",
	to_hex(cmd[0] , 1), to_hex(cmd[0] , 0), 
	to_hex(cmd[1] , 1), to_hex(cmd[1] , 0), 
	to_hex(cmd[2] , 1), to_hex(cmd[2] , 0), 
	take_signature_name( (cmd[2] << 16) + (cmd[1] << 8) + cmd[0])
    );
    show_message(0, text, NULL, NULL);
}

void erase_25lcxx()
{
    int timeout;
    char cmd[3];    
    unsigned long *lb;
    
    lb = checkbox(
	"[TITLE]Erase Chip[/TITLE][TYPE:WN]"
	"[CB:1:0: Tick to clear whole chip.]"
    );

    if( !lb ) return;

    spi_start( 33 );

    // send command wren
    cmd[0] = CMD_WREN;
    spi_send_seq( 100, 8, cmd);
    hw_set_cs( 1 );
    hw_delay(100);
    hw_set_cs( 0 );
    // send command erase
    cmd[0] = CMD_ERASE;
    spi_send_seq( 100, 8, cmd);
    hw_set_cs( 1 );
    hw_delay(100);
    hw_set_cs( 0 );
    
    timeout = 0;
    progress_loop(timeout, 20, "Erasing"){
	hw_delay( 500000 );
	cmd[0] = CMD_RDSR;
	spi_send_seq( 100, 8, cmd);
	spi_recv_seq( 10, 8, cmd);
	break_if( !(cmd[0] & WIP) ); // wait to clear Write in Progress
    }
    ERROR_VAL = 0;
    spi_stop();
    finish_action();    

    if( timeout >= 19 )
	show_message(0, "[ER][TEXT] Chip erase fail. \n Timeout occured.[/TEXT][OK]OK", NULL, NULL);
    else
	show_message(0, "[IF][TEXT] Chip erased succesfully.[/TEXT][OK]OK", NULL, NULL);
}

void read_25lcxx(unsigned int dev_size)
{
    int page, pages, i;
    char buff[ 260 ];

    spi_start( 33 );
    pages = dev_size / 256;
    progress_loop(page, pages, "Reading ..."){
	read_page( buff, page );
	for(i = 0; i < 256; i++)
		put_buffer( i + page * 256, *(buff + 4 + i) );
    }

    spi_stop();
    finish_action();
}

void verify_25lcxx(unsigned int dev_size)
{
    char text[1024];
    char ch = 0, bf = 0;    
    char buff[ 260 ]; // 1 command byte + 3 address byte + 256 page bytes
    int i, page, pages, erraddr;

    spi_start( 33 );

    erraddr = -1;
    pages = dev_size / 256;
    i = 0;
    progress_loop(page, pages, "Verify ..."){
	// verify page    
	read_page( buff, page );
	for(i = 0; i < 256; i++){ 
	    ch = buff[i + 4];	// chip data
	    bf = get_buffer( i + page * 256); // buffer data
	    erraddr = page * 256 + i;
	    if( ch != bf ){
	        i = 300;
	        break;
	    }
	}
	break_if( i == 300 );
    }

    spi_stop();
    finish_action();

    text[0] = 0;
    if( ERROR_VAL )
	    sprintf( text, "[ER][TEXT] Chip verify fail. \nDiffer in location 0x%x\n0x%x(chip) != 0x%x(buffer)[/TEXT][OK]OK", erraddr, ch, bf);

    show_message(0, ERROR_VAL ? text: "[IF][TEXT] Memory and buffer are consitent[/TEXT][BR]OK", NULL, NULL);    
    ERROR_VAL = 0;
}

void test_25lcxx(unsigned int dev_size)
{
    char text[1024];
    char ch = 0;    
    char buff[ 260 ]; // 1 command byte + 3 address byte + 256 page bytes
    int i, page, pages, erraddr;

    spi_start( 33 );

    erraddr = -1;
    pages = dev_size / 256;
    i = 0;
    progress_loop(page, pages, "Test blank ..."){
	// verify page    
	read_page( buff, page );
	for(i = 0; i < 256; i++){ 
	    ch = buff[i + 4];	// chip data
	    erraddr = page * 256 + i;
	    if( ch != 0xff ){
	        i = 300;
	        break;
	    }
	}
	break_if( i == 300 );
    }

    spi_stop();
    finish_action();

    text[0] = 0;
    if( ERROR_VAL )
	    sprintf( text, "[ER][TEXT] Chip is dirty at location 0x%x\n0x%x(chip) != 0xff [/TEXT][OK]OK", erraddr, ch);

    show_message(0, ERROR_VAL ? text: "[IF][TEXT] Memory is clear[/TEXT][BR]OK", NULL, NULL);    
    ERROR_VAL = 0;
}

void write_25lcxx(unsigned int dev_size)
{
    char text[1024];
    char ch, bf;    
    char buff[ 260 ]; // 1 command byte + 3 address byte + 256 page bytes
    unsigned long *lb;
    int i, timeout, page, pages, erraddr;

    lb = checkbox(
	"[TITLE]Write Chip[/TITLE][TYPE:WN]"
	"[CB:1:0: Tick to program whole chip.]"
//	"[CB:2:1: Erase chip.]"
//	"[CB:4:1: Verify chip.]"
    );
    if( !lb ) return;
//    if( *lb & 2 ) 
//	erase_25lcxx();

    spi_start( 33 );
    i = 0;
    erraddr = -1;
    pages = dev_size / 256;
    progress_loop(page, pages, "Writing ..."){
	// prepare data. First 4 bytes are for command PP
	for( i = 0; i < 256; i++) buff[ i + 4] = get_buffer( i + page * 256);
	// send data to chip
	write_page( buff, page );
	// wait for command complete
	hw_set_cs( 0 ); 
	timeout = 100;
	do{
	    hw_delay( 100 );
	    ch = get_status();
	    timeout--;
	}while( timeout && (ch & WIP));
	break_if( timeout == 0 );
	// verify page    
	read_page( buff, page );
//	for(i = 0; i < 256; i++){ 
//	    ch = buff[i + 4];	// chip data
//	    bf = get_buffer( i + page * 256); // buffer data
//	    erraddr = page * 256 + i;
//	    if( ch != bf ){
//	        i = 300;
//	        break;
//	    }
//	}
	break_if( i == 300 );
    }
    spi_stop();
    // Error message
    if( ERROR_VAL ){
	if( i != 300)
	    show_message(0, "[ER][TEXT] Chip program fail. \n Write timeout occured.[/TEXT][OK]OK", NULL, NULL);
	else {
	    sprintf( text, "[ER][TEXT] Chip program fail. \nDiffer in location 0x%x: 0x%x(chip) != 0x%x(buffer)[/TEXT][OK]OK", erraddr, ch, bf);
	    show_message(0, text, NULL, NULL);	
	}
    } else
	show_message(0, "[IF][TEXT] Chip programed succesfully.[/TEXT][OK]OK", NULL, NULL);
    finish_action();
//    if( *lb & 4 )
//	verify_25lcxx( dev_size );
}

/*********************************************************************************************
*
* Registering callback functions. It create callback function, that are invoked by action buttons.
* You can ommit that section and indirectly call action function, but it gives you possibility to add additional parameters, and
* assign one action function to different, but similar chips.
*
*/

REGISTER_FUNCTION( read,  25xx, 25lcxx, L2005_SIZE );
REGISTER_FUNCTION( write, 25xx, 25lcxx, L2005_SIZE );
REGISTER_FUNCTION( verify, 25xx, 25lcxx, L2005_SIZE );
REGISTER_FUNCTION( sign, 25xx, 25lcxx);
REGISTER_FUNCTION( erase, 25xx, 25lcxx);
REGISTER_FUNCTION( test, 25xx, 25lcxx, L2005_SIZE);

/******************************************************************************************************
*
* Chip plugin initialisation. It defines actions possible on the chip, buffer size and callback funcion for action on given chip. It also sets
* menu path, chip name and chip family. Chip family is an identifier used by device driver to identify chip (e.g. show proper switch settings, picture etc).
* Device actions for family is described in <device>.xml file.
*
*/
REGISTER_MODULE_BEGIN(25xx)

    register_chip_begin("/Serial EEPROM/25Lxx", "MX25L2005", "25Cxx", L2005_SIZE);
	add_action(MODULE_READ_ACTION, read_25xx);
	add_action(MODULE_PROG_ACTION, write_25xx);
	add_action(MODULE_VERIFY_ACTION, verify_25xx);
	add_action(MODULE_TEST_ACTION, test_25xx);
	add_action(MODULE_SIGN_ACTION, sign_25xx);
	add_action(MODULE_ERASE_ACTION, erase_25xx);
    register_chip_end;

REGISTER_MODULE_END
