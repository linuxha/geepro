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

#ifndef __PROTOCOLS_H__
#define __PROTOCOLS_H__

#define uWire_ERASE_OPC		0x07
#define uWire_ERAL_OPC		0x04
#define uWire_EWDS_OPC		0x04
#define uWire_EWEN_OPC		0x04
#define uWire_READ_OPC		0x06
#define uWire_WRITE_OPC		0x05
#define uWire_WRAL_OPC		0x04

#define uWire_ERASE_AAA		0x00
#define uWire_ERAL_AAA		0x02
#define uWire_EWDS_AAA		0x00
#define uWire_EWEN_AAA		0x03
#define uWire_READ_AAA		0x00
#define uWire_WRITE_AAA		0x00
#define uWire_WRAL_AAA		0x01


#ifdef __cplusplus
extern "C" {
#endif

/* I2C protocol */
extern void init_i2c();
extern void scl_tik_i2c();
extern void start_i2c();
extern void stop_i2c();
extern void send_bit_i2c( char bit );
extern char get_bit_i2c();
extern void send_byte_i2c( char byte );
extern char recv_byte_i2c();
extern char wait_ack_i2c();

/* µWire protocol */
extern void uWire_init(char org );
extern void uWire_cs( char state );
extern char uWire_bit( char si, int us); // send/receive in full duplex one bit
extern unsigned int uWire_word( unsigned int si, int length, int us); // send/receive in full duplex word
extern void uWire_start(int opcode, int aaa_mask, int adrlen, int address, int us);
extern void uWire_stop(int us);
extern int  uWire_wait_busy(int us, int timeout); // return true if timeout
extern void uWire_erase_cmd( int addr, int alen, int us);
extern void uWire_eral_cmd( int alen, int us);
extern void uWire_ewds_cmd( int alen, int us);
extern void uWire_ewen_cmd( int alen, int us);
extern void uWire_read_cmd( int addr, int alen, int us);
extern void uWire_write_cmd( int addr, int alen, int us);
extern void uWire_wral_cmd( int alen, int us);

/* SPI protocol */
extern char spi_send_data( int data );
extern int  spi_recv_data();
extern char spi_reset();
#ifdef __cplusplus
}
#endif

#endif

