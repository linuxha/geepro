/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2011 Krzysztof Komarnicki
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
#include "../drivers/hwdriver.h"
#include "protocols.h"

//#define DEBUG

/*******************************************************************************************************************
	    I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C I2C
********************************************************************************************************************/


#define TI 16
#define TIMEOUT	100

void init_i2c()
{
    hw_set_sda(1);
    hw_set_scl(1);
    hw_set_hold(0);
    hw_delay( TI );
    hw_sw_vcc(1);
    hw_delay( 10 * TI );  // time for POR (Power On Ready)
}

void scl_tik_i2c()
{
    hw_delay( TI / 2 );
    hw_set_scl(1);
    hw_delay( TI );
    hw_set_scl(0);
    hw_delay( TI / 2 );
}

void start_i2c()
{
    hw_set_sda(1);
    hw_delay( TI / 2 );    
    hw_set_scl(1);
    hw_delay( TI / 2 );    

    hw_set_sda(0);
    hw_delay( TI / 2 );
    hw_set_scl(0);
    hw_delay( TI / 2 );
}

void stop_i2c()
{
    hw_set_sda(0);
    hw_delay( TI );
    hw_set_scl(0);
    hw_delay( TI );
    hw_set_scl(1);
    hw_delay( TI );
    hw_set_sda(1);
    hw_delay( TI );
}

void send_bit_i2c( char bit )
{
    hw_set_sda(bit);
    scl_tik_i2c();
}

char get_bit_i2c()
{
    char b;
    
    hw_set_sda( 1 );
    hw_set_scl(1);    
    hw_delay( TI );    

    b = hw_get_sda();
    hw_delay( TI);    
    hw_set_scl(0);
    return b;
}

void send_byte_i2c( char byte )
{
    int i;
    for( i = 0x80; i; i >>= 1 ) send_bit_i2c( (byte & i) ? 1:0 );
}

char recv_byte_i2c()
{
    int i;
    char b = 0;

    for( i = 8; i; i-- ){
	b <<= 1;
	b |= get_bit_i2c();
    }

    return b;
}

char wait_ack_i2c()
{
    int i;
    char b;
    
    hw_set_sda( 1 );		// release SDA
    hw_delay( TI / 2 );    	// wait for stabilize
    hw_set_scl(1);    		// SCL = 1
    for( b = 1, i = 0; (i < TIMEOUT) && b; i++){
	hw_delay( TI / 2 );    	// wait for SDA = 0
	b = hw_get_sda();
    }
    hw_set_scl(0);	
    return (b != 0) ? 1:0;
}

/*******************************************************************************************************************
        µWire µWire µWire µWire µWire µWire µWire µWire µWire µWire µWire µWire µWire µWire µWire µWire µWire
********************************************************************************************************************/

void uWire_init(char org )
{
//    hw_set_org( org == 16 ? 1:0);
    hw_set_cs( 0 );
    hw_set_clk( 0 );
    hw_set_di( 0 );
//    hw_set_do( 0 );
    hw_ms_delay( 100 );    
    hw_sw_vcc( 1 );
    hw_ms_delay( 100 );
}

void uWire_cs( char state )
{
    hw_set_cs( state );
}

// send/receive in full duplex one bit
char uWire_bit( char si, int us)
{
    hw_set_di( si ? 1 : 0);
    hw_us_delay(us / 2);
    hw_set_clk( 1 );    
    hw_us_delay( us );
    hw_set_clk( 0 );
    hw_us_delay(us / 2);
    return hw_get_do() ? 1 : 0;
}

// send/receive in full duplex word
unsigned int uWire_word( unsigned int si, int length, int us)
{
    int data;

    for(data = 0; length; length--){
		    data |= uWire_bit((si >> (length - 1)) & 1, us) << (length - 1);
#ifdef DEBUG
	printf("%i -> %i\n", length - 1, (si >> (length - 1)) & 1);
#endif
    }
#ifdef DEBUG
	printf("---\n");
#endif
    return data;
}

void uWire_start(int opcode, int aaa_mask, int adrlen, int address, int us)
{
    uWire_cs( 1 );
    uWire_word(opcode, 3, us);
    uWire_word((aaa_mask << (adrlen - 2)) | address, adrlen, us);
}

void uWire_stop(int us)
{
    hw_set_cs( 0 );
    hw_set_clk( 0 );
    hw_set_di( 0 );
    hw_set_cs( 0 );    
    hw_us_delay( us );
}

// return true if timeout
int uWire_wait_busy(int us, int timeout)
{
    hw_set_cs( 1 );
    for(; timeout; timeout--)
	if( hw_get_do() ) return 0;
	hw_us_delay( us );
    return 1;
}

void uWire_erase_cmd( int addr, int alen, int us)
{
    uWire_start( uWire_ERASE_OPC, uWire_ERASE_AAA, alen, addr, us );
    uWire_stop( us );
}

void uWire_eral_cmd( int alen, int us)
{
    uWire_start( uWire_ERAL_OPC, uWire_ERAL_AAA, alen, 0, us );
    uWire_stop( us );
}

void uWire_ewds_cmd( int alen, int us)
{
    uWire_start( uWire_EWDS_OPC, uWire_EWDS_AAA, alen, 0, us );
    uWire_stop( us );
}

void uWire_ewen_cmd( int alen, int us)
{
    uWire_start( uWire_EWEN_OPC, uWire_EWEN_AAA, alen, 0, us );
    uWire_stop( us );
}

void uWire_read_cmd( int addr, int alen, int us)
{
    uWire_start( uWire_READ_OPC, uWire_READ_AAA, alen, addr, us );
}

void uWire_write_cmd( int addr, int alen, int us)
{
    uWire_start( uWire_WRITE_OPC, uWire_WRITE_AAA, alen, addr, us );
}

void uWire_wral_cmd( int alen, int us)
{
    uWire_start( uWire_WRAL_OPC, uWire_WRAL_AAA, alen, 0, us );
}

/*******************************************************************************************************************
		 SPI SPI  SPI SPI  SPI SPI  SPI SPI  SPI SPI  SPI SPI  SPI SPI  SPI SPI  SPI SPI
********************************************************************************************************************/

