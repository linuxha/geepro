/* $Revision: 1.15 $ */
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

#include "drivers.h"

static int addr_init_mask = 0x800000;    

/* tabela poszukiwan identyfikatorow kontrolek */
static const gui_xml_lt willem_lt[] = 
{
    /* piny na ZIF32 */
    {"pin_1", 1},   {"pin_32", 32},
    {"pin_2", 2},   {"pin_31", 31},
    {"pin_3", 3},   {"pin_30", 30},
    {"pin_4", 4},   {"pin_29", 29},
    {"pin_5", 5},   {"pin_28", 28},
    {"pin_6", 6},   {"pin_27", 27},
    {"pin_7", 7},   {"pin_26", 26},
    {"pin_8", 8},   {"pin_25", 25},
    {"pin_9", 9},   {"pin_24", 24},
    {"pin_10", 10}, {"pin_23", 23},
    {"pin_11", 11}, {"pin_22", 22},
    {"pin_12", 12}, {"pin_21", 21},
    {"pin_13", 13}, {"pin_20", 20},
    {"pin_14", 14}, {"pin_19", 19},
    {"pin_15", 15}, {"pin_18", 18},
    {"pin_16", 16}, {"pin_17", 17},
    /* ustawianie danej i adresu */
    {"address", 33},
    {"data", 34},
    {"rd_data_bt", 35},
    /* test seriala */
    {"pin_cs", 36},
    {"pin_clk", 37},
    {"pin_di", 38},
    {"clk_sqw", 39},
    {"pin_do_bt", 40}
};

enum{
    ADDRESS_SBT = 33,
    DATA_SBT,
    RD_DATA_BT,
    PIN_1_CS,
    PIN_2_CLK,
    PIN_3_DI,
    CLK_SQW_BT,
    PIN_DO_BT
};



/* tablice translacji numeru bitu na pin ukladu i na odwrot dla danej i adresu */
static const char addr_bit2pin[18] = {11,10,9,8,7,6,5,4,26,25,22,24,3,27,28,2,1,29};
static const char addr_pin2bit[32] = {0,16,15,12,7,6,5,4,3,2,1,0,0,0,0,0, 0,0,0,0,0,0,10,0,11,9,8,13,14,17,0,0};
static const char data_pin2bit[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0, 3,4,5,6,7,0,0,0,0,0,0,0,0,0,0,0};
static const char data_bit2pin[8] =  {12,13,14,16,17,18,19,20};

/* nazwy pinow dla adresu i danej */
static const char *address_pin_name[] = {"pin_12","pin_11","pin_10","pin_9","pin_8","pin_7","pin_6","pin_5","pin_27",
					 "pin_26","pin_23","pin_25","pin_4","pin_28","pin_29","pin_3","pin_2","pin_30"};
static const char *data_pin_name[] = {"pin_13","pin_14","pin_15","pin_17","pin_18","pin_19","pin_20","pin_21"};


static int _addr=0;
static int programmer_mode = 0;

extern int ___uid___;
extern const char *shared_drivers_xml_file;

/**************************************************************************************************************************/
/* sterownik */

static int willem_set_dip_sw(int dip){ DEBUG("[DM]Set Dip SW: 0x%x\n",dip); return 0; }
static int willem_set_vpp_voltage(int voltage){ DEBUG("[DM]Set VPP value: %f\n",voltage / 100.0); return 0; }
static int willem_set_vcc_voltage(int voltage){ DEBUG("[DM]Set VCC value: %f\n",voltage / 100.0); return 0; }

/* programmer control */
static int willem_vcc_on()
{
    addr_init_mask = 0x800000;    
    return parport_set_bit(PC,PP_16);
}

static int willem_vcc_off()
{
    addr_init_mask = 0x800000;    
    return parport_clr_bit(PC,PP_16);
}

static int willem_vpp_on()
{
    addr_init_mask = 0x800000;    
    return parport_set_bit(PC,PP_01);
}

static int willem_vpp_off()
{
    addr_init_mask = 0x800000;    
    return parport_clr_bit(PC,PP_01);
}

/* ZIF-32 - test settings */
static int willem_set_par_data_pin(char data)
{
    if(parport_clr_bit(PC,PP_14) == PP_ERROR) return HW_ERROR;
    return parport_set(PA,data);
}

