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

// AT90S1200 Commands
#define AT90S1200_CHIP_ERASE		0x80
#define AT90S1200_WRITE_FUSE_BITS       0x40
#define AT90S1200_WRITE_LB              0x20
#define AT90S1200_WRITE_FLASH           0x10
#define AT90S1200_WRITE_EEPROM          0x11
#define AT90S1200_READ_SIGNATURE        0x08
#define AT90S1200_READ_LB               0x04
#define AT90S1200_READ_FLASH            0x02
#define AT90S1200_READ_EEPROM           0x03
#define A7_A8_A9_MASK			0x380

// XA
#define AT90S1200_XA_LOAD_CMD		0x04
#define AT90S1200_XA_LOAD_DATA		0x01
#define AT90S1200_XA_LOAD_FLASH		0x00
#define AT90S1200_XA_NO_ACTION		0x05
// BS
#define AT90S1200_BS_H			0x02


#define SIZE_AT89C1051		1024
#define SIZE_AT89C2051		2048
#define SIZE_AT89C4051		4096
#define SIZE_AT89C51		4096
#define SIZE_AT89C52		8192
#define SIZE_AT90S1200_FLASH	1024
#define SIZE_AT90S1200_EEPROM	64
#define SIZE_AT90S1200		SIZE_AT90S1200_FLASH
#define SIZE_AT90S2313_FLASH	2048
#define SIZE_AT90S2313_EEPROM	128
#define SIZE_AT90S2313		SIZE_AT90S2313_FLASH
#define SIZE_ATtiny2313_FLASH	2048
#define SIZE_ATtiny2313_EEPROM	128
#define SIZE_ATtiny2313		SIZE_ATtiny2313_FLASH

#define SIZE_I8751		4096
#define SIZE_I8752		8192

//AT89C5x Mode
#define AT89C5x_READ_MODE	(0x0c << 15)
#define AT89C5x_WRITE_MODE	(0x0e << 15)
#define AT89C5x_LB1_MODE	(0x0f << 15)
#define AT89C5x_LB2_MODE	(0x03 << 15)
#define AT89C5x_LB3_MODE	(0x05 << 15)
#define AT89C5x_ERASE_MODE	(0x01 << 15)
#define AT89C5x_SIGN_MODE	(0x00 << 15)
#define AT89C5x_P27_H_MODE	(0x02 << 15)
#define AT89C5x_MODE_MASK	0x78000

// AT89Cx051 modes
#define AT89Cx051_WR_MODE	(0x0e << 6)
#define AT89Cx051_RD_MODE	(0x0a << 6)
#define AT89Cx051_LB1_MODE	(0x0f << 6)
#define AT89Cx051_LB2_MODE	(0x05 << 6)
#define AT89Cx051_ERA_MODE	(0x01 << 6)
#define AT89Cx051_SIGN_MODE	(0x00 << 6)
#define AT89Cx051_MODE_MASK	0x003c0

// AT89Cx051 adapter mux 
#define AT89Cx051_X1_MUX	(0x03 << 15)
#define AT89Cx051_X1_OFF_MUX	(0x00 << 15)
#define AT89Cx051_PROG_MUX	(0x02 << 17)
#define AT89Cx051_RST_MUX	(0x01 << 17)
#define AT90Sx_INT1_MUX		(0x03 << 17)
#define AT89Cx051_MUX_MASK	0x78000

// stan linii RESET
#define AT89Cx051_RST_LOW	0
#define AT89Cx051_RST_HIGH	1
#define AT89Cx051_RST_VPP	2

#define I87C5x_LB_MODE		(0x0f << 15)

char at89cXX_lb = 0;	/* default mode for LB1, LB2 */
int addr_state = 0;

// Low level access to adapter
//-----------------------------------------------------------------------------------------------------
void set_AT89Cx051_mode(int mode) // Sets mode pins: P3.3, P3.4, P3.5, P3.7
{
    addr_state &= ~AT89Cx051_MODE_MASK;
    addr_state |= mode & AT89Cx051_MODE_MASK;
    set_address(addr_state);  /* ustawienie linii selekcji stanu: A6-P3.3, A7-P3.5, A8-P3.4, A9-P3.7*/
}

void AT89Cx051_pulse(int duration) // generate programing/address pulse at S4
{
    ce(0, duration);
    ce(1,1);
}

void AT89Cx051_mux(int mode) // Sets A15,A16,A17,A18
{
    addr_state &= ~AT89Cx051_MUX_MASK;
    addr_state |= mode & AT89Cx051_MUX_MASK;
    set_address(addr_state);
}

void AT89Cx051_RST(char state )
{
    switch(state){
	case AT89Cx051_RST_LOW:	 hw_sw_vpp(0); ce(0, 1); AT89Cx051_mux(AT89Cx051_RST_MUX);  break;
	case AT89Cx051_RST_HIGH: hw_sw_vpp(0); ce(1, 1); AT89Cx051_mux(AT89Cx051_RST_MUX);  break;
	case AT89Cx051_RST_VPP:  ce(1, 1); AT89Cx051_mux(AT89Cx051_PROG_MUX); hw_sw_vpp(1); break;
    }        
}

unsigned char AT89Cx051_transcode0(unsigned char data)
{
    char tmp;
    tmp  = (data << 4) & 0xc0;
    tmp |= (data >> 7) & 0x01;
    tmp |= (data >> 5) & 0x02;
    tmp |= (data >> 2) & 0x08;
    tmp |= (data << 1) & 0x20;
    tmp |= (data << 3) & 0x10;
    tmp |= (data << 2) & 0x04;
    return tmp;
}

unsigned char AT89Cx051_transcode1(unsigned char data)
{
    char tmp;
    tmp  = (data >> 4) & 0x0c;
    tmp |= (data >> 1) & 0x10;
    tmp |= (data >> 3) & 0x02;
    tmp |= (data << 2) & 0x20;
    tmp |= (data >> 2) & 0x01;
    tmp |= (data << 5) & 0x40;
    tmp |= (data << 7) & 0x80;
    return tmp;
}

unsigned char AT89Cx051_get_data()
{
    return AT89Cx051_transcode0(hw_get_data());
}

void AT89Cx051_put_data(char data)
{
    set_data( AT89Cx051_transcode1( data & 0xff ));
}

//----------------------------------------------------------------------------------------------------------

int test_blank_AT89Cx051(int size, char mode)
{
    int addr = 0;
    int tmp = 0;

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    AT89Cx051_RST(AT89Cx051_RST_LOW);    // RST to GND
    AT89Cx051_mux(AT89Cx051_X1_OFF_MUX); // X1  to GND
    hw_delay(1000); // 1ms
    AT89Cx051_RST(AT89Cx051_RST_HIGH);    // RST to H, clear internal address counter
    hw_delay(1000); // 1ms
    set_AT89Cx051_mode(AT89Cx051_RD_MODE); // set mode
    hw_delay(1000); // 1ms    
    AT89Cx051_mux(AT89Cx051_X1_MUX); // X1 as pulse
    progress_loop(addr, size, "Checking blank"){
	hw_delay(100);
	tmp = AT89Cx051_get_data();
	break_if( tmp != 0xff);
	AT89Cx051_pulse( 150 );
    }
    if(!mode) show_message(0,(tmp == 0xff) ? "[IF][TEXT]Flash is empty[/TEXT][BR]OK" : "[WN][TEXT]Flash is dirty !!![/TEXT][BR]OK",NULL, NULL);
    set_address(0);
    finish_action();
    return tmp != 0xff;
}

