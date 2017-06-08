/* $Revision: 1.16 $ */
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

#ifndef __gui_h_
#define __gui_h_
#include "../src/geepro.h"
#include "../src/chip.h"
#include "../src/iface.h"

/* max ilosc przyciskó akcji */
#define GUI_MAX_ACTIONS		16

#define GUI(x)	((gui *)x)
#define MAX_CB_TABLE	1024

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _gui_action gui_action;

struct _gui_action
{
    char *name;
    void *root;
    void *action;
    void *widget;
    gui_action *next;
};

typedef struct
{
    char gui_run;	/* true if in gui event loop */
    void *wmain;        /* glówne okno */
    void *mb_dev;       /* lista ukladów do wyboru */
    void *toolbox;      /* toolbox dla doczepiania kontrolek operacji na ukladzie */
    void *notebook;     /* notebook, dla dodawania kart */    
    void *table;        /* tabela pakujaca, zawierjaca przycisk wyboru urzadzenia */
    void *main_table;	/* main pack table */

    void *prog_combox;  /* combox wyboru programatora */
    void *iface;        /* combox wyboru urzadzenia */
    void *dev_entry;    /* pole nazwy ukladu    */
    void *buffer_entry; /* pole rozmiaru bufora */
    void *crc_entry;    /* pole sumy CRC */
    void *file_entry;    /* pole sumy CRC */
    void *chip_desc;    /* pole opisu ukladu */
//    void *status_bar;   /* pasek statusu u dolu okna */

    void *bineditor;	/* edytor binarny bufora */
    int icon_size;	/* zarejestrowany rozmiar suwaków jumperów i przelaczników  */
    void *xml;		/* gui budowane dynamicznie przez sterownik programatora na podstawie xml-a */
    
    void *progress_win;    /* pasek postepu */
    void *progress_bar;    /* pasek postepu */

    /* lista wskaznikow do przycisków akcji */
    gui_action *action;

    /* MISC */
    long long fct;		/* Last loaded file creation/modify time to detect file change during program run */
    int	 cbtable[MAX_CB_TABLE];	/* checkbox values */
} gui;



typedef struct _sqw_gen sqw_gen;

typedef void (*gui_sqw_generator)(sqw_gen *);

struct _sqw_gen
{
    void *wper;
    void *wdut;
    void *wlen;
    void *wseq;
    gui_sqw_generator generator;
    gui *parent;
    int period;
    int duty;
    int len;
    int seq;
};

#define GUI_DIPSW_ON		"dpsw-off"
#define GUI_DIPSW_OFF		"dpsw-on"
#define GUI_JUMPER_UP		"dpsw-on"
#define GUI_JUMPER_DN		"dpsw-off"
#define GUI_JUMPER_OPEN		"dpsw-on"
#define GUI_JUMPER_CLOSE	"dpsw-off"


/* drobny font */
#define VIEW_FONT_DSC		"Luxi Mono 8"

#define MAIN_WINDOW_WIDTH	520
#define MAIN_WINDOW_HEIGHT	600
#define PICT_AREA_WIDTH		170
#define PICT_AREA_HEIGHT	170
#define SW_AREA_WIDTH		150
#define SW_AREA_HEIGHT		35
//#define PIX_BUF_VIEW		30
#define WG_VIEW_DA		31
#define WG_VIEWER_ADJ		32

#define CELL_SIZE_X		18
#define CELL_ADRESS_SIZE_X	60
#define CELL_SIZE_Y		16
#define CELL_TITLE_Y		22
#define PB_HEIGHT		25

/* wyciac oba */
#define GTK_WINDOW_DIALOG GTK_WINDOW_TOPLEVEL
#define TOOLBAR_PARAMS