static int willem_set_par_addr_pin(int addr)
{
    int mask, err=0;

    _addr = addr;
    err = parport_set_bit(PC, PP_14); /* przelaczenie multipleksera U7 na D i CLK*/
    mask = addr_init_mask;
    err |= parport_clr_bit(PA, PP_03 | PP_02); /* wyzerowanie D0 i D1 -> D i CLK przesuwnika */
    while(mask && !err){
//	timer_us_delay(TA_01);
	if(addr & mask) err |= parport_set_bit(PA, PP_03); else err |= parport_clr_bit(PA, PP_03);
//	timer_us_delay(TA_02);
	err |= parport_set_bit(PA, PP_02);
//	timer_us_delay(TA_03);
	err |= parport_clr_bit(PA, PP_02);
	mask = mask >> 1;
    }
    err |= parport_clr_bit(PC, PP_14); /* przelaczenie multipleksera U7 na dane */ 
    return err;
}

static int willempro2_set_par_addr_pin(int addr)
{
    // pro 2 pozwala przeslac do kazdego rejestru 4015 oddzielnie 
    int mask0,mask1,mask2, err=0;

    _addr = addr;
    err = parport_set_bit(PC, PP_14); /* przelaczenie multipleksera U7 na D i CLK*/
    mask0 = 0x000080;
    mask1 = 0x008000;
    mask2 = 0x800000;
    err |= parport_clr_bit(PA, PP_07 | PP_06 | PP_03 | PP_02); /* wyzerowanie D5,D4,D0 i D1 -> D i CLK przesuwnika */
    while(mask0 && !err){
	timer_us_delay(TA_01);
	if(addr & mask0) err |= parport_set_bit(PA, PP_03); else err |= parport_clr_bit(PA, PP_03);
	if(addr & mask1) err |= parport_set_bit(PA, PP_06); else err |= parport_clr_bit(PA, PP_06);
	if(addr & mask2) err |= parport_set_bit(PA, PP_07); else err |= parport_clr_bit(PA, PP_07);
	timer_us_delay(TA_02);
	err |= parport_set_bit(PA, PP_02);
	timer_us_delay(TA_03);
	err |= parport_clr_bit(PA, PP_02);
	mask0 = mask0 >> 1;
	mask1 = mask1 >> 1;
	mask2 = mask2 >> 1;
    }
    err |= parport_clr_bit(PC, PP_14); /* przelaczenie multipleksera U7 na dane */ 
    return err;
}

static int willem_inc_addr()
{
    if(_addr < -1) _addr++;
    return willem_set_par_addr_pin(_addr);
}

static int willem_dec_addr()
{
    if(_addr > 0) _addr--;
    return willem_set_par_addr_pin(_addr);
}

static int willem_rst_addr()
{
    _addr=0;
    return willem_set_par_addr_pin(_addr);
}

static int willem_set_we_pin(char _bool)
{
    if(_bool) return parport_set_bit(PC,PP_17); 
    return parport_clr_bit(PC,PP_17);
}

static int willem_set_oe_pin(char _bool)
{
    if(_bool) return parport_clr_bit(PC,PP_14); 
    return parport_set_bit(PC,PP_14);
}

static int willem_set_ce_pin(char _bool)
{
    if(programmer_mode == PRAGMA_CE_EQ_PGM ) return 0;
    if(_bool) return parport_set_bit(PC,PP_17); 
    return parport_clr_bit(PC,PP_17);
}

static int willem_set_pgm_pin(char _bool) // the same as CE, dependent on dip switch
{
    if(_bool) return parport_set_bit(PC,PP_17); 
    return parport_clr_bit(PC,PP_17);
}

static int willem_get_par_data_pin(void)
{
    unsigned char data,i;
    int err =0, x;    
    
    data = 0;
    err = parport_set_bit(PC,PP_14);
    err |= parport_set_bit(PA,PP_04);
    err |= parport_clr_bit(PA,PP_03);    
    timer_us_delay(TD_01);
    err |= parport_set_bit(PA,PP_03);    
    timer_us_delay(TD_02);
    err |= parport_clr_bit(PA,PP_04);        
    timer_us_delay(TD_03);
    err |= parport_set_bit(PA,PP_04);        
    err |= parport_clr_bit(PA,PP_03);        

    for(i = 0x80; i && !err; i >>= 1){
	timer_us_delay(TD_04);
	if(!(x = parport_get_bit(PB, PP_10))) data |= i; 
	if(x == PP_ERROR) err = HW_ERROR;	
	err |= parport_clr_bit(PA,PP_04);
	timer_us_delay(TD_05);
	err |= parport_set_bit(PA,PP_04);
    }

    return data | err;
}