void read_AT89Cx051(int size)
{
    int addr = 0;

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    AT89Cx051_RST(AT89Cx051_RST_LOW);    // RST to GND
    AT89Cx051_mux(AT89Cx051_X1_OFF_MUX); // X1  to GND
    hw_delay(1000); // 1ms
    AT89Cx051_RST(AT89Cx051_RST_HIGH);    // RST to H, clear internal address counter
    hw_delay(1000); // 1ms
    set_AT89Cx051_mode(AT89Cx051_RD_MODE); // set mode
    hw_delay(1000); // 1ms    
    AT89Cx051_mux(AT89Cx051_X1_MUX); // X1 as pulse
    progress_loop(addr, size, "Reading"){
	hw_delay(100);
	put_buffer( addr, AT89Cx051_get_data());
	AT89Cx051_pulse( 150 );
    }
    set_address(0);
    finish_action();
}

void sign_AT89Cx051(int size)
{
    int addr = 0;
    int signature = 0;
    char text[256];

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilisation
    AT89Cx051_RST(AT89Cx051_RST_LOW);    // RST to GND
    AT89Cx051_mux(AT89Cx051_X1_OFF_MUX); // X1  to GND
    hw_delay(1000); // 1ms
    AT89Cx051_RST(AT89Cx051_RST_HIGH);    // RST to H, clear internal address counter
    hw_delay(1000); // 1ms
    set_AT89Cx051_mode(AT89Cx051_SIGN_MODE); // set mode
    hw_delay(1000); // 1ms    
    AT89Cx051_mux(AT89Cx051_X1_MUX); // X1 as pulse
    progress_loop(addr, 3, "Reading"){
	hw_delay(100);
	signature |= AT89Cx051_get_data() << (addr * 8);
	AT89Cx051_pulse( 150 );
    }
    set_address(0);
    finish_action();
    sprintf(
	text, "[IF][TEXT]Chip signature: 0x%X%X%X%X%X%X\n%s[/TEXT][BR]OK", 
	to_hex(signature, 5),to_hex(signature, 4),  
	to_hex(signature, 3),to_hex(signature, 2),  
	to_hex(signature, 1),to_hex(signature, 0), 
	take_signature_name( signature )
    );
    show_message(0, text,NULL,NULL);
}

void verify_AT89Cx051(int size, char silent)
{
    int addr = 0;
    char rdata = 0, wdata = 0;
    char text[256];

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    AT89Cx051_RST(AT89Cx051_RST_LOW);    // RST to GND
    AT89Cx051_mux(AT89Cx051_X1_OFF_MUX); // X1  to GND
    hw_delay(1000); // 1ms
    AT89Cx051_RST(AT89Cx051_RST_HIGH);    // RST to H, clear internal address counter
    hw_delay(1000); // 1ms
    set_AT89Cx051_mode(AT89Cx051_RD_MODE); // set mode
    hw_delay(1000); // 1ms    
    AT89Cx051_mux(AT89Cx051_X1_MUX); // X1 as pulse
    progress_loop(addr, size, "Veryfying"){
	hw_delay(100);
	rdata = AT89Cx051_get_data();
	wdata = get_buffer(addr);
	break_if( rdata != wdata);
	AT89Cx051_pulse( 150 );
    }
    if(!silent | (wdata != rdata)){
	 sprintf(text, "[WN][TEXT]Flash and buffer differ !!!\nAddress = 0x%X \nBuffer = 0x%X%X\nChip = 0x%X%X[/TEXT][BR]OK",
	    addr,
	    to_hex(wdata, 1), to_hex(wdata, 0),
	    to_hex(rdata, 1), to_hex(rdata, 0) 
	 );	    
	 show_message(0,(rdata == wdata) ? "[IF][TEXT]Flash and buffer are consistent[/TEXT][BR]OK" : text, NULL, NULL);
     }
    set_address(0);
    finish_action();
}

void erase_AT89Cx051(int size)
{
    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    AT89Cx051_RST(AT89Cx051_RST_LOW);    // RST to GND
    AT89Cx051_mux(AT89Cx051_X1_OFF_MUX); // X1  to GND
    hw_delay(1000); // 1ms
    AT89Cx051_RST(AT89Cx051_RST_HIGH);    // RST to H, clear internal address counter
    hw_delay(1000); // 1ms
    set_AT89Cx051_mode(AT89Cx051_ERA_MODE); // set mode to erase
    hw_delay(1000); // 1ms    
    AT89Cx051_mux(AT89Cx051_PROG_MUX); // X1 as pulse
    hw_sw_vpp(1);	// set VPP    
    // erase pulse
    AT89Cx051_pulse( 11000 ); // 11ms
    set_address(0);
    finish_action();
    test_blank_AT89Cx051(size, 0);
}

void write_AT89Cx051(int size)
{
    int addr = 0;
    char wdata = 0, rdata = 0;
    char text[256];

    if(test_blank_AT89Cx051(size, 1)){
       SET_ERROR;
       return;
    }
    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    AT89Cx051_RST(AT89Cx051_RST_LOW);    // RST to GND
    AT89Cx051_mux(AT89Cx051_X1_OFF_MUX); // X1  to GND
    hw_delay(1000); // 1ms
    AT89Cx051_RST(AT89Cx051_RST_HIGH);    // RST to H, clear internal address counter
    hw_delay(1000); // 1ms
    progress_loop(addr, size, "Writing"){
	set_AT89Cx051_mode(AT89Cx051_WR_MODE); // set mode to write
	hw_delay(100); // 100µs    
	AT89Cx051_mux(AT89Cx051_PROG_MUX); // PROG as pulse
	wdata = get_buffer(addr);	   // get data from buffer and store it for veryfication
	AT89Cx051_put_data(wdata);	   // out data
	hw_delay(10);
	hw_sw_vpp(1);			   // VPP ON
	hw_delay(20);
	AT89Cx051_pulse( 1250 );           // 1.25 ms program pulse
	hw_delay(20);
	hw_sw_vpp(0);			   // VPP OFF
	hw_delay(500); 			   // 100µs    
	set_AT89Cx051_mode(AT89Cx051_RD_MODE); // set mode to read for veryfication
	hw_delay(500); 			   // 500µs    
	rdata = AT89Cx051_get_data();
	break_if(rdata != wdata);                // veryfication
	AT89Cx051_mux(AT89Cx051_X1_MUX);   // X1 as pulse
	hw_delay(20);
	AT89Cx051_pulse( 150 );		   // address increment
	hw_delay(20);
    }
    if(rdata != wdata){
	sprintf(
	    text, "[ER][TEXT]Veryfication error: 0x%X%X do not match 0x%X%X at address 0x%X[/TEXT][BR]OK", 
	    to_hex(rdata, 1), to_hex(rdata, 0), 
	    to_hex(wdata, 1), to_hex(wdata, 0), 
	    addr
	);
	show_message(0,text, NULL, NULL);
	SET_ERROR;
    }
    set_address(0);
    finish_action();
    verify_AT89Cx051(size, 1);		// verificate whole at end
}

