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

static int funprog_get_miso()
{
    return parport_get_bit(PB, PP_10);
}

static int funprog_set_mosi( char state )
{
    if( state )
	return parport_set_bit(PA, PP_07);
    else
	return parport_clr_bit(PA, PP_07);
}

static int funprog_set_sck( char state )
{
    if( state )
	return parport_set_bit(PA, PP_08);
    else
	return parport_clr_bit(PA, PP_08);
}

static int funprog_set_rst( char state )
{
    if( state )
	return parport_set_bit(PA, PP_06);
    else
	return parport_clr_bit(PA, PP_06);
}

static int funprog_set_xtal( char state )
{
    if( state )
	return parport_set_bit(PA, PP_05);
    else
	return parport_clr_bit(PA, PP_05);
}

static int funprog_sw_vcc( char state )
{
    int err = 0;

    if( state ){
	err  = parport_set_bit( PA, PP_02 );
	err |= parport_set_bit( PA, PP_03 );
	err |= parport_set_bit( PA, PP_04 );
    } else {
	err  = parport_clr_bit( PA, PP_02 );
	err |= parport_clr_bit( PA, PP_03 );
	err |= parport_clr_bit( PA, PP_04 );    
    }
    return err;
}

static int funprog_reset()
{
    int err = 0;
    
    err =  parport_reset();
    err |= funprog_sw_vcc( 0 );    
    err |= funprog_set_mosi( 0 );
    err |= funprog_set_sck( 0 );
    err |= funprog_set_rst( 0 );
    err |= funprog_set_xtal( 0 );
    
    return err;
}

static int funprog_open(const char *ptr, int flags)
{
    if(parport_init(ptr, flags)) return HW_ERROR;
    return funprog_reset();
}

static int funprog_close()
{
    if( funprog_reset() == PP_ERROR ) return HW_ERROR;
    return parport_cleanup();
}

/*
    GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI 
*/
static void funprog_event(gui_xml_ev *ev, int value, const char *sval)
{
    gui_xml_val_str gxvs;
    const char *tmp;
    
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"VCC", &gxvs);
    funprog_sw_vcc(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"MOSI", &gxvs);
    funprog_set_mosi(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"RST", &gxvs);
    funprog_set_rst(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"SCK", &gxvs);
    funprog_set_sck(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"XTAL", &gxvs);
    funprog_set_xtal(gxvs.ival);

    tmp = funprog_get_miso() ? "1":"0";
    gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_ENTRY, (char *)"MISO", (int *)tmp);    
}

static gui_xml_ifattr	funprog_if_attr[4] = {{ "chip", "none" }, { "programmer", "funprog" }, { "family", "none" }, { NULL, NULL }};

static int funprog_gui(geepro *gep, const char *chip_name, const char *family)
{
    // HW test page
    funprog_if_attr[0].val = chip_name;
    funprog_if_attr[2].val = family;
    gui_xml_build(GUI_XML(GUI(gep->gui)->xml), (char *)"file://./drivers/funprog.xml", (char *)"info,notebook", funprog_if_attr);
    gui_xml_register_event_func(GUI_XML(GUI(gep->gui)->xml), funprog_event);
    return 0;
}

/*
    API API API API API API API API API API API API API API API API API API API API API API API API API API API API 
*/
int funprog_api(en_hw_api func, int val, void *ptr)
{
    switch(func)
    {
	case HW_IFACE: return IFACE_LPT;
	case HW_NAME : DRIVER_NAME(ptr) = (char *)"Funprog"; return HW_SUCCESS;
	case HW_OPEN : return funprog_open((const char *)ptr, val);
	case HW_CLOSE: return funprog_close();
	case HW_TEST : return 1;
	// GUI
	case HW_GINIT: return funprog_gui( GEEPRO(ptr), (const char *)"none", (const char *)"" );
	case HW_SET_CHIP: return funprog_gui( GEEPRO(ptr), GEEPRO(ptr)->chp->chip_name, GEEPRO(ptr)->chp->chip_family );
	// iface
	case HW_GET_DO	     : // alternative alias 
	case HW_GET_ISP_MISO : return funprog_get_miso();
	case HW_SET_DI	     : // alternative alias 
	case HW_SET_ISP_MOSI : return funprog_set_mosi( val );
	case HW_SET_CLK      : // alternative alias 
	case HW_SET_ISP_SCK  : return funprog_set_sck( val );
	case HW_SET_HOLD     : // alternative alias 
	case HW_SET_ISP_RST  : return funprog_set_rst( val );
	case HW_SET_AVR_XTAL : return funprog_set_xtal( val );
	case HW_SW_VCC	     : return funprog_sw_vcc( val );
	default: return 0;
    }

    return -2;
}

driver_register_begin
    register_api( funprog_api );
driver_register_end

