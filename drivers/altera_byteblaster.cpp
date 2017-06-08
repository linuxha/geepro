/* $Revision: 1.2 $ */
/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2006 Krzysztof Komarnicki
 * Email: krzkomar@wp.pl
 *
 * Altera byteblaster cable driver
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

static int altera_bb_test_connected()
{
    if( parport_clr_bit(PA, PP_07) ) return 0;
    if( parport_get_bit(PB, PP_10) != 0 ) return 0;

    if( parport_set_bit(PA, PP_07) ) return 0;
    if( parport_get_bit(PB, PP_10) == 0 ) return 0;

    return 1;
}

static int altera_bb_sw_vcc( char state ) // AF line
{
    if( state )
	return parport_clr_bit(PC, PP_14);
    return parport_set_bit(PC, PP_14);
}

static int altera_bb_get_tdo() 
{
    return parport_get_bit(PB, PP_11);
}

static int altera_bb_get_nstat() 
{
    return parport_get_bit(PB, PP_13);
}

static int altera_bb_set_tdi(char state ) 
{
    if( state )
	return parport_set_bit(PA, PP_08);
    return parport_clr_bit(PA, PP_08);
}

static int altera_bb_set_tms(char state ) 
{
    if( state )
	return parport_set_bit(PA, PP_03);
    return parport_clr_bit(PA, PP_03);
}

static int altera_bb_set_tck(char state ) 
{
    if( state )
	return parport_set_bit(PA, PP_02);
    return parport_clr_bit(PA, PP_02);
}

static int altera_bb_reset()
{
    int err = 0;
    
    err =  parport_reset();
    err |= altera_bb_sw_vcc( 0 );	// tristate buffer off
    err |= altera_bb_set_tdi( 0 );
    err |= altera_bb_set_tms( 0 );
    err |= altera_bb_set_tck( 0 );
    
    return err;
}

static int altera_bb_open(const char *ptr, int flags)
{
    if(parport_init(ptr, flags)) return HW_ERROR;
    return altera_bb_reset();
}

static int altera_bb_close()
{
    if( altera_bb_reset() == PP_ERROR ) return HW_ERROR;
    return parport_cleanup();
}

/*
    GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI 
*/

static void altera_bb_event(gui_xml_ev *ev, int value, const char *sval)
{
    gui_xml_val_str gxvs;
    const char *tmp;
    
    altera_bb_sw_vcc( 1 );

    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TDI", &gxvs);
    altera_bb_set_tdi(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TCK", &gxvs);
    altera_bb_set_tck(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TMS", &gxvs);
    altera_bb_set_tms(gxvs.ival);
    tmp = altera_bb_get_tdo() ? "1":"0";
    gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_ENTRY, (char *)"TDO", (int *)tmp);    
    tmp = altera_bb_get_tdo() ? "1":"0";
    gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_ENTRY, (char *)"nSTAT", (int *)tmp);    

}

static gui_xml_ifattr	altera_bb_if_attr[4] = {{ "chip", "none" }, { "programmer", "xilinx3" }, { "family", "none" }, { NULL, NULL }};

static int altera_bb_gui(geepro *gep, const char *chip_name, const char *family)
{
    // HW test page
    altera_bb_if_attr[0].val = chip_name;
    altera_bb_if_attr[2].val = family;
    gui_xml_build(GUI_XML(GUI(gep->gui)->xml), (char *)"file://./drivers/altera_byteblaster.xml", (char *)"info,notebook", altera_bb_if_attr);
    gui_xml_register_event_func(GUI_XML(GUI(gep->gui)->xml), altera_bb_event);
    return 0;
}

/*
    API API API API API API API API API API API API API API API API API API API API API API API API API API API API 
*/
int altera_bb_api(en_hw_api func, int val, void *ptr)
{
    switch(func)
    {
	case HW_IFACE: return IFACE_LPT;
	case HW_NAME : DRIVER_NAME(ptr) = (char *)"JTAG Altera byteblaster"; return HW_SUCCESS;
	case HW_OPEN : return altera_bb_open((const char *)ptr, val);
	case HW_CLOSE: return altera_bb_close();
	case HW_TEST : return altera_bb_test_connected();
	// GUI
	case HW_GINIT: return altera_bb_gui( GEEPRO(ptr), (const char *)"none", (const char *)"" );
	case HW_SET_CHIP: return altera_bb_gui( GEEPRO(ptr), GEEPRO(ptr)->chp->chip_name, GEEPRO(ptr)->chp->chip_family );
	// iface
	case HW_GET_JTAG_TDO : return altera_bb_get_tdo();
	case HW_SET_JTAG_TDI : return altera_bb_set_tdi( val );
	case HW_SET_JTAG_TMS : return altera_bb_set_tms( val );
	case HW_SET_JTAG_TCK : return altera_bb_set_tck( val );
	case HW_GET_JTAG_NSTAT : return altera_bb_get_nstat();
	case HW_SET_JTAG_TRST : return 0; // not implemented in byteblaster
	case HW_SW_VCC	     : return altera_bb_sw_vcc( val );
	default: return 0;
    }

    return -2;
}

driver_register_begin
    register_api( altera_bb_api );
driver_register_end