void lock_bit_AT89Cx051(int size)
{
    unsigned long *lb;
    lb = checkbox(
	"[TITLE]LOCKBITS[/TITLE][TYPE:QS]"
	"[CB:1:0:LB1 (further programing of the flash is disabled)]"
	"[CB:2:0:LB2 (same as LB1, also verify is disabled)]"
    );
    if(!lb) return;

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    if(*lb & 1){
	AT89Cx051_RST(AT89Cx051_RST_LOW);    // RST to GND
	AT89Cx051_mux(AT89Cx051_X1_OFF_MUX); // X1  to GND
	hw_delay(1000); // 1ms
	AT89Cx051_RST(AT89Cx051_RST_HIGH);    // RST to H, clear internal address counter
	hw_delay(1000); // 1ms
	set_AT89Cx051_mode(AT89Cx051_LB1_MODE); // set mode to erase
	hw_delay(1000); // 1ms    
	AT89Cx051_mux(AT89Cx051_PROG_MUX); // X1 as pulse
	hw_sw_vpp(1);	// set VPP    
	// erase pulse
	AT89Cx051_pulse( 1250 ); // 1,25ms
	hw_delay(1000);
	hw_sw_vpp(0);
    }
    if(*lb & 2){
	AT89Cx051_RST(AT89Cx051_RST_LOW);    // RST to GND
	AT89Cx051_mux(AT89Cx051_X1_OFF_MUX); // X1  to GND
	hw_delay(1000); // 1ms
	AT89Cx051_RST(AT89Cx051_RST_HIGH);    // RST to H, clear internal address counter
	hw_delay(1000); // 1ms
	set_AT89Cx051_mode(AT89Cx051_LB2_MODE); // set mode to erase
	hw_delay(1000); // 1ms    
	AT89Cx051_mux(AT89Cx051_PROG_MUX); // X1 as pulse
	hw_sw_vpp(1);	// set VPP    
	// erase pulse
	AT89Cx051_pulse( 1250 ); // 1,25ms
	hw_delay(1000);
	hw_sw_vpp(0);
    }
    set_address(0);
    finish_action();
}
/*************************************************************************/
void set_AT89C5x_mode(int mode)
{
    addr_state &= ~AT89C5x_MODE_MASK;
    addr_state |= mode & AT89C5x_MODE_MASK;
    set_address(addr_state);  /* ustawienie linii selekcji stanu: A15-P2.6, A16-P2.7, A17-P3.6, A18-P3.7 */
}

void set_AT89C5x_addr(int addr)
{
    addr_state &= AT89C5x_MODE_MASK;
    addr_state |= addr & ~AT89C5x_MODE_MASK;
    set_address(addr_state);  /* ustawienie linii adresowych A0..A14, z pominięciem linii sterujących */
}

#define AT89C5x_prog_pulse	AT89Cx051_pulse	

unsigned char read_byte_AT89C5x(int addr, int mode)
{
    set_AT89C5x_mode( mode | AT89C5x_P27_H_MODE);
    hw_delay(1); // 1us 
    set_AT89C5x_addr(addr);
    hw_delay(1); // 1us 
    set_AT89C5x_mode( mode );	
    hw_delay(1); // 1us 
    return hw_get_data();
}

void read_AT89C5x(int size)
{ 
    int addr = 0;
//    int signature;
    addr_state = 0;

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    ce(1,100);
//    signature = 0;
    progress_loop(addr, size, "Reading")
	put_buffer( addr, read_byte_AT89C5x(addr, AT89C5x_READ_MODE) );

    set_address(0);
    set_data(0);
    finish_action();
}

void sign_AT89C5x(int size)
{ 

    int addr = 0;
    int signature;
    char text[256];

    addr_state = 0;
    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    ce(1,100);
    signature = 0;
    progress_loop(addr, 3, "Reading")
	signature |= read_byte_AT89C5x(addr + 0x30, AT89C5x_SIGN_MODE) << (8 * addr);
    set_address(0);
    set_data(0);
    finish_action();
    sprintf(
	text, "[IF][TEXT]Chip signature: 0x%X%X%X%X%X%X\n%s[/TEXT][BR]OK", 
	to_hex(signature, 5),to_hex(signature, 4),  
	to_hex(signature, 3),to_hex(signature, 2),  
	to_hex(signature, 1),to_hex(signature, 0), 
	take_signature_name( signature )
    );
    show_message(0, text,NULL,NULL);
}

void verify_AT89C5x(int size, char silent)
{ 
    int addr = 0;
    char text[256];
    addr_state = 0;
    char rdata = 0, wdata = 0;

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    ce(1,100);
    progress_loop(addr, size, "Verifying"){
	rdata = read_byte_AT89C5x(addr, AT89C5x_READ_MODE);
	wdata = get_buffer(addr);
	break_if( rdata != wdata);
    }

    if(!silent | (wdata != rdata)){
	 sprintf(text, "[WN][TEXT]Flash and buffer differ !!!\nAddress = 0x%X \nBuffer = 0x%X%X\nChip = 0x%X%X[/TEXT][BR]OK",
	    addr,
	    to_hex(wdata, 1), to_hex(wdata, 0),
	    to_hex(rdata, 1), to_hex(rdata, 0) 
	 );	    
	 show_message(0,(rdata == wdata) ? "[IF][TEXT]Flash and buffer are consistent[/TEXT][BR]OK" : text, NULL, NULL);
     }
    
    set_address(0);
    set_data(0);
    finish_action();
}

void test_blank_AT89C5x(int size, char silent)
{ 
    int addr = 0;
    char text[256];
    addr_state = 0;
    unsigned char rdata = 0;

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    ce(1,100);
    progress_loop(addr, size, "Test blank"){
	rdata = read_byte_AT89C5x(addr, AT89C5x_READ_MODE);
	break_if( rdata != 0xff);
    }
    if(!silent | (rdata != 0xff)){
	 sprintf(text, "[WN][TEXT]Flash is not empty !!!\nAddress = 0x%X\n byte = 0x%X%X[/TEXT][BR]OK",
	    addr,
	    to_hex(rdata, 1), to_hex(rdata, 0) 
	 );	    
	 show_message(0,(rdata == 0xff) ? "[IF][TEXT]Flash is clear[/TEXT][BR]OK" : text, NULL, NULL);
    }
    set_address(0);
    set_data(0);
    finish_action();
}
/*
    set_AT89C5x_mode( mode | AT89C5x_P27_H_MODE);
    hw_delay(1); // 1us 
    set_AT89C5x_addr(addr);
    hw_delay(1); // 1us 
    set_AT89C5x_mode( mode );	
    hw_delay(1); // 1us 
    return hw_get_data();
*/
#define AT89C5x_pulse	AT89Cx051_pulse

void write_AT89C5x(int size)
{ 
    int addr = 0;
    unsigned char wdata = 0, rdata = 0;
    char text[256];
    addr_state = 0;

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    ce(1,100);
    progress_loop(addr, size, "Writing"){
        wdata = get_buffer(addr);
        if(wdata == 0xff){
	    rdata = 0xff;
    	    continue; // skip 0xff
        }
	set_AT89C5x_mode( AT89C5x_WRITE_MODE);
	hw_sw_vpp(1);
	hw_delay(1); // 1us 
        set_AT89C5x_addr(addr);
        set_data(wdata);
	hw_delay(1); // 1us 
	AT89C5x_pulse( 1250 );   // 1.25 ms program pulse
	hw_sw_vpp(0);
	hw_delay(400); // 400us 
	set_AT89C5x_mode( AT89C5x_READ_MODE );	
	hw_delay(400); // 400us 
	rdata = hw_get_data();
	break_if(rdata != wdata);
    }
    if(rdata != wdata){
	sprintf(
	    text, "[ER][TEXT]Veryfication error: 0x%X%X do not match 0x%X%X at address 0x%X[/TEXT][BR]OK", 
	    to_hex(rdata, 1), to_hex(rdata, 0), 
	    to_hex(wdata, 1), to_hex(wdata, 0), 
	    addr
	);
	show_message(0,text, NULL, NULL);
	SET_ERROR;
    }
    set_address(0);
    set_data(0);
    finish_action();
    verify_AT89C5x(size, 1); // verificate whole at end
}

