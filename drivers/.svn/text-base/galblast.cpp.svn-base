/* $Revision: 1.2 $ */
/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2006 Krzysztof Komarnicki
 * Email: krzkomar@wp.pl
 *
 * GALBLAST driver
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

static int galb_sw_vcc( char state )
{
    if( state )
	return parport_clr_bit(PC, PP_16);
    return parport_set_bit(PC, PP_16);
}

static char glb_volt_set(int v ) // calculate voltage set
{
    return v;
}

static int galb_set_vpp( int volt )
{
    char err, x;
    x = parport_get(PA);
    if( x == PP_ERROR) return HW_ERROR;
    err =  parport_set(PA, glb_volt_set(volt));
    hw_delay(2);
    err |= parport_clr_bit(PC, PP_14);
    hw_delay(2);
    err |= parport_set_bit(PC, PP_14);
    hw_delay(2);
    err |= parport_set(PA, x );
    hw_delay(2);
    return err;
}

static int galb_reset()
{
    int err = 0;
    
    err =  parport_reset();
    err |= parport_set(PA, 0);
    parport_set_bit(PC, PP_16);
    parport_set_bit(PC, PP_14);
    return err;
}

static int galb_open(const char *ptr, int flags)
{
    if(parport_init(ptr, flags)) return HW_ERROR;
    return galb_reset();
}

static int galb_close()
{
    if( galb_reset() == PP_ERROR ) return HW_ERROR;
    return parport_cleanup();
}

/* signals*/
static int galb_get_sdout()
{
    return parport_get_bit(PB, PP_10);
}

static int galb_set_sdin(char state)
{
    if( state )
	return parport_set_bit(PA, PP_02);
    return parport_clr_bit(PA, PP_02);
}

static int galb_set_sclk(char state)
{
    if( state )
	return parport_set_bit(PA, PP_09);
    return parport_clr_bit(PA, PP_09);
}

static int galb_set_stb(char state)
{
    if( state )
	return parport_clr_bit(PC, PP_01);
    return parport_set_bit(PC, PP_01);
}

static int galb_set_pv(char state)
{
    if( state )
	return parport_set_bit(PC, PP_17);
    return parport_clr_bit(PC, PP_17);
}

static int galb_set_ra(char ra)
{
    char x;

    x = parport_get(PA);
    if( x == PP_ERROR) return HW_ERROR;
    x = (x & 0x81) | ( (ra << 1) & 0x7e);
    return parport_set(PA, x);    
}

/*
    GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI 
*/

static void galb_event(gui_xml_ev *ev, int value, const char *sval)
{
    char ra;
    gui_xml_val_str gxvs;
    const char *tmp;

    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"ON_OFF", &gxvs);        
    galb_sw_vcc( gxvs.ival );    
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"SDIN", &gxvs);
    galb_set_sdin(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"SCLK", &gxvs);
    galb_set_sclk(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"STB", &gxvs);
    galb_set_stb(gxvs.ival);
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"PV", &gxvs);
    galb_set_pv(gxvs.ival);
    ra = 0;
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"RA0", &gxvs); ra |= gxvs.ival ? 0x01:0;
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"RA1", &gxvs); ra |= gxvs.ival ? 0x02:0;
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"RA2", &gxvs); ra |= gxvs.ival ? 0x04:0;
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"RA3", &gxvs); ra |= gxvs.ival ? 0x08:0;
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"RA4", &gxvs); ra |= gxvs.ival ? 0x10:0;
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, (char *)"RA5", &gxvs); ra |= gxvs.ival ? 0x20:0;
    galb_set_ra( ra );
    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_SPIN_BUTTON, (char *)"VOLTAGE", &gxvs);
    galb_set_vpp( gxvs.ival );
    tmp = galb_get_sdout() ? "1":"0";
    gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_ENTRY, (char *)"SDOUT", (int *)tmp);    
}

static gui_xml_ifattr	galb_if_attr[4] = {{ "chip", "none" }, { "programmer", "galblast" }, { "family", "none" }, { NULL, NULL }};

static int galb_gui(geepro *gep, const char *chip_name, const char *family)
{
    // HW test page
    galb_if_attr[0].val = chip_name;
    galb_if_attr[2].val = family;
    gui_xml_build(GUI_XML(GUI(gep->gui)->xml), (char *)"file://./drivers/galblast.xml", (char *)"info,notebook", galb_if_attr);
    gui_xml_register_event_func(GUI_XML(GUI(gep->gui)->xml), galb_event);
    return 0;
}

/*
    API API API API API API API API API API API API API API API API API API API API API API API API API API API API 
*/
int galb_api(en_hw_api func, int val, void *ptr)
{
    switch(func)
    {
	case HW_IFACE: return IFACE_LPT;
	case HW_NAME : DRIVER_NAME(ptr) = (char *)"GALBlast"; return HW_SUCCESS;
	case HW_OPEN : return galb_open((const char *)ptr, val);
	case HW_CLOSE: return galb_close();
	case HW_TEST : return 1; // no programmer identification
	// GUI
	case HW_GINIT: return galb_gui( GEEPRO(ptr), (const char *)"none", (const char *)"" );
	case HW_SET_CHIP: return galb_gui( GEEPRO(ptr), GEEPRO(ptr)->chp->chip_name, GEEPRO(ptr)->chp->chip_family );
	// iface
	case HW_SET_VPP	     : return galb_set_vpp( val );
	case HW_SW_VCC	     : return galb_sw_vcc( val );
	case HW_GET_GAL_SDOUT: return galb_get_sdout();
	case HW_SET_GAL_SDIN : return galb_set_sdin(val);
	case HW_SET_GAL_SCLK : return galb_set_sclk(val);
	case HW_SET_GAL_STB  : return galb_set_stb(val);
	case HW_SET_GAL_PV   : return galb_set_pv(val);
	case HW_SET_GAL_RA   : return galb_set_ra(val);
	default: return 0;
    }

    return -2;
}

driver_register_begin
    register_api( galb_api );
driver_register_end

