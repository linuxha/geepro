/* $Revision: 1.2 $ */
/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2006 Krzysztof Komarnicki
 * Email: krzkomar@wp.pl
 *
 * Xilinx cable III driver
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

static int x3_test_connected()
{
    if( parport_get_bit(PB, PP_15) == 0 ){  // VCC sense
	printf("[ERR] No power !\n");
	return 0; 
    }

    if( parport_clr_bit(PA, PP_08) ) return 0;
    if( parport_get_bit(PB, PP_11) != 0 ) return 0;
    if( parport_get_bit(PB, PP_12) != 0 ) return 0;

    if( parport_set_bit(PA, PP_08) ) return 0;
    if( parport_get_bit(PB, PP_11) == 0 ) return 0;    
    if( parport_get_bit(PB, PP_12) == 0 ) return 0;

    return 1;
}

static int x3_sw_vcc( char state ) // CTRL line
{
    if( state )
	return parport_clr_bit(PA, PP_05);
    return parport_set_bit(PA, PP_05);
}

static int x3_set_tdo(char state ) // PROG line
{
    if( state )
	return parport_set_bit(PA, PP_06);
    return parport_clr_bit(PA, PP_06);
}

static int x3_get_tdo() // DONE line
{
    return parport_get_bit(PB, PP_13);
}

static int x3_set_tdi(char state ) // DIN line
{
    if( state )
	return parport_set_bit(PA, PP_02);
    return parport_clr_bit(PA, PP_02);
}

static int x3_set_tms(char state ) // TMS_IN line
{
    if( state )
	return parport_set_bit(PA, PP_04);
    return parport_clr_bit(PA, PP_04);
}

static int x3_set_tck(char state ) // CLK line
{
    if( state )
	return parport_set_bit(PA, PP_03);
    return parport_clr_bit(PA, PP_03);
}

static int x3_reset()
{
    int err = 0;
    
    err =  parport_reset();
    err |= x3_sw_vcc( 0 );	// tristate buffer off
    err |= x3_set_tdo( 1 );	// ready to read line TDO
    err |= x3_set_tdi( 0 );
    err |= x3_set_tms( 0 );
    err |= x3_set_tck( 0 );
    
    return err;
}

static int x3_open(const char *ptr, int flags)
{
    if(parport_init(ptr, flags)) return HW_ERROR;
    return x3_reset();
}

static int x3_close()
{
    if( x3_reset() == PP_ERROR ) return HW_ERROR;
    return parport_cleanup();
}

/*
    GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI 
*/

static void x3_event(gui_xml_ev *ev, int value, const char *sval)
{
    gui_xml_val_str gxvs;
    const char *tmp;
    
    x3_sw_vcc( 1 );

    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TDO_PRG", &gxvs);
    x3_set_tdo(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TDI", &gxvs);
    x3_set_tdi(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TCK", &gxvs);
    x3_set_tck(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TMS", &gxvs);
    x3_set_tms(gxvs.ival);
    tmp = x3_get_tdo() ? "1":"0";
    gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_ENTRY, (char *)"TDO", (int *)tmp);    
}

static gui_xml_ifattr	x3_if_attr[4] = {{ "chip", "none" }, { "programmer", "xilinx3" }, { "family", "none" }, { NULL, NULL }};

static int x3_gui(geepro *gep, const char *chip_name, const char *family)
{
    // HW test page
    x3_if_attr[0].val = chip_name;
    x3_if_attr[2].val = family;
    gui_xml_build(GUI_XML(GUI(gep->gui)->xml), (char *)"file://./drivers/xilinx_cable_iii.xml", (char *)"info,notebook", x3_if_attr);
    gui_xml_register_event_func(GUI_XML(GUI(gep->gui)->xml), x3_event);
    return 0;
}

/*
    API API API API API API API API API API API API API API API API API API API API API API API API API API API API 
*/
int x3_api(en_hw_api func, int val, void *ptr)
{
    switch(func)
    {
	case HW_IFACE: return IFACE_LPT;
	case HW_NAME : DRIVER_NAME(ptr) = (char *)"JTAG Xilinx cable III"; return HW_SUCCESS;
	case HW_OPEN : return x3_open((const char *)ptr, val);
	case HW_CLOSE: return x3_close();
	case HW_TEST : return x3_test_connected();
	// GUI
	case HW_GINIT: return x3_gui( GEEPRO(ptr), (const char *)"none", (const char *)"" );
	case HW_SET_CHIP: return x3_gui( GEEPRO(ptr), GEEPRO(ptr)->chp->chip_name, GEEPRO(ptr)->chp->chip_family );
	// iface
	case HW_GET_JTAG_TDO : return x3_get_tdo();
	case HW_SET_JTAG_TDO : return x3_set_tdo( val );
	case HW_SET_JTAG_TDI : return x3_set_tdi( val );
	case HW_SET_JTAG_TMS : return x3_set_tms( val );
	case HW_SET_JTAG_TCK : return x3_set_tck( val );
	case HW_SET_JTAG_TRST : return 0; // not implemented in cable III
	case HW_SW_VCC	     : return x3_sw_vcc( val );
	default: return 0;
    }

    return -2;
}

driver_register_begin
    register_api( x3_api );
driver_register_end

