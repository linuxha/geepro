/* $Revision: 1.5 $ */
/* xml parser for gui
 * Copyright (C) 2007 Krzysztof Komarnicki
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

#ifndef __gui_xml_h__
#define __gui_xml_h__
#include "gui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GUI_XML(x)	((gui_xml*)x)

typedef struct _gui_xml_ev gui_xml_ev;
typedef void (*gui_xml_event)(gui_xml_ev *ev, int value, const char *sval);

/*
    atrybut/wartosc dla: <if atrybut1=wartosc1 atrybut2=wartosc2 ... > ... </if>
*/
typedef struct{
    const char *attr;
    const char *val;
} gui_xml_ifattr;

/*
    Return value for gui_xml_get_widget_value()
*/
typedef struct{
    int	 ival;
    char *sval;
} gui_xml_val_str;

/* 
  typ elementu tablicy przeszukiwania nazw dla gui_xml_trans_id()
*/
typedef struct
{
    const char *name;
    int  id;
} gui_xml_lt; 

struct _gui_xml_ev
{
    void *root_parent;
    char *id;
    int  type;
    void *widget;
    gui_xml_ev *next;
};

typedef struct
{
    void *parent;
    char suppress;  /* flaga stlumienia echa sygnalu */
    void *notebook;
    void *info;
    void *description;
    int  sw_size;
    gui_xml_event ev;
    gui_xml_ev *event;
} gui_xml;

typedef enum
{
    GUI_XML_INFO_ROOT = 1,
    GUI_XML_NOTEBOOK_ROOT,
    GUI_XML_BUTTON,
    GUI_XML_CHECK_BUTTON,
    GUI_XML_SPIN_BUTTON,
    GUI_XML_ENTRY
}gui_xml_ev_wg;

/* 
  pobiera informacje z gui i alokuje pamiec na strukture gui_xml, ktora powinna byc zwolniona 
*/
extern void *gui_xml_new(gui *g);

/* rejestruje funkcje zwrotna dla zdarzen */
extern void gui_xml_register_event_func(gui_xml *g, gui_xml_event ev);

/* 
    buduje GUI na podstawie lancucha (lub pliku, jesli xml zaczuna sie od file://) xml
    section - lista sekcji jakie maja byc wykorzystane
    chip_name - parametr warunkowy dla <if chip=chip_name>

     Na samym poczatku likwiduje poprzednie GUI    
    jesli blad zwraca -1, inaczej 0
*/
extern int gui_xml_build(gui_xml *g, char *xml, const char *section, gui_xml_ifattr *ifattr);

/*
    likwiduje GUI, nie zwalnia pamieci na strukture gui_xml
*/
extern void gui_xml_destroy(gui_xml *g);

/*
    zwraca id kontrolki po translacji lancucha id kontrolki na wartosc liczbowa wg tabeli lt
    jak nie znajdzie elementu zwraca -1, 

    size_lt - ilosc elementow tabeli 
*/
#define GUI_XML_TRANS_ID(ev, lt)	gui_xml_trans_id(ev, lt, sizeof(lt) / sizeof(lt[0]))
extern int gui_xml_trans_id(gui_xml_ev *ev, const gui_xml_lt *lt, int size_lt);

/*
    ustawienie danej kontrolki
*/
extern gui_xml_ev *gui_xml_set_widget_value(gui_xml *g, gui_xml_ev_wg wg, const char *id, int *val);

/*
    pobranie wartosci kontrolki
    jesli brak widgetu lub widget nie posiada wartosci zwraca -1
*/
extern void gui_xml_get_widget_value(gui_xml *g, gui_xml_ev_wg wg, const char *id, gui_xml_val_str *val);

/*
    zwraca strukture zdarzenia dla kontolki typu wg i danym id
*/
extern gui_xml_ev *gui_xml_event_lookup(gui_xml *g, const char *id, gui_xml_ev_wg wg);

#ifdef __cplusplus
} //extern "C"
#endif

#endif