#define	PIN_01	"1-Vpp"
#define	PIN_02	"2-A16"
#define	PIN_03	"3-A15"
#define	PIN_04	"4-A12"
#define	PIN_05	"5-A7"
#define	PIN_06	"6-A6"
#define	PIN_07	"7-A5"
#define	PIN_08	"8-A4"
#define	PIN_09	"9-A3"
#define	PIN_10	"10-A2"
#define	PIN_11	"11-A1"
#define	PIN_12	"12-A0"
#define	PIN_13	"13-D0"
#define	PIN_14	"14-D1"
#define	PIN_15	"15-D2"
#define	PIN_16	"16-GND"
#define	PIN_32	"Vcc-32"
#define	PIN_31	"WE-31"
#define PIN_30	"A17-30"
#define PIN_29	"A14-29"
#define PIN_28	"A13-28"
#define PIN_27	"A8-27"
#define PIN_26	"A9-26"
#define PIN_25	"A11-25"
#define PIN_24	"OE-24"
#define PIN_23	"A10-23"
#define PIN_22	"CE-22"
#define PIN_21	"D7-21"
#define PIN_20	"D6-20"
#define PIN_19	"D5-19"
#define PIN_18	"D4-18"
#define PIN_17	"D3-17"

#define ADD_PIN_R(nm,id, act)	gui_add_pin(gep, tmp, nm, 1, 1.3, id, gui_test_pin##act)
#define ADD_PIN_L(nm,id, act)	gui_add_pin(gep, tmp, nm, 0, 1.3, id, gui_test_pin##act)    
#define SET_HW_ADDR(adr)	g_print("Set addr: %i\n",adr);
#define SET_HW_DATA(data)	g_print("Set data: %i\n",data);
#define gui_error_box(wg, msg, opt...)	gui_dialog_box(wg,"[ER][TEXT]"msg"[/TEXT][BR] OK ", ##opt)

#define FO_RB_FIRST	1
#define FO_RB_NEXT	2
#define FO_LABEL	3
#define FO_CHECK	4
#define FO_ENTRY	5
#define FO_CPAGE	6

#define FO_NONE		0
#define FO_H_FIRST	1
#define FO_H_NEXT	2

//extern void gui_set_default(geepro *gep);
extern void gui_stat_rfsh(geepro*);
extern void gui_menu_setup(geepro*);
/*
    format fmt:
    [klasa][TEXT] format_tekstu [/TEXT][przycisk1]nazwa[przycisk2]nazwa ....
    
    klasa:
    [CR] - blad krytyczny
    [ER] - zwykly blad
    [WN] - ostrzezenie
    [IF] - informacja
    [HL] - pomoc
    [QS] - pytanie
    [AU] - autentykacja

    text:
     text wiadomosci sformatowany uproszczonym XML, (patrz: Pango Text Attribute Markup Language)

    format tekstu:
     format zgodny z printf
     
    przyciski:
    [BL] - przycisk dodany od lewej strony
    [BR] - przycisk dodany od prawej strony

    Wartosc zwracane: 
    -1 - blað
     0 - zamkniecie okna przyciskiem zamykania na belce
     1,2,... - kolejny numer przycisku ( względem definicji w formacie)

    Uwaga: Istotna wielkosc liter tokenów
*/
extern int  gui_dialog_box(geepro*, const char *fmt, ...);
extern void gui_progress_break(geepro*);
extern void gui_progress_bar_init(geepro*,const char *title, long range);
extern char gui_progress_bar_set(geepro*,long value, long max);
extern void gui_progress_bar_free(geepro*);
extern void gui_run(geepro *);
extern void gui_kill_me(geepro*);
extern char gui_cmp_pls(geepro*,int a,int b);
extern void gui_exit(geepro *gep);
extern void gui_set_statusbar(geepro *gep, char *tmp, char *fmt, ...);
extern unsigned long *gui_checkbox(geepro *gep, const char *fmt);
extern char gui_test_connection(geepro *);

/* uzywnae przez drivery */
extern void gui_clk_sqw(gui*, gui_sqw_generator);
///* uzywane przez pluginy ukladow */
//extern void gui_add_action(geepro *g, void *, const char *bt_name, const char *tip, void *cb);

#ifdef __cplusplus
 } // extern C
#endif

#endif