void erase_AT89C5x(int size)
{ 
    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    ce(1,100);
    set_AT89C5x_mode( AT89C5x_ERASE_MODE);
    hw_sw_vpp(1);	// set VPP    
    // erase pulse
    AT89C5x_pulse( 11000 ); // 11ms
    set_address(0);
    finish_action();
    test_blank_AT89C5x(size, 0);
}

void lock_bit_AT89C5x(int size)
{ 
    unsigned long *lb;
    lb = checkbox(
	"[TITLE]LOCKBITS[/TITLE][TYPE:QS]"
	"[CB:1:0:LB1 (further programing of the flash is disabled, lock on MOVC)]"
	"[CB:2:0:LB2 (same as LB1, also verify is disabled)]"
	"[CB:4:0:LB3 (same as LB2, also external execution is disabled)]"
    );
    if(!lb) return;

    hw_sw_vpp(0);    // VPP OFF
    hw_sw_vcc(1);    // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    if(*lb & 1){
	ce(1,100);
	set_AT89C5x_mode( AT89C5x_LB1_MODE);
	hw_sw_vpp(1);	// set VPP    
	AT89C5x_pulse( 11000 ); // 11ms
	set_address(0);
	hw_sw_vpp(0);	// set VPP    
    }
    hw_delay(1000); // 1ms
    if(*lb & 2){
	ce(1,100);
	set_AT89C5x_mode( AT89C5x_LB2_MODE);
	hw_sw_vpp(1);	// set VPP    
	AT89C5x_pulse( 11000 ); // 11ms
	set_address(0);
	hw_sw_vpp(0);	// set VPP    
    }
    hw_delay(1000); // 1ms
    if(*lb & 4){
	ce(1,100);
	set_AT89C5x_mode( AT89C5x_LB3_MODE);
	hw_sw_vpp(1);	// set VPP    
	AT89C5x_pulse( 11000 ); // 11ms
	set_address(0);
	hw_sw_vpp(0);	// set VPP    
    }
    finish_action();
}

/***************************************************************************/
void AT90S1200_enter_program_mode()
{
    hw_sw_vpp( 0 );
    hw_sw_vcc( 1 );
    hw_delay(100); // wait 100µs
    ce( 1, 1 );
    set_address( 1 << 17 ); // select RESET pin
    ce( 0, 1 ); // set RESET pin to "0"
    hw_delay(100); // wait 100µs
    hw_sw_vpp( 1 );
    ce( 1, 1 ); // RESET PIN to H, ignoring value, VPP override it
    hw_delay(100); // wait 100µs
}

unsigned char AT90S1200_get_data( int bs )
{
    unsigned char tmp;
    bs &= 1;
    ce(0, 1);
    set_address( (1 << 18) | (bs << 8) ); // XA1,XA0 = "00", BS = bs, /OE = "CE", XTAL_MUX = "0"
    tmp = AT89Cx051_get_data();
    ce( 1, 100 );
    return tmp;
}

void AT90S1200_load_address_low_byte( char addr )
{
    set_address( (1 << 15) | (1 << 16) ); // XA1,XA0 = "00", BS = 0, /OE = 1, XTAL_MUX = /CE,
    AT89Cx051_put_data( addr ); // address
    ce( 0, 100 ); // positive XTAL pulse
    ce( 1, 100 );
}

void AT90S1200_load_address_high_byte( char addr )
{
    set_address( (1 << 15) | (1 << 16) | (1 << 8) ); // XA1,XA0 = "00", BS = 0, /OE = 1, XTAL_MUX = /CE,
    AT89Cx051_put_data( addr ); // address
    ce( 0, 100 ); // positive XTAL pulse
    ce( 1, 100 );
}

void AT90S1200_load_cmd( char cmd )
{
    ce( 1, 1 );
    set_address( (1 << 9) | (1 << 15) | (1 << 16) ); // XA1,XA0 = "10", BS = 0, /OE = 1, XTAL_MUX = /CE,
    AT89Cx051_put_data( cmd ); // command signature
    ce( 0, 100 ); // positive XTAL pulse
    ce( 1, 100 );
}

void AT90S1200_load_data_low_byte( char data )
{
    set_address( (1 << 15) | (1 << 16) | (1 << 7)); // XA1,XA0 = "01", BS = 0, /OE = 1, XTAL_MUX = /CE,
    AT89Cx051_put_data( data ); // address
    ce( 0, 100 ); // positive XTAL pulse
    ce( 1, 100 );
}

#define AT90S1200_load_data_high_byte AT90S1200_load_data_low_byte

void AT90S1200_WR_pulse(int  bs )
{
    bs &= 1;
    ce( 1, 100 );
    set_address( (bs << 8) | (1 << 17) | (1 << 18)); // XA1, XA0 = "00", BS = 0, /OE = 1, XTAL_MUX = WR,    
    ce( 0, 100 );
    ce( 1, 100 );
}

void AT90S1200_write_data_low_byte()
{
    AT90S1200_WR_pulse( 0 );
}

void AT90S1200_write_data_high_byte()
{
    AT90S1200_WR_pulse( 1 );
}

/***********/
void signature_AT90S1200_(int size)
{
    int signature = 0;
    int addr;
    char text[256];

    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_READ_SIGNATURE );

    progress_loop(addr, 3, "Reading signature"){
	AT90S1200_load_address_low_byte( addr );
	signature |= BYTE_POSITION( AT90S1200_get_data( 0 ), addr );
    }    

    set_address(0);
    set_data(0);
    finish_action();
    sprintf(
	text, "[IF][TEXT]Chip signature: 0x%X%X%X%X%X%X\n%s[/TEXT][BR]OK", 
	to_hex(signature, 5),to_hex(signature, 4),  
	to_hex(signature, 3),to_hex(signature, 2),  
	to_hex(signature, 1),to_hex(signature, 0), 
	take_signature_name( signature )
    );
    show_message(0, text,NULL,NULL);
    finish_action();
}

void read_flash_AT90S1200_(int size)
{
    int addr;

    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_READ_FLASH );

    progress_loop(addr, size / 2, "Reading FLASH"){
	AT90S1200_load_address_high_byte( (addr >> 8) & 0xff );
	AT90S1200_load_address_low_byte( addr & 0xff);
	put_buffer( addr * 2 + 0,  AT90S1200_get_data( 0 ));
	put_buffer( addr * 2 + 1,  AT90S1200_get_data( 1 ));
    }    

    set_address(0);
    set_data(0);
    finish_action();
}

void read_eeprom_AT90S1200_(int size)
{
    int addr;

    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_READ_EEPROM );

    progress_loop(addr, size, "Reading EEPROM"){
	AT90S1200_load_address_low_byte( addr & 0xff);
	put_buffer( addr, AT90S1200_get_data( 0 ) );
    }    

    set_address(0);
    set_data(0);
    finish_action();
}

