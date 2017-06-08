/* $Revision: 1.2 $ */
/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2006 Krzysztof Komarnicki
 * Email: krzkomar@wp.pl
 *
 * Simple JTAG noname cable 
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

static int xyz_sw_vcc( char state )
{
    if( state )
	return parport_set_bit(PC, PP_01);
    return parport_clr_bit(PC, PP_01);
}

static int xyz_get_tdo()
{
    return parport_get_bit(PB, PP_13);
}

static int xyz_set_tdi(char state )
{
    if( state )
	return parport_set_bit(PA, PP_04);
    return parport_clr_bit(PA, PP_04);
}

static int xyz_set_tms(char state ) 
{
    if( state )
	return parport_set_bit(PA, PP_02);
    return parport_clr_bit(PA, PP_02);
}

static int xyz_set_tck(char state )
{
    if( state )
	return parport_set_bit(PA, PP_03);
    return parport_clr_bit(PA, PP_03);
}

static int xyz_set_trst(char state )
{
    if( state )
	return parport_set_bit(PA, PP_05);
    return parport_clr_bit(PA, PP_05);
}

static int xyz_reset()
{
    int err = 0;
    
    err =  parport_reset();
    err |= xyz_sw_vcc( 0 );	// tristate buffer off
    err |= xyz_set_tdi( 0 );
    err |= xyz_set_tms( 0 );
    err |= xyz_set_tck( 0 );
    err |= xyz_set_trst( 0 );    
    return err;
}

static int xyz_open(const char *ptr, int flags)
{
    if(parport_init(ptr, flags)) return HW_ERROR;
    return xyz_reset();
}

static int xyz_close()
{
    if( xyz_reset() == PP_ERROR ) return HW_ERROR;
    return parport_cleanup();
}

/*
    GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI 
*/

static void xyz_event(gui_xml_ev *ev, int value, const char *sval)
{
    gui_xml_val_str gxvs;
    const char *tmp;
    
    xyz_sw_vcc( 1 );

    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TRST", &gxvs);
    xyz_set_trst(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TDI", &gxvs);
    xyz_set_tdi(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TCK", &gxvs);
    xyz_set_tck(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"TMS", &gxvs);
    xyz_set_tms(gxvs.ival);
    tmp = xyz_get_tdo() ? "1":"0";
    gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_ENTRY, (char *)"TDO", (int *)tmp);    
}

static gui_xml_ifattr	xyz_if_attr[4] = {{ "chip", "none" }, { "programmer", "JTAG noname" }, { "family", "none" }, { NULL, NULL }};

static int xyz_gui(geepro *gep, const char *chip_name, const char *family)
{
    // HW test page
    xyz_if_attr[0].val = chip_name;
    xyz_if_attr[2].val = family;
    gui_xml_build(GUI_XML(GUI(gep->gui)->xml), (char *)"file://./drivers/jtag_noname.xml", (char *)"info,notebook", xyz_if_attr);
    gui_xml_register_event_func(GUI_XML(GUI(gep->gui)->xml), xyz_event);
    return 0;
}

/*
    API API API API API API API API API API API API API API API API API API API API API API API API API API API API 
*/
int xyz_api(en_hw_api func, int val, void *ptr)
{
    switch(func)
    {
	case HW_IFACE: return IFACE_LPT;
	case HW_NAME : DRIVER_NAME(ptr) = (char *)"JTAG - unknown name"; return HW_SUCCESS;
	case HW_OPEN : return xyz_open((const char *)ptr, val);
	case HW_CLOSE: return xyz_close();
	case HW_TEST : return 1; // no way to test connected
	// GUI
	case HW_GINIT: return xyz_gui( GEEPRO(ptr), (const char *)"none", (const char *)"" );
	case HW_SET_CHIP: return xyz_gui( GEEPRO(ptr), GEEPRO(ptr)->chp->chip_name, GEEPRO(ptr)->chp->chip_family );
	// iface
	case HW_GET_JTAG_TDO : return xyz_get_tdo();
	case HW_SET_JTAG_TDI : return xyz_set_tdi( val );
	case HW_SET_JTAG_TMS : return xyz_set_tms( val );
	case HW_SET_JTAG_TCK : return xyz_set_tck( val );
	case HW_SET_JTAG_TRST : return xyz_set_trst( val );
	case HW_SW_VCC	     : return xyz_sw_vcc( val );
	default: return 0;
    }

    return -2;
}

driver_register_begin
    register_api( xyz_api );
driver_register_end