/* serial 93Cxx */
static int willem_set_cs_pin(char _bool)
{
    if(_bool) return parport_set_bit(PC,PP_17); 
    return parport_clr_bit(PC,PP_17); 
}

static int willem_set_clk_pin(char _bool)
{
    if(parport_clr_bit(PC,PP_14) == PP_ERROR) return HW_ERROR;
    if(_bool) return parport_set_bit(PA,PP_03); 
    return parport_clr_bit(PA,PP_03);
}

static int willem_set_di_pin(char _bool)
{
    if(_bool) return parport_clr_bit(PA,PP_01); 
    return parport_set_bit(PA,PP_01);
}

static int willem_get_do_pin(void)
{
    return parport_get_bit(PB,PP_11);
}

/* pic & 24cxx */
static int willem_set_scl_pin(char val)
{ return willem_set_clk_pin(val); }

static int willem_set_sda_pin(char val)
{ return willem_set_di_pin(!val); }

static int willem_set_test_pin(char val)
{ return willem_set_cs_pin(val); }

static int willem_get_sda_pin(void)
{ return willem_get_do_pin(); }

static int willem_get_scl_pin(void)
{ 
    return (willem_get_par_data_pin() >> 1) & 1; // D1 signal
}

static int willem_set_pragma(int pragma)
{ 
    programmer_mode = pragma;
    return 0; 
}

static int willem_reset(void)
{
    int err=0;
    err = parport_reset();
    err |= willem_vpp_off();
    err |= willem_vcc_off();
    err |= willem_set_vpp_voltage(0);
    err |= willem_set_vcc_voltage(0);
    err |= willem_set_dip_sw(0);
    err |= willem_set_par_data_pin(0);
    err |= willem_set_par_addr_pin(0);
    err |= willem_set_we_pin(0);        
    err |= willem_set_oe_pin(0);
    err |= willem_set_ce_pin(0);
    err |= willem_rst_addr();
    err |= parport_reset();
    return err;
}

/**************************************************************************************************************************/
/* Test programatora */

void willem_sqw_gen(sqw_gen *gen)
{
    printf("generator\n");
}

#define W20(time, r) \
    willem_set_di_pin(r);\
    usleep(time);\
    if(willem_get_do_pin() == r) { willem_vcc_off(); return 0; }\
    usleep(time);\

static int willem_test_connection()
{
    willem_vcc_on();    
    willem_set_di_pin(0);
    usleep(10000);
    W20(1000, 1);
    W20(1000, 0);
    W20(1000, 1);
    W20(1000, 0);
    W20(1000, 1);
    W20(1000, 1);
    W20(1000, 1);
    W20(1000, 0);
    W20(1000, 0);
    usleep(1000);
    willem_vcc_off();
    return 1;
}