void verify_flash_AT90S1200_(int size, char silent)
{
    int addr = 0;
    char text[256];
    addr_state = 0;
    char rdata = 0, wdata = 0, k;

    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_READ_FLASH );

    progress_loop(addr, size / 2, "Veryfying FLASH"){
	AT90S1200_load_address_high_byte( (addr >> 8) & 0xff );
	AT90S1200_load_address_low_byte( addr & 0xff);
	rdata = AT90S1200_get_data( 0 );
	wdata = get_buffer(addr*2);
	k = 0;
	break_if( rdata != wdata);
	k = 1;
	rdata = AT90S1200_get_data( 1 );
	wdata = get_buffer(addr*2 + 1);
	if( rdata != wdata){
	   progressbar_free();
	   break;
	}
    }    

    if(!silent | (wdata != rdata)){
	 sprintf(text, "[WN][TEXT]Flash and buffer differ !!!\nAddress = 0x%X \nBuffer = 0x%X%X\nChip = 0x%X%X[/TEXT][BR]OK",
	    addr * 2 + k,
	    to_hex(wdata, 1), to_hex(wdata, 0),
	    to_hex(rdata, 1), to_hex(rdata, 0) 
	 );	    
	 show_message(0,(rdata == wdata) ? "[IF][TEXT]Flash and buffer are consistent[/TEXT][BR]OK" : text, NULL, NULL);
     }
    
    set_address(0);
    set_data(0);
    finish_action();
}

void verify_eeprom_AT90S1200_(int size, char silent)
{
    int addr = 0;
    char text[256];
    addr_state = 0;
    char rdata = 0, wdata = 0;

    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_READ_EEPROM );

    progress_loop(addr, size, "Veryfying FLASH"){
	AT90S1200_load_address_low_byte( addr & 0xff);
	rdata = AT90S1200_get_data( 0 );
	wdata = get_buffer(addr);
	break_if( rdata != wdata);
    }    

    if(!silent | (wdata != rdata)){
	 sprintf(text, "[WN][TEXT]EEPROM and buffer differ !!!\nAddress = 0x%X \nBuffer = 0x%X%X\nChip = 0x%X%X[/TEXT][BR]OK",
	    addr,
	    to_hex(wdata, 1), to_hex(wdata, 0),
	    to_hex(rdata, 1), to_hex(rdata, 0) 
	 );	    
	 show_message(0,(rdata == wdata) ? "[IF][TEXT]EEPROM and buffer are consistent[/TEXT][BR]OK" : text, NULL, NULL);
     }
    
    set_address(0);
    set_data(0);
    finish_action();

}

char test_blank_AT90S1200_(int size, char silent)
{
    int addr = 0;
    char text[256];
    addr_state = 0;
    unsigned char rdata = 0;

    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_READ_FLASH );

    progress_loop(addr, size / 2, "Test blank FLASH"){
	if((addr & 0xff) == 0) 
	    AT90S1200_load_address_high_byte( (addr >> 8) & 0xff );
	AT90S1200_load_address_low_byte( addr & 0xff);
	rdata = AT90S1200_get_data( 0 );
	break_if( rdata != 0xff );
	rdata = AT90S1200_get_data( 1 );
	break_if( rdata != 0xff );
    }    

    if(!silent | (rdata != 0xff)){
	 sprintf(text, "[WN][TEXT]Flash is not empty !!!\nAddress = 0x%X\n byte = 0x%X%X[/TEXT][BR]OK",
	    addr * 2,
	    to_hex(rdata, 1), to_hex(rdata, 0) 
	 );	    
	 show_message(0,(rdata == 0xff) ? "[IF][TEXT]Flash is clear[/TEXT][BR]OK" : text, NULL, NULL);
	return 1;
    }
    set_address(0);
    set_data(0);
    finish_action();
    return 0;
}

void chiperase_AT90S1200_(int size)
{
    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_CHIP_ERASE );
    AT90S1200_WR_pulse( 0 );
    hw_delay(10000);
    set_address(0);
    set_data(0);
    finish_action();
}

void write_flash_AT90S1200_(int size)
{
    int addr;

    if(test_blank_AT90S1200_( size, 1)) return;

    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_WRITE_FLASH );

    progress_loop(addr, size / 2, "Writing FLASH"){
	AT90S1200_load_address_high_byte( ( addr >> 8 ) & 0xff );
	AT90S1200_load_address_low_byte( addr & 0xff);
	AT90S1200_load_data_low_byte( get_buffer(addr * 2) );
	AT90S1200_write_data_low_byte();
	hw_delay(1500);
	AT90S1200_load_data_high_byte( get_buffer(addr * 2 + 1) );
	AT90S1200_write_data_high_byte();
	hw_delay(1500);
    }    
    set_address(0);
    set_data(0);
    finish_action();
    verify_flash_AT90S1200_(size, 1); // verificate whole at end
}

void write_eeprom_AT90S1200_(int size)
{
    int addr;

    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_WRITE_EEPROM );

    progress_loop(addr, size, "Writing EEPROM"){
	AT90S1200_load_address_low_byte( addr & 0xff);
	AT90S1200_load_data_low_byte( get_buffer(addr) );
	AT90S1200_write_data_low_byte();
	hw_delay(1500);
    }    

    set_address(0);
    set_data(0);
    finish_action();
    verify_eeprom_AT90S1200_(size, 1); // verificate whole at end
}

unsigned char AT90S1200_load_lb()
{
    unsigned char fuses;
    addr_state = 0;

    AT90S1200_enter_program_mode();
    ce( 1, 1 );
    set_address( (1 << 9) | (1 << 15) | (1 << 16) ); // XA1,XA0 = "10", BS = 0, /OE = 1, XTAL_MUX = /CE,
    AT89Cx051_put_data( AT90S1200_READ_LB ); // command signature
    ce( 0, 100 ); // positive XTAL pulse
    ce( 1, 100 );

    set_address( (1 << 18) | (1 << 8) ); // XA1,XA0 = "00", BS = 1, /OE = "CE", XTAL_MUX = "0"
    ce(0, 100);
    fuses = AT89Cx051_get_data();
    ce( 1, 100 );

    set_address(0);
    set_data(0);
    finish_action();

    return fuses;
}

void AT90S1200_set_fuse_bits( char bytes)
{
    addr_state = 0;

    AT90S1200_enter_program_mode();

    bytes |= ~0x21; // allow bit 5 and bit 0

    ce( 1, 1 );
    set_address( (1 << 9) | (1 << 15) | (1 << 16) ); // XA1,XA0 = "10", BS = 0, /OE = 1, XTAL_MUX = /CE,
    AT89Cx051_put_data( AT90S1200_WRITE_FUSE_BITS ); // command signature
    ce( 0, 100 ); // positive XTAL pulse
    ce( 1, 100 );

    set_address( (1 << 15) | (1 << 16) | (1 << 7)); // XA1,XA0 = "01", BS = 0, /OE = 1, XTAL_MUX = /CE,
    AT89Cx051_put_data( bytes ); // address
    ce( 0, 100 ); // positive XTAL pulse
    ce( 1, 100 );

    set_address( (1 << 17) | (1 << 18));
    ce( 0, 2000 ); // /WR pulse 2ms
    ce( 1, 100 );

    hw_delay( 10000 );
    set_address(0);
    set_data(0);
}

void AT90S1200_set_lock_bits( char locks )
{
    addr_state = 0;
    AT90S1200_enter_program_mode();
    AT90S1200_load_cmd( AT90S1200_WRITE_LB );    
    AT90S1200_load_data_low_byte( locks | 0xf9 );
    ce( 1, 100 );
    set_address( (1 << 17) | (1 << 18)); // XA1, XA0 = "00", BS = 0, /OE = 1, XTAL_MUX = WR,    
    ce( 0, 10000 ); // /WR pulse
    ce( 1, 100 );
    hw_delay( 10000 );
    set_address(0);
    set_data(0);
    finish_action();
}    

