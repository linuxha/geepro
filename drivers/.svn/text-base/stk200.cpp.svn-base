/* $Revision: 1.2 $ */
/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2006 Krzysztof Komarnicki
 * Email: krzkomar@wp.pl
 *
 * STK200 parallel port driver
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

#include "drivers.h"

static int stk200_set_test(char test)
{
    if( test & 1 ){
	if(parport_set_bit(PA, PP_02)) return PP_ERROR;
    } else {
	if(parport_clr_bit(PA, PP_02)) return PP_ERROR;
    }
    if( test & 2 ){
	if(parport_set_bit(PA, PP_03)) return PP_ERROR;
    } else {
	if(parport_clr_bit(PA, PP_03)) return PP_ERROR;
    }
    return 0;
}

static int stk200_get_test()
{    
    char t, x;

    t = 0;
    x = parport_get_bit(PB, PP_12);
    if( x == PP_ERROR ) return PP_ERROR;
    t |= x ? 1:0;
    x = parport_get_bit(PB, PP_11);
    if( x == PP_ERROR ) return PP_ERROR;
    t |= x ? 2:0;
    return t;
}

static int stk200_test_connected()
{
    char i, x;

    for(i = 0; i < 4; i++){
	if(stk200_set_test( i )) return 0;
	x = stk200_get_test();
	if( x == PP_ERROR ) return 0;
	if( x != i ) return 0;    
    }
    return 1;
}

static int stk200_get_miso()
{
    int x;

    x = parport_get_bit(PB, PP_10);
    if( x == PP_ERROR ) return PP_ERROR;
    return x;
}

static int stk200_connect( char state ) // on/off 3 state buffers
{
    if( state ){
	if(parport_clr_bit(PA, PP_04)) return HW_ERROR;    
	if(parport_clr_bit(PA, PP_05)) return HW_ERROR;        
    } else {
	if(parport_set_bit(PA, PP_04)) return HW_ERROR;    
	if(parport_set_bit(PA, PP_05)) return HW_ERROR;        
    }
    return 0;
}

static int stk200_set_mosi( char state )
{
    if( state )
	return parport_set_bit(PA, PP_07);
    else
	return parport_clr_bit(PA, PP_07);
}

static int stk200_set_sck( char state )
{
    if( state )
	return parport_set_bit(PA, PP_06);
    else
	return parport_clr_bit(PA, PP_06);
}

static int stk200_set_rst( char state ) // 09
{
    if( state )
	return parport_set_bit(PA, PP_09);
    else
	return parport_clr_bit(PA, PP_09);
}

static int stk200_set_led( char state ) // 08
{
    if( state )
	return parport_clr_bit(PA, PP_08);
    else
	return parport_set_bit(PA, PP_08);
}

static int stk200_sw_vcc( char state )
{
    int err;
    err  = stk200_set_led( state );
    err |= stk200_connect( state );
    return err;
}

static int stk200_reset()
{
    int err = 0;
    
    err =  parport_reset();
    err |= stk200_connect( 0 );    
    err |= stk200_set_mosi( 0 );
    err |= stk200_set_sck( 0 );
    err |= stk200_set_rst( 0 );
    err |= stk200_set_led( 0 );
    
    return err;
}

static int stk200_open(const char *ptr, int flags)
{
    if(parport_init(ptr, flags)) return HW_ERROR;
    return stk200_reset();
}

static int stk200_close()
{
    if( stk200_reset() == PP_ERROR ) return HW_ERROR;
    return parport_cleanup();
}

/*
    GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI 
*/
static void stk200_event(gui_xml_ev *ev, int value, const char *sval)
{
    gui_xml_val_str gxvs;
    const char *tmp;
    
    stk200_connect( 1 );    
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"MOSI", &gxvs);
    stk200_set_mosi(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"RST", &gxvs);
    stk200_set_rst(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"SCK", &gxvs);
    stk200_set_sck(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"LED", &gxvs);
    stk200_set_led(gxvs.ival);

    tmp = stk200_get_miso() ? "1":"0";
    gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_ENTRY, (char *)"MISO", (int *)tmp);    
}

static gui_xml_ifattr	stk200_if_attr[4] = {{ "chip", "none" }, { "programmer", "stk200" }, { "family", "none" }, { NULL, NULL }};

static int stk200_gui(geepro *gep, const char *chip_name, const char *family)
{
    // HW test page
    stk200_if_attr[0].val = chip_name;
    stk200_if_attr[2].val = family;
    gui_xml_build(GUI_XML(GUI(gep->gui)->xml), (char *)"file://./drivers/stk200.xml", (char *)"info,notebook", stk200_if_attr);
    gui_xml_register_event_func(GUI_XML(GUI(gep->gui)->xml), stk200_event);
    return 0;
}

/*
    API API API API API API API API API API API API API API API API API API API API API API API API API API API API 
*/
int stk200_api(en_hw_api func, int val, void *ptr)
{
    switch(func)
    {
	case HW_IFACE: return IFACE_LPT;
	case HW_NAME : DRIVER_NAME(ptr) = (char *)"STK200"; return HW_SUCCESS;
	case HW_OPEN : return stk200_open((const char *)ptr, val);
	case HW_CLOSE: return stk200_close();
	case HW_TEST : return stk200_test_connected();
	// GUI
	case HW_GINIT: return stk200_gui( GEEPRO(ptr), (const char *)"none", (const char *)"" );
	case HW_SET_CHIP: return stk200_gui( GEEPRO(ptr), GEEPRO(ptr)->chp->chip_name, GEEPRO(ptr)->chp->chip_family );
	// iface
	case HW_GET_DO	     : // alternative alias 
	case HW_GET_ISP_MISO : return stk200_get_miso();
	case HW_SET_DI	     : // alternative alias 
	case HW_SET_ISP_MOSI : return stk200_set_mosi( val );
	case HW_SET_CLK      : // alternative alias 
	case HW_SET_ISP_SCK  : return stk200_set_sck( val );
	case HW_SET_HOLD     : // alternative alias 
	case HW_SET_ISP_RST  : return stk200_set_rst( val );
	case HW_SET_ISP_LED  : return stk200_set_led( val );
	case HW_SW_VCC	     : return stk200_sw_vcc( val );
	default: return 0;
    }

    return -2;
}

driver_register_begin
    register_api( stk200_api );
driver_register_end