static void willem_event(gui_xml_ev *ev, int value, const char *sval)
{
    int pin, tmp, i, mask, z;
    char tmpstr[2];
    gui_xml_val_str gxvs;

    pin = GUI_XML_TRANS_ID(ev, willem_lt);
    /* jesli pin adresu */
    if(((pin > 1) && (pin < 13)) || (pin == 23) || ((pin > 24) && (pin < 31))){
	/* pobiera aktualny adres */
	gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_SPIN_BUTTON, (char *)"address", &gxvs);
	tmp = gxvs.ival;

	mask = (int)(1 << addr_pin2bit[pin - 1]);
	if(value)
	    tmp |= mask;
	else
	    tmp &= ~mask;	
	/* ustawia nowa wartosc dla spinbuttona */
	gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_SPIN_BUTTON, (char *)"address",  &tmp);
	/* ustaw port */
	hw_set_addr(tmp);
    }

    /* jesli pin danej */
    if(((pin > 12) && (pin < 16)) || ((pin > 16) && (pin < 22))){
	/* pobieera aktualna dana */
	gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_SPIN_BUTTON, (char *)"data", &gxvs);
	tmp = gxvs.ival;
	mask = (int)(1 << data_pin2bit[pin - 1]);
	if(value)
	    tmp |= mask;
	else
	    tmp &= ~mask;	
	/* ustawia nowa dana */
	gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_SPIN_BUTTON, (char *)"data",  &tmp);
	hw_set_data(tmp);
    }    

    switch(pin){
	/* piny ZIF 32 */
	case 1:  hw_sw_vpp(value); break;
	case 22: hw_set_ce(value); break;
	case 24: hw_set_oe(value); break;
	case 31: hw_set_we(value); break;
	case 32: hw_sw_vcc(value); break;
	/* pozostale kontrolki */
	case ADDRESS_SBT: /* Reczne ustawienie adresu za pomoca spin buttona */
		    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_SPIN_BUTTON, (char *)"address", &gxvs);
		    tmp = gxvs.ival;
		    /* ustawienie przyciskow symb piny */
		    for(i = 0, mask = 1; i < 18; i++, mask <<= 1){
			z = tmp & mask;
			gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, address_pin_name[i], &z);
		    }
		    hw_set_addr(tmp);
		    break;
	case DATA_SBT: /* Reczne ustawienie danej za pomoca spin buttona */
		    gui_xml_get_widget_value((gui_xml *)(ev->root_parent), GUI_XML_SPIN_BUTTON, (char *)"data", &gxvs);
		    tmp = gxvs.ival;
		    for(i = 0, mask = 1; i < 8; i++, mask <<= 1){
			z = tmp & mask;		
			gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, data_pin_name[i],  &z);
		    }
		    hw_set_data(tmp);
		    break;

	case RD_DATA_BT: tmp = hw_get_data();
		    gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_SPIN_BUTTON, (char *)"data",  &tmp); 
		    for(i = 0, mask = 1; i < 8; i++, mask <<= 1){
			z = tmp & mask;
			gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_CHECK_BUTTON, data_pin_name[i],  &z);
		    }
		    break;
	case PIN_1_CS:  hw_set_cs(value); break;
	case PIN_2_CLK: hw_set_clk(value); break;
	case PIN_3_DI:  hw_set_di(value); break;
	case PIN_DO_BT:
		tmpstr[0] = (hw_get_do() & 1) + '0'; 
		tmpstr[1] = 0;
		gui_xml_set_widget_value((gui_xml *)(ev->root_parent), GUI_XML_ENTRY, (char *)"pin_do_et", (int*)tmpstr);
		break;
	
	case CLK_SQW_BT: gui_clk_sqw(GUI(GUI_XML(ev->root_parent)->parent), willem_sqw_gen); break;

	default: break;
    }
}

/**************************************************************************************************************************/
/* inicjowanie sterownika */
static gui_xml_ifattr willem_if_attr[4] = {{"chip","none"},{"programmer","willem_40"},{"family",""}, {NULL, NULL}};

static void willem_set_gui_main(geepro *gep, const char *chip_name, const char *family, const char *programmer)
{
    willem_if_attr[0].val = chip_name;
    willem_if_attr[1].val = programmer;
    willem_if_attr[2].val = family;
    gui_xml_build(GUI_XML(GUI(gep->gui)->xml), (char *)shared_drivers_xml_file, (char *)"info,notebook", willem_if_attr);
    gui_xml_register_event_func(GUI_XML(GUI(gep->gui)->xml), willem_event);
}

static int willem_gui_init(void *ptr, const char *chip_name, const char *family, const char *programmer)
{
    willem_set_gui_main((geepro*)ptr, chip_name, family, programmer);
    return 0;
}

static int willem_open(const char *ptr, int flags)
{
    if(parport_init(ptr, flags) ) return HW_ERROR;
    return willem_reset();
}

static int pcb3_open(const char *ptr, int flags)
{
    if(parport_init(ptr, flags) == PP_ERROR) return HW_ERROR;
    return willem_reset();
}

static int willem_close(void)
{
    if(willem_reset() == PP_ERROR) return HW_ERROR;
    return parport_cleanup();
}

static void willem_set_addr_range(int val )
{
    addr_init_mask = val;
}

/**************************************************************************************************************************/