void fusebits_AT90S1200_(int size, char mode) // mode=0 - AT90S1200
{
    unsigned long *res;
    char fusebits;
    char text[512];
    char prev;

    fusebits = AT90S1200_load_lb();
    prev = fusebits;
    fusebits = ~fusebits;
    sprintf(text, 
	"[TITLE]LOCKBITS AND FUSES[/TITLE][TYPE:QS]"
	"[FRAME:Lock Bits]"
	"[CB:2:%x:LOCK:Lock Bit 1 (further programing of the FLASH and EEPROM is disabled)]"
	"[CB:4:%x:LOCK:Lock Bit 2 (same as LB 1, also verify is disabled)]\n"
	"[/FRAME]"
	"[FRAME:Fuse Bits]"
	"[CB:32:%x:SPIEN (SPI snable)]"
	"[CB:1:%x:%s]"
	"[/FRAME]",
	(fusebits >> 7) & 1, (fusebits >> 6) & 1,
	(fusebits >> 5) & 1, fusebits & 1,
	mode ? "FSTRT (Short Start-up Time)":"RCEN   (RC Oscillator enable)" 
    );

    if(!(res = (checkbox( text )))){
	finish_action();
	return;
    }
    fusebits = *res;
    // write fuses if change happened
    if( (prev & 0x21) != (fusebits & 0x21) )
	AT90S1200_set_fuse_bits( ~fusebits );
    
    prev = ((prev >> 4) & 0x04) | ((prev >> 6) & 0x02) | 0xf9;

    // write lockbits
    if((fusebits & 0x06) != (prev & 0x06) )
	AT90S1200_set_lock_bits( ~fusebits );
}

void write_i87xx( int size, char mode_p )
{ 
    int addr = 0, err_counter, rpt, n;
    unsigned char wdata = 0, rdata = 0;
    char text[256];
    addr_state = 0;

    hw_sw_vpp(0);   // VPP OFF
    hw_sw_vcc(1);   // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    ce(1,100);
    err_counter = 0;
    rpt = 0;
    progress_loop(addr, size, "Writing"){
i875x_REPEAT:
        wdata = get_buffer(addr);
        if(wdata == 0xff){
	    rdata = 0xff;
    	    continue; // skip 0xff
        }
	set_AT89C5x_mode( AT89C5x_WRITE_MODE);
	hw_sw_vpp(1);
	hw_delay(1); // 1us 
        set_AT89C5x_addr(addr);
        set_data(wdata);
	hw_delay(1); // 1us 
	if(mode_p)
	for(n = 0; n < 5; n++)
	    AT89C5x_pulse( 100 );   // 5 x 100µs program pulse
	else
	    AT89C5x_pulse( 50000 );   // 50ms program pulse	
	hw_sw_vpp(0);
	hw_delay(400); // 400us 
	set_AT89C5x_mode( AT89C5x_READ_MODE );	
	hw_delay(400); // 400us 
	rdata = hw_get_data();
	if(rdata == wdata){
	    err_counter = 0;
	    continue;
	}
	if(++err_counter < 50){
	    rpt++;
	    goto i875x_REPEAT;
	}
	progressbar_free();
	break;
    }
    if(rdata != wdata){
	sprintf(
	    text, "[ER][TEXT]Veryfication error: 0x%X%X do not match 0x%X%X at address 0x%X[/TEXT][BR]OK", 
	    to_hex(rdata, 1), to_hex(rdata, 0), 
	    to_hex(wdata, 1), to_hex(wdata, 0), 
	    addr
	);
	show_message(0,text, NULL, NULL);
	SET_ERROR;
    }
    if(rpt){
	sprintf(
	    text, "[ER][TEXT] Trouble in write: %i repeats. [/TEXT][BR]OK", 
	    rpt
	);
	show_message(0,text, NULL, NULL);
    }
    set_address(0);
    set_data(0);
    finish_action();
    verify_AT89C5x(size, 1); // verificate whole at end
}

void lock_i87xx( int size )
{
    unsigned long *lb;
    lb = checkbox(
	"[TITLE]LOCKBITS[/TITLE][TYPE:QS]"
	"[CB:1:0:Lock Bit (no veryfication, no external execution)]"
    );
    if( !lb ) return;
    if( *lb != 1) return;

    hw_sw_vpp(0);    // VPP OFF
    hw_sw_vcc(1);    // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    ce(1,100);
    set_AT89C5x_mode( I87C5x_LB_MODE);
    hw_sw_vpp(1);	// set VPP    
    AT89C5x_pulse( 50000 ); // 50ms
    set_address(0);
    hw_sw_vpp(0);	// set VPP    
    hw_delay(1000); // 1ms
    finish_action();
}

void lock_p87xx( int size )
{
    unsigned long *lb;
    lb = checkbox(
	"[TITLE]LOCKBITS[/TITLE][TYPE:QS]"
	"[CB:1:0:LB1 (further programing of the flash is disabled, lock on MOVC)]"
	"[CB:2:0:LB2 (same as LB1, also verify is disabled)]"
	"[CB:4:0:LB3 (same as LB2, also external execution is disabled)]"
    );
    if(!lb) return;

    hw_sw_vpp(0);    // VPP OFF
    hw_sw_vcc(1);    // VCC ON
    hw_delay(10000); // 10ms for power stabilise
    if(*lb & 1){
	ce(1,100);
	set_AT89C5x_mode( AT89C5x_LB1_MODE);
	hw_sw_vpp(1);	// set VPP    
	AT89C5x_pulse( 50000 ); // 50ms
	set_address(0);
	hw_sw_vpp(0);	// set VPP    
    }
    hw_delay(1000); // 1ms
    if(*lb & 2){
	ce(1,100);
	set_AT89C5x_mode( AT89C5x_LB2_MODE);
	hw_sw_vpp(1);	// set VPP    
	AT89C5x_pulse( 50000 ); // 50ms
	set_address(0);
	hw_sw_vpp(0);	// set VPP    
    }
    hw_delay(1000); // 1ms
    if(*lb & 4){
	ce(1,100);
	set_AT89C5x_mode( AT89C5x_LB3_MODE);
	hw_sw_vpp(1);	// set VPP    
	AT89C5x_pulse( 50000 ); // 50ms
	set_address(0);
	hw_sw_vpp(0);	// set VPP    
    }
    finish_action();
}

/************************************************************************
* i87Cxx
*/
REGISTER_FUNCTION( read,       i8751, AT89C5x, SIZE_I8751 );
REGISTER_FUNCTION( write,      i8751, i87xx, SIZE_I8751, 0 );
REGISTER_FUNCTION( verify,     i8751, AT89C5x, SIZE_I8751, 0 );
REGISTER_FUNCTION( lock,       i8751, i87xx, SIZE_I8751 );
REGISTER_FUNCTION( test_blank, i8751, AT89C5x, SIZE_I8751, 0 );

REGISTER_FUNCTION( read,       i8752, AT89C5x, SIZE_I8752 );
REGISTER_FUNCTION( write,      i8752, i87xx, SIZE_I8752, 0 );
REGISTER_FUNCTION( verify,     i8752, AT89C5x, SIZE_I8752, 0 );
REGISTER_FUNCTION( lock,       i8752, i87xx, SIZE_I8752 );
REGISTER_FUNCTION( test_blank, i8752, AT89C5x, SIZE_I8752, 0 );

REGISTER_FUNCTION( read,       p8751, AT89C5x, SIZE_I8751 );
REGISTER_FUNCTION( write,      p8751, i87xx, SIZE_I8751, 1 );
REGISTER_FUNCTION( verify,     p8751, AT89C5x, SIZE_I8751, 0 );
REGISTER_FUNCTION( lock,       p8751, p87xx, SIZE_I8751 );
REGISTER_FUNCTION( test_blank, p8751, AT89C5x, SIZE_I8751, 0 );