int willem_40_hardware_driver(en_hw_api funct, int val, void *ptr)
{
    switch(funct){
	/* ogólne */
	case HW_NAME:	  DRIVER_NAME(ptr) = (char *)"WILLEM 4.0"; return HW_SUCCESS;
	case HW_IFACE:	  return IFACE_LPT;
	case HW_GINIT:    return willem_gui_init(ptr, (char *)"none", (char *)"", (char *)"willem_40");
	case HW_SET_CHIP: return willem_gui_init(ptr, ((geepro *)ptr)->chp->chip_name, ((geepro *)ptr)->chp->chip_family, "willem_40");
	case HW_TEST:	  return willem_test_connection();
	case HW_RESET:    return willem_reset();
	case HW_OPEN:     return willem_open((const char *)ptr,val);
	case HW_CLOSE:    return willem_close();
	case HW_SET_VCC:  return willem_set_vcc_voltage(val);
	case HW_SET_VPP:  return willem_set_vpp_voltage(val);
	case HW_SW_VCC:	  if(val == 0) return willem_vcc_off(); else return willem_vcc_on();
	case HW_SW_VPP:	  if(val == 0) return willem_vpp_off(); else return willem_vpp_on();
	case HW_PRAGMA:   return willem_set_pragma( val );
	case HW_SW_DPSW:  return willem_set_dip_sw(val);
	case HW_SET_ADDR_RANGE: willem_set_addr_range( val ); return HW_SUCCESS;
	/* funkcje gniazda eprom */
	case HW_SET_DATA: return willem_set_par_data_pin(val);
	case HW_SET_ADDR: return willem_set_par_addr_pin(val);
	case HW_INC_ADDR: return willem_inc_addr();
	case HW_DEC_ADDR: return willem_dec_addr();
	case HW_RST_ADDR: return willem_rst_addr();
	case HW_GET_DATA: return willem_get_par_data_pin();
	case HW_SET_WE:   return willem_set_we_pin(val);
	case HW_SET_OE:   return willem_set_oe_pin(val);
	case HW_SET_CE:   return willem_set_ce_pin(val);
	case HW_SET_PGM:  return willem_set_pgm_pin(val);
	/* Serial SPI jak 93Cxx, 25Cxx*/
	case HW_SET_CS:	  return willem_set_cs_pin(val);
	case HW_SET_CLK:  return willem_set_clk_pin(val);
	case HW_SET_DI:	  return willem_set_di_pin(val);
	case HW_GET_DO:	  return willem_get_do_pin();
	/* Serial I2C jak 24Cxx, PIC */
	case HW_SET_HOLD: return willem_set_test_pin(val);
	case HW_SET_SCL:  return willem_set_scl_pin(val);
	case HW_GET_SCL:  return willem_get_scl_pin();
	case HW_SET_SDA:  return willem_set_sda_pin(val);
	case HW_GET_SDA:  return willem_get_sda_pin();
	case HW_DELAY:	  timer_us_delay(val); break;
	case HW_LATENCY:  timer_latency(val, ___uid___); break;
	default:  	  return HW_ERROR;
    }
    return -2;
}


/**************************************************************************************************************/

int willem4_hardware_driver(en_hw_api funct, int val, void *ptr)
{
    switch(funct){
	/* ogólne */
	case HW_NAME:	  DRIVER_NAME(ptr) = (char *)"PCB 3"; return 0;
	case HW_IFACE:	  return IFACE_LPT;
	case HW_GINIT:    return willem_gui_init(ptr,(char *)"none", (char *)"", (char *)"willem_pcb3");
	case HW_SET_CHIP: return willem_gui_init(ptr, ((geepro *)ptr)->chp->chip_name, ((geepro *)ptr)->chp->chip_family, "willem_pcb3");
	case HW_TEST:	  return willem_test_connection();
	case HW_RESET:    return willem_reset();
	case HW_OPEN:     return pcb3_open((const char *)ptr,val);
	case HW_CLOSE:    return willem_close();
	case HW_SET_VCC:  return willem_set_vcc_voltage(val);
	case HW_SET_VPP:  return willem_set_vpp_voltage(val);
	case HW_SW_VCC:	  if(val == 0) return willem_vcc_off(); else return willem_vcc_on();
	case HW_SW_VPP:	  if(val == 0) return willem_vpp_off(); else return willem_vpp_on();
	case HW_PRAGMA:   return willem_set_pragma( val );
	case HW_SW_DPSW:  return willem_set_dip_sw(val);
	case HW_SET_ADDR_RANGE: willem_set_addr_range( val ); return HW_SUCCESS;
	/* funkcje gniazda eprom */
	case HW_SET_DATA: return willem_set_par_data_pin(val);
	case HW_SET_ADDR: return willem_set_par_addr_pin(val);
	case HW_INC_ADDR: return willem_inc_addr();
	case HW_DEC_ADDR: return willem_dec_addr();
	case HW_RST_ADDR: return willem_rst_addr();
	case HW_GET_DATA: return willem_get_par_data_pin();
	case HW_SET_WE:   return willem_set_we_pin(val);
	case HW_SET_OE:   return willem_set_oe_pin(val);
	case HW_SET_CE:   return willem_set_ce_pin(val);
	case HW_SET_PGM:  return willem_set_pgm_pin(val);
	/* Serial SPI jak 93Cxx, 25Cxx*/
	case HW_SET_CS:	  return willem_set_cs_pin(val);
	case HW_SET_CLK:  return willem_set_clk_pin(val);
	case HW_SET_DI:	  return willem_set_di_pin(val);
	case HW_GET_DO:	  return willem_get_do_pin();
	/* Serial I2C jak 24Cxx, PIC */
	case HW_SET_HOLD: return willem_set_test_pin(val);
	case HW_SET_SCL:  return willem_set_scl_pin(val);
	case HW_SET_SDA:  return willem_set_sda_pin(val);
	case HW_GET_SDA:  return willem_get_sda_pin();
	case HW_DELAY:	  timer_us_delay(val); break;
	case HW_LATENCY:  timer_latency(val, ___uid___); break;
	default:  	  return HW_ERROR;
    }
    return -2;
}