REGISTER_FUNCTION( read,       p8752, AT89C5x, SIZE_I8752 );
REGISTER_FUNCTION( write,      p8752, i87xx, SIZE_I8752, 1 );
REGISTER_FUNCTION( verify,     p8752, AT89C5x, SIZE_I8752, 0 );
REGISTER_FUNCTION( lock,       p8752, p87xx, SIZE_I8752 );
REGISTER_FUNCTION( test_blank, p8752, AT89C5x, SIZE_I8752, 0 );

/************************************************************************
* AT89Cx051
*/
REGISTER_FUNCTION( read,       AT89C1051, AT89Cx051, SIZE_AT89C1051 );
REGISTER_FUNCTION( read,       AT89C2051, AT89Cx051, SIZE_AT89C2051 );
REGISTER_FUNCTION( read,       AT89C4051, AT89Cx051, SIZE_AT89C4051 );
REGISTER_FUNCTION( verify,     AT89C1051, AT89Cx051, SIZE_AT89C1051, 0 );
REGISTER_FUNCTION( verify,     AT89C2051, AT89Cx051, SIZE_AT89C2051, 0 );
REGISTER_FUNCTION( verify,     AT89C4051, AT89Cx051, SIZE_AT89C4051, 0 );
REGISTER_FUNCTION( write,      AT89C1051, AT89Cx051, SIZE_AT89C1051 );
REGISTER_FUNCTION( write,      AT89C2051, AT89Cx051, SIZE_AT89C2051 );
REGISTER_FUNCTION( write,      AT89C4051, AT89Cx051, SIZE_AT89C4051 );
REGISTER_FUNCTION( erase,      AT89C1051, AT89Cx051, SIZE_AT89C1051 );
REGISTER_FUNCTION( erase,      AT89C2051, AT89Cx051, SIZE_AT89C2051 );
REGISTER_FUNCTION( erase,      AT89C4051, AT89Cx051, SIZE_AT89C4051 );
REGISTER_FUNCTION( sign,       AT89C1051, AT89Cx051, SIZE_AT89C1051 );
REGISTER_FUNCTION( sign,       AT89C2051, AT89Cx051, SIZE_AT89C2051 );
REGISTER_FUNCTION( sign,       AT89C4051, AT89Cx051, SIZE_AT89C4051 );
REGISTER_FUNCTION( lock_bit,   AT89C1051, AT89Cx051, SIZE_AT89C1051 );
REGISTER_FUNCTION( lock_bit,   AT89C2051, AT89Cx051, SIZE_AT89C2051 );
REGISTER_FUNCTION( lock_bit,   AT89C4051, AT89Cx051, SIZE_AT89C4051 );
REGISTER_FUNCTION( test_blank, AT89C1051, AT89Cx051, SIZE_AT89C1051, 0 );
REGISTER_FUNCTION( test_blank, AT89C2051, AT89Cx051, SIZE_AT89C2051, 0 );
REGISTER_FUNCTION( test_blank, AT89C4051, AT89Cx051, SIZE_AT89C4051, 0 );

/************************************************************************
* AT89C5x
*/
REGISTER_FUNCTION( read,       AT89C51, AT89C5x, SIZE_AT89C51 );
REGISTER_FUNCTION( read,       AT89C52, AT89C5x, SIZE_AT89C52 );
REGISTER_FUNCTION( write,      AT89C51, AT89C5x, SIZE_AT89C51 );
REGISTER_FUNCTION( write,      AT89C52, AT89C5x, SIZE_AT89C52 );
REGISTER_FUNCTION( verify,     AT89C51, AT89C5x, SIZE_AT89C51, 0 );
REGISTER_FUNCTION( verify,     AT89C52, AT89C5x, SIZE_AT89C52, 0 );
REGISTER_FUNCTION( erase,      AT89C51, AT89C5x, SIZE_AT89C51 );
REGISTER_FUNCTION( erase,      AT89C52, AT89C5x, SIZE_AT89C52 );
REGISTER_FUNCTION( sign,       AT89C51, AT89C5x, SIZE_AT89C51 );
REGISTER_FUNCTION( sign,       AT89C52, AT89C5x, SIZE_AT89C52 );
REGISTER_FUNCTION( lock_bit,   AT89C51, AT89C5x, SIZE_AT89C51 );
REGISTER_FUNCTION( lock_bit,   AT89C52, AT89C5x, SIZE_AT89C52 );
REGISTER_FUNCTION( test_blank, AT89C51, AT89C5x, SIZE_AT89C51, 0 );
REGISTER_FUNCTION( test_blank, AT89C52, AT89C5x, SIZE_AT89C52, 0 );

/*************************************************************************
* AT90S1200 & AT90S2313
*/
REGISTER_FUNCTION( read_flash,   AT90S1200, AT90S1200_, SIZE_AT90S1200_FLASH  );
REGISTER_FUNCTION( read_eeprom,  AT90S1200, AT90S1200_, SIZE_AT90S1200_EEPROM );
REGISTER_FUNCTION( write_flash,  AT90S1200, AT90S1200_, SIZE_AT90S1200_FLASH  );
REGISTER_FUNCTION( write_eeprom, AT90S1200, AT90S1200_, SIZE_AT90S1200_EEPROM );
REGISTER_FUNCTION( verify_flash, AT90S1200, AT90S1200_, SIZE_AT90S1200_FLASH, 0 );
REGISTER_FUNCTION( verify_eeprom,AT90S1200, AT90S1200_, SIZE_AT90S1200_EEPROM, 0 );
REGISTER_FUNCTION( fusebits,     AT90S1200, AT90S1200_, SIZE_AT90S1200_FLASH, 0 );
REGISTER_FUNCTION( chiperase,    AT90S1200, AT90S1200_, SIZE_AT90S1200_FLASH  );
REGISTER_FUNCTION( signature,    AT90S1200, AT90S1200_, SIZE_AT90S1200_FLASH  );
REGISTER_FUNCTION( test_blank,   AT90S1200, AT90S1200_, SIZE_AT90S1200_FLASH, 0  );

REGISTER_FUNCTION( read_flash,   AT90S2313, AT90S1200_, SIZE_AT90S2313_FLASH  );
REGISTER_FUNCTION( read_eeprom,  AT90S2313, AT90S1200_, SIZE_AT90S2313_EEPROM );
REGISTER_FUNCTION( write_flash,  AT90S2313, AT90S1200_, SIZE_AT90S2313_FLASH  );
REGISTER_FUNCTION( write_eeprom, AT90S2313, AT90S1200_, SIZE_AT90S2313_EEPROM );
REGISTER_FUNCTION( verify_flash, AT90S2313, AT90S1200_, SIZE_AT90S2313_FLASH, 0  );
REGISTER_FUNCTION( verify_eeprom,AT90S2313, AT90S1200_, SIZE_AT90S2313_EEPROM, 0 );
REGISTER_FUNCTION( fusebits,     AT90S2313, AT90S1200_, SIZE_AT90S2313_FLASH, 1 );
REGISTER_FUNCTION( chiperase,    AT90S2313, AT90S1200_, SIZE_AT90S2313_FLASH  );
REGISTER_FUNCTION( signature,    AT90S2313, AT90S1200_, SIZE_AT90S2313_FLASH  );
REGISTER_FUNCTION( test_blank,   AT90S2313, AT90S1200_, SIZE_AT90S2313_FLASH, 0  );