/**************************************************************************************************************/

int willempro2_hardware_driver(en_hw_api funct, int val, void *ptr)
{
    switch(funct){
	/* ogólne */
	case HW_NAME:	  DRIVER_NAME(ptr) = (char *)"WILLEM PRO 2"; return 0;
	case HW_IFACE:	  return IFACE_LPT;
	case HW_GINIT:    return willem_gui_init(ptr,(char *)"none", (char *)"", (char *)"willem_pro2");
	case HW_SET_CHIP: return willem_gui_init(ptr, ((geepro *)ptr)->chp->chip_name, ((geepro *)ptr)->chp->chip_family, "willem_pro2");
	case HW_TEST:	  return willem_test_connection();
	case HW_RESET:    return willem_reset();
	case HW_OPEN:     return pcb3_open((const char *)ptr,val);
	case HW_CLOSE:    return willem_close();
	case HW_SET_VCC:  return willem_set_vcc_voltage(val);
	case HW_SET_VPP:  return willem_set_vpp_voltage(val);
	case HW_SW_VCC:	  if(val == 0) return willem_vcc_off(); else return willem_vcc_on();
	case HW_SW_VPP:	  if(val == 0) return willem_vpp_off(); else return willem_vpp_on();
	case HW_PRAGMA:   return willem_set_pragma( val );
	case HW_SW_DPSW:  return willem_set_dip_sw(val);
	case HW_SET_ADDR_RANGE: willem_set_addr_range( val ); return HW_SUCCESS;
	/* funkcje gniazda eprom */
	case HW_SET_DATA: return willem_set_par_data_pin(val);
	case HW_SET_ADDR: return willempro2_set_par_addr_pin(val);
	case HW_INC_ADDR: return willem_inc_addr();
	case HW_DEC_ADDR: return willem_dec_addr();
	case HW_RST_ADDR: return willem_rst_addr();
	case HW_GET_DATA: return willem_get_par_data_pin();
	case HW_SET_WE:   return willem_set_we_pin(val);
	case HW_SET_OE:   return willem_set_oe_pin(val);
	case HW_SET_CE:   return willem_set_ce_pin(val);
	case HW_SET_PGM:  return willem_set_pgm_pin(val);
	/* Serial SPI jak 93Cxx, 25Cxx*/
	case HW_SET_CS:	  return willem_set_cs_pin(val);
	case HW_SET_CLK:  return willem_set_clk_pin(val);
	case HW_SET_DI:	  return willem_set_di_pin(val);
	case HW_GET_DO:	  return willem_get_do_pin();
	/* Serial I2C jak 24Cxx, PIC */
	case HW_SET_HOLD: return willem_set_test_pin(val);
	case HW_SET_SCL:  return willem_set_scl_pin(val);
	case HW_SET_SDA:  return willem_set_sda_pin(val);
	case HW_GET_SDA:  return willem_get_sda_pin();
	case HW_DELAY:	  timer_us_delay(val); break;
	case HW_LATENCY:  timer_latency(val, ___uid___); break;
	default:  	  return HW_ERROR;
    }
    return -2;
}


/*************************************************************************************************************************/
/* Rejestracja driverów */

driver_register_begin

    register_api( willem_40_hardware_driver );
    register_api( willem4_hardware_driver );
    register_api( willempro2_hardware_driver );

driver_register_end