/***********************************************************************************
* missing: AT90S2333, AT90S4433, AT90S8535, AT90S4434, AT90S8515, AT90S4414, AT90S2323
*/

REGISTER_MODULE_BEGIN( MCS-51 )
// INTEL i875x
    register_chip_begin("/uk/MCS-51/INTEL","i8751", "i875x", SIZE_I8751);
	add_action(MODULE_READ_ACTION, read_i8751 );
	add_action(MODULE_PROG_ACTION, write_i8751);
	add_action(MODULE_VERIFY_ACTION, verify_i8751);
	add_action(MODULE_LOCKBIT_ACTION, lock_i8751);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_i8751);
    register_chip_end;
    register_chip_begin("/uk/MCS-51/INTEL","i8752", "i875x", SIZE_I8752);
	add_action(MODULE_READ_ACTION, read_i8752 );
	add_action(MODULE_PROG_ACTION, write_i8752);
	add_action(MODULE_VERIFY_ACTION, verify_i8752);
	add_action(MODULE_LOCKBIT_ACTION, lock_i8752);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_i8752);
    register_chip_end;
    register_chip_begin("/uk/MCS-51/PHILIPS","P8751", "i875x", SIZE_I8751);
	add_action(MODULE_READ_ACTION, read_p8751 );
	add_action(MODULE_PROG_ACTION, write_p8751);
	add_action(MODULE_VERIFY_ACTION, verify_p8751);
	add_action(MODULE_LOCKBIT_ACTION, lock_p8751);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_p8751);
    register_chip_end;
    register_chip_begin("/uk/MCS-51/PHILIPS","P8752", "i875x", SIZE_I8752);
	add_action(MODULE_READ_ACTION, read_p8752 );
	add_action(MODULE_PROG_ACTION, write_p8752);
	add_action(MODULE_VERIFY_ACTION, verify_p8752);
	add_action(MODULE_LOCKBIT_ACTION, lock_p8752);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_p8752);
    register_chip_end;

// ATMEL AT89Cx051
    register_chip_begin("/uk/MCS-51/ATMEL/AT89Cx051","AT89C1051", "AT89Cxx51", SIZE_AT89C1051);
	add_action(MODULE_READ_ACTION, read_AT89C1051);
	add_action(MODULE_PROG_ACTION, write_AT89C1051);
	add_action(MODULE_VERIFY_ACTION, verify_AT89C1051);
	add_action(MODULE_LOCKBIT_ACTION, lock_bit_AT89C1051);
	add_action(MODULE_ERASE_ACTION, erase_AT89C1051);
	add_action(MODULE_SIGN_ACTION, sign_AT89C1051);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_AT89C1051);
    register_chip_end;
    register_chip_begin("/uk/MCS-51/ATMEL/AT89Cx051","AT89C2051", "AT89Cxx51", SIZE_AT89C2051);
	add_action(MODULE_READ_ACTION, read_AT89C2051);
	add_action(MODULE_PROG_ACTION, write_AT89C2051);
	add_action(MODULE_VERIFY_ACTION, verify_AT89C2051);
	add_action(MODULE_LOCKBIT_ACTION, lock_bit_AT89C2051);
	add_action(MODULE_ERASE_ACTION, erase_AT89C2051);
	add_action(MODULE_SIGN_ACTION, sign_AT89C2051);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_AT89C2051);
    register_chip_end;
    register_chip_begin("/uk/MCS-51/ATMEL/AT89Cx051","AT89C4051", "AT89Cxx51", SIZE_AT89C4051);
	add_action(MODULE_READ_ACTION, read_AT89C4051);
	add_action(MODULE_PROG_ACTION, write_AT89C4051);
	add_action(MODULE_VERIFY_ACTION, verify_AT89C4051);
	add_action(MODULE_LOCKBIT_ACTION, lock_bit_AT89C4051);
	add_action(MODULE_ERASE_ACTION, erase_AT89C4051);
	add_action(MODULE_SIGN_ACTION, sign_AT89C4051);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_AT89C4051);
    register_chip_end;
// ATMEL AT89C5x
    register_chip_begin("/uk/MCS-51/ATMEL/AT89C5x","AT89C51", "AT89C5x", SIZE_AT89C51);
	add_action(MODULE_READ_ACTION, read_AT89C51);
	add_action(MODULE_PROG_ACTION, write_AT89C51);
	add_action(MODULE_VERIFY_ACTION, verify_AT89C51);
	add_action(MODULE_LOCKBIT_ACTION, lock_bit_AT89C51);
	add_action(MODULE_ERASE_ACTION, erase_AT89C51);
	add_action(MODULE_SIGN_ACTION, sign_AT89C51);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_AT89C51);
    register_chip_end;
    register_chip_begin("/uk/MCS-51/ATMEL/AT89C5x","AT89C52", "AT89C5x", SIZE_AT89C52);
	add_action(MODULE_READ_ACTION, read_AT89C52);
	add_action(MODULE_PROG_ACTION, write_AT89C52);
	add_action(MODULE_VERIFY_ACTION, verify_AT89C52);
	add_action(MODULE_LOCKBIT_ACTION, lock_bit_AT89C52);
	add_action(MODULE_ERASE_ACTION, erase_AT89C52);
	add_action(MODULE_SIGN_ACTION, sign_AT89C52);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_AT89C52);
    register_chip_end;
// ATMEL 90S1200 & 90S2313
    register_chip_begin("/uk/AVR/AT90S","AT90S1200", "AT90S20pin", SIZE_AT90S1200);
	add_action(MODULE_READ_FLASH_ACTION, read_flash_AT90S1200);
	add_action(MODULE_READ_EEPROM_ACTION, read_eeprom_AT90S1200);
	add_action(MODULE_PROG_FLASH_ACTION, write_flash_AT90S1200);
	add_action(MODULE_PROG_EEPROM_ACTION, write_eeprom_AT90S1200);
	add_action(MODULE_VERIFY_FLASH_ACTION, verify_flash_AT90S1200);
	add_action(MODULE_VERIFY_EEPROM_ACTION, verify_eeprom_AT90S1200);
	add_action(MODULE_LOCKBIT_ACTION, fusebits_AT90S1200);
	add_action(MODULE_ERASE_ACTION, chiperase_AT90S1200);
	add_action(MODULE_SIGN_ACTION, signature_AT90S1200);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_AT90S1200);
    register_chip_end;
    register_chip_begin("/uk/AVR/AT90S","AT90S2313", "AT90S20pin", SIZE_AT90S2313);
	add_action(MODULE_READ_FLASH_ACTION, read_flash_AT90S2313);
	add_action(MODULE_READ_EEPROM_ACTION, read_eeprom_AT90S2313);
	add_action(MODULE_PROG_FLASH_ACTION, write_flash_AT90S2313);
	add_action(MODULE_PROG_EEPROM_ACTION, write_eeprom_AT90S2313);
	add_action(MODULE_VERIFY_FLASH_ACTION, verify_flash_AT90S2313);
	add_action(MODULE_VERIFY_EEPROM_ACTION, verify_eeprom_AT90S2313);
	add_action(MODULE_LOCKBIT_ACTION, fusebits_AT90S2313);
	add_action(MODULE_ERASE_ACTION, chiperase_AT90S2313);
	add_action(MODULE_SIGN_ACTION, signature_AT90S2313);
	add_action(MODULE_TEST_BLANK_ACTION, test_blank_AT90S2313);
    register_chip_end;
 
REGISTER_MODULE_END

