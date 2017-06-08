/* $Revision: 1.9 $ */
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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libxml/tree.h>
#include "gui_xml.h"

//#define DEBUG(fmt, p...)	printf("|-- DEBUG --> " fmt "\n", ## p)
#define DEBUG(fmt, p...)

extern const char *shared_geepro_dir;

static void gui_xml_parse_element(gui_xml *g, GtkWidget *wg, xmlDocPtr doc, xmlNode *cur, gui_xml_ifattr *parm);

static void gui_xml_event_default(gui_xml_ev *ev, int val, const char *sval)
{
    printf("{default dummy xml event handler}: widget: %i '%s' value: %i svalue: '%s'\n", ev->type, ev->id, val, sval);
}

static void gui_xml_gtk_event_handler(GtkWidget *wg, gui_xml_ev *gx)
{
    int val = 0;
    const char *sval;
    gui_xml *p = ((gui_xml*)gx->root_parent);

    if(p->suppress) return; /* stlumienie echa sygnalu */

    DEBUG("Event: %i %s", gx->type, gx->id);

    sval ="";
    switch(gx->type){
	case GUI_XML_BUTTON: val = 1; break;
	case GUI_XML_SPIN_BUTTON: val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gx->widget)); break;
	case GUI_XML_CHECK_BUTTON: val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gx->widget)); break;
	case GUI_XML_ENTRY: val = 0; sval = gtk_entry_get_text(GTK_ENTRY(gx->widget)); break;
    }

    p->ev(gx, val, sval);
}


static void gui_xml_event_destroy(gui_xml *g)
{
    gui_xml_ev *ev = g->event, *tmp;
    
    while( ev ){
	tmp = ev->next;
	DEBUG("Unregister: %i, id:'%s'", ev->type, ev->id);    
	if(ev->id) free(ev->id);
	free(ev);
	ev = tmp;
    }
    g->event = NULL;
}

static void gui_xml_signal_register(gui_xml *g, GtkWidget *wg, char *id, const char *signal, int type)
{
    gui_xml_ev *tmp, *i;

    DEBUG("Register:%i, id:'%s', signal:'%s'", type, id, signal);    

    if(gui_xml_event_lookup(g, id, type)){
	printf("Error: Duplicate id field '%s' for %i. Register widget event skipped.\n", id, type);
	return;
    }

    /* utworzenie nowego ogniwa */
    if(!(tmp = (gui_xml_ev*)malloc(sizeof(gui_xml_ev)))){
	printf("Error: out of memory!\n");
	return;
    }

    tmp->root_parent = g;
    tmp->widget = wg;
    tmp->type = type;
    tmp->id = NULL;
    if(id){
	if(!(tmp->id = strdup(id))){
	    printf("Error: out of memory!\n");
	    free(tmp);
	    return;
	}
    }
    tmp->next = NULL;

    /* dolaczenie do kolejki */
    if(!g->event)
	g->event = tmp;
    else {
	for(i = g->event; i->next; i = i->next);
	i->next = tmp;
    }
    /* rejestracja sygnalu */
    if(id && signal){

	g_signal_connect(G_OBJECT(wg), signal, G_CALLBACK(gui_xml_gtk_event_handler), tmp);
    }
}

static void gui_xml_container_add(gui_xml *g, xmlNode *cur, xmlDocPtr doc, GtkWidget *parent, GtkWidget *child, char recursive, gui_xml_ifattr *parm)
{
    int b0=0, b1=0, b2=0, b3=0, flagx, flagy, spx, spy;
    char *pos;

    if(recursive && cur->xmlChildrenNode){
	gui_xml_parse_element(g, child, doc, cur->xmlChildrenNode, parm );
    }
    
    b0 = b1 = b2 = b3 = spx = 0;
    pos = (char *)xmlGetProp(cur, (unsigned char *)"expand");
    if(pos) b0 = !strcmp(pos, "true");
    pos = (char *)xmlGetProp(cur, (unsigned char *)"fill");
    if(pos) b1 = !strcmp(pos, "true");
    pos = (char *)xmlGetProp(cur, (unsigned char *)"border");
    if(pos) b2 = atoi(pos), spx=1;
    pos = (char *)xmlGetProp(cur, (unsigned char *)"box_end");
    if(pos) b3 = !strcmp(pos, "true");
    if(spx){
	if(b3)
	    gtk_box_pack_end(GTK_BOX(parent), child, b0, b1, b2);
	else
	    gtk_box_pack_start(GTK_BOX(parent), child, b0, b1, b2);
	return;
    }

    b0 = b1 = b2 = b3 = 0;
    pos = (char *)xmlGetProp(cur, (unsigned char *)"expandx");
    if(pos) b0 = !strcmp(pos, "true");    
    pos = (char *)xmlGetProp(cur, (unsigned char *)"expandy");
    if(pos) b1 = !strcmp(pos, "true");
    pos = (char *)xmlGetProp(cur, (unsigned char *)"fillx");
    if(pos) b2 = !strcmp(pos, "true");
    pos = (char *)xmlGetProp(cur, (unsigned char *)"filly");
    if(pos) b3 = !strcmp(pos, "true");

    if(b0) b0 = GTK_EXPAND;
    if(b1) b1 = GTK_EXPAND;
    if(b2) b2 = GTK_FILL;
    if(b3) b3 = GTK_FILL;

    flagx = b0 | b2;
    flagy = b1 | b3;    

    spx = spy = 0;    
    pos = (char *)xmlGetProp(cur, (unsigned char *)"spacex");
    if(pos) spx = atoi(pos);    
    pos = (char *)xmlGetProp(cur, (unsigned char *)"spacey");
    if(pos) spy = atoi(pos);
    pos = (char *)xmlGetProp(cur, (unsigned char *)"pos");

    if(pos){
	sscanf(pos, "%i, %i, %i, %i", &b0, &b1, &b2, &b3);
	gtk_table_attach(GTK_TABLE(parent), child, b0, b1, b2, b3, flagx, flagy, spx, spy);
    }else
 	if(GTK_IS_BOX( parent ))
	    gtk_box_pack_start(GTK_BOX(parent), child, TRUE, TRUE, 0);
	else
	    gtk_container_add(GTK_CONTAINER(parent), child);
}

static GtkWidget *gui_xml_frame_new(xmlNode *cur)
{
    char *label, *align, *shadow;
    int a0=0, a1=0;
    int bn;
    GtkWidget *tmp;

    label = (char *)xmlGetProp(cur, (unsigned char *)"label");
    align = (char *)xmlGetProp(cur, (unsigned char *)"align");
    shadow = (char *)xmlGetProp(cur, (unsigned char *)"shadow");
    tmp = gtk_frame_new(label);
    if(align){
	sscanf(align, "%i, %i", &a0, &a1);
	gtk_frame_set_label_align(GTK_FRAME(tmp), (float)a0 / 100, (float)a1 / 100);
    }
    if(shadow){
	bn = GTK_SHADOW_NONE;
	if(!strcmp(shadow, "in")) bn = GTK_SHADOW_IN;
	 else
	if(!strcmp(shadow, "out")) bn = GTK_SHADOW_OUT;		
	 else
	if(!strcmp(shadow, "etched_in")) bn = GTK_SHADOW_ETCHED_IN;
	 else
	if(!strcmp(shadow, "etched_out")) bn = GTK_SHADOW_ETCHED_OUT;
	gtk_frame_set_shadow_type(GTK_FRAME(tmp), bn);
    }
    return tmp;
}

static GtkWidget *gui_xml_box_new(xmlNode *cur, char dir)
{
    char *arg0;
    int arg1, arg2;
    GtkWidget *wg;

    arg0 = (char *)xmlGetProp(cur, (unsigned char *)"expand");
    arg1 = FALSE; arg2 = 0;
    if(!strcmp(arg0, "true")) arg1 = TRUE;

    arg0 = (char *)xmlGetProp(cur, (unsigned char *)"spacing");
    if(arg0) arg2 = atoi(arg0);
    wg = gtk_box_new(dir ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, arg2);
    gtk_box_set_homogeneous(GTK_BOX(wg), arg1);
    return wg;
}

static GtkWidget *gui_xml_dipsw(gui_xml *g, xmlNode *cur)
{
    GtkWidget *wg0, *wg1, *wg2;
    char tmp[8], *arg0, *desc, rev = 0;
    int i=0, mask, len = 0;
    long set = 0;

    arg0 = (char *)xmlGetProp(cur, (unsigned char *)"len");
    if(arg0) len = atoi(arg0);
    arg0 = (char *)xmlGetProp(cur, (unsigned char *)"value");
    if(arg0) set = strtol(arg0, NULL, 0);
    desc = (char *)xmlGetProp(cur, (unsigned char *)"name");
    arg0 = (char *)xmlGetProp(cur, (unsigned char *)"reversed");
    if(arg0) rev = !strcmp(arg0, "true");
    
    wg0 = gtk_table_new(i,2, FALSE);
    memset(tmp, 0, 8);
    if(len <= 0) return NULL;
    
    if(rev)
	for(mask = 1 << (len-1), i = 0; i < len; i++,mask >>= 1){
	    sprintf(tmp, "%X", len - i);
	    wg1 = gtk_label_new(tmp);
	    wg2 = gtk_image_new_from_stock(set & mask ? GUI_DIPSW_ON : GUI_DIPSW_OFF, g->sw_size);
	    gtk_table_attach(GTK_TABLE(wg0), wg1, i,i+1, 0,1, 0,0, 0,0);
	    gtk_table_attach(GTK_TABLE(wg0), wg2, i,i+1, 1,2, 0,0, 0,0);
	}
    else
	for(mask = 1, i = 0; i < len; i++,mask <<= 1){
	    sprintf(tmp, "%X", i + 1);
	    wg1 = gtk_label_new(tmp);
	    wg2 = gtk_image_new_from_stock(set & mask ? GUI_DIPSW_ON : GUI_DIPSW_OFF, g->sw_size);
	    gtk_table_attach(GTK_TABLE(wg0), wg1, i,i+1, 0,1, 0,0, 0,0);
	    gtk_table_attach(GTK_TABLE(wg0), wg2, i,i+1, 1,2, 0,0, 0,0);
	}

    if(desc){
	wg1 = wg0;
	wg0 = gtk_frame_new(desc);
	gtk_frame_set_label_align(GTK_FRAME(wg0), 0.5, 0.5);
	gtk_container_add(GTK_CONTAINER(wg0), wg1);
    }

    wg2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_container_set_border_width(GTK_CONTAINER(wg2), 5);
    gtk_container_set_border_width(GTK_CONTAINER(wg0), 3);
    gtk_box_pack_start(GTK_BOX(wg2), wg0, TRUE, FALSE, 0);
    return wg2;
}

static GtkWidget *gui_xml_jumper(gui_xml *g, xmlNode *cur)
{
    GtkWidget *wg0, *wg1;
    char state, *idx_up, *idx_dn;

    state = 0;
    idx_up = (char *)xmlGetProp(cur, (unsigned char *)"set");
    if(idx_up) state = !strcmp(idx_up, "1");
    idx_up = (char *)xmlGetProp(cur, (unsigned char *)"name_up");
    idx_dn = (char *)xmlGetProp(cur, (unsigned char *)"name_dn");

    wg0 = gtk_table_new(1,3, FALSE);
    if(idx_up){
	wg1 = gtk_label_new(idx_up);
	gtk_table_attach(GTK_TABLE(wg0), wg1, 0,1, 0,1, 0,0, 0,0);
    }
    wg1 = gtk_image_new_from_stock(state ? GUI_DIPSW_ON : GUI_DIPSW_OFF, g->sw_size);
    gtk_table_attach(GTK_TABLE(wg0), wg1, 0,1, 1,2, 0,0, 0,0);
    if(idx_dn){
	wg1 = gtk_label_new(idx_dn);
	gtk_table_attach(GTK_TABLE(wg0), wg1, 0,1, 2,3, 0,0, 0,0);
    }
    return wg0;
}

static GtkWidget *gui_xml_table(gui_xml *g, xmlNode *cur)
{
    int x=0, y=0;
    char *tmp=NULL, eq=0;
    
    tmp = (char *)xmlGetProp(cur, (unsigned char *)"dim");
    if(!tmp) return NULL;
    sscanf(tmp, "%i, %i", &x, &y);

    tmp = (char *)xmlGetProp(cur, (unsigned char *)"equal");
    if(tmp) eq = !strcmp(tmp, "true");

    if(!x || !y) return NULL;
    return gtk_table_new(x, y, eq);
}


static GtkWidget *gui_xml_chbutton(gui_xml *g, xmlNode *cur)
{
    GtkWidget *tmp;
    GtkWidget *tmp_2, * tmp_3, *tmp_4, *tmp_1;
    GtkStyle *style;    
    long fs;
    char rl = 0, *arg, *id;

    if(!(id = (char *)xmlGetProp(cur, (unsigned char *)"id"))){
	return NULL;
    };
    arg = (char *)xmlGetProp(cur, (unsigned char *)"place");
    if(arg) rl = !strcmp(arg, "right");
    
    tmp = gtk_alignment_new( rl ? 1:0, 0.5,0,0);
    tmp_2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_container_add(GTK_CONTAINER(tmp), tmp_2);
    tmp_3 = gtk_check_button_new();
    gui_xml_signal_register(g, tmp_3, id, "toggled", GUI_XML_CHECK_BUTTON);
    tmp_4 = gtk_label_new((char *)xmlGetProp(cur, (unsigned char *)"label"));
    style = gtk_widget_get_style(tmp_4);
    fs = pango_font_description_get_size(style->font_desc);
    pango_font_description_set_size(style->font_desc, fs / 1.3);
    gtk_widget_modify_font(tmp_4,style->font_desc);
    pango_font_description_set_size(style->font_desc, fs );

    arg = (char *)xmlGetProp(cur, (unsigned char *)"sensitive");
    if(arg){ 
	if(!strcmp(arg, "false"))
	    gtk_widget_set_sensitive(GTK_WIDGET(tmp_3), 0);
    }
    if(rl){
	tmp_1 = tmp_4;
	tmp_4 = tmp_3;
	tmp_3 = tmp_1;
    }
    gtk_box_pack_start(GTK_BOX(tmp_2), tmp_3, FALSE,FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(tmp_2), tmp_4, FALSE,FALSE, 0); 

    return tmp; 
}

static GtkWidget *gui_xml_button(gui_xml *g, xmlNode *cur)
{
    GtkWidget *tmp;
    char *arg, *id;
        
    if(!(id = (char *)xmlGetProp(cur, (unsigned char *)"id"))){
	return NULL;
    };
    tmp = gtk_button_new_with_label((char *)xmlGetProp(cur, (unsigned char *)"label"));
    arg = (char *)xmlGetProp(cur, (unsigned char *)"sensitive");
    if(arg){ 
	if(!strcmp(arg, "false"))
	    gtk_widget_set_sensitive(GTK_WIDGET(tmp), 0);
    }
    gui_xml_signal_register(g, tmp, id, "clicked", GUI_XML_BUTTON);
    return tmp;
}

static GtkWidget *gui_xml_spinbutton(gui_xml *g, xmlNode *cur)
{
    GtkWidget *tmp1;
    GtkAdjustment *adj;
    char *arg, *id;
    int max=0;    

    if(!(id = (char *)xmlGetProp(cur, (unsigned char *)"id"))){
	return NULL;
    };

    arg = (char *)xmlGetProp(cur, (unsigned char *)"max");
    if(arg) max = strtol(arg, NULL, 0);
    adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0.0, max, 1, 16, 0));    
    tmp1 = gtk_spin_button_new(adj, 1, 0);
    arg = (char *)xmlGetProp(cur, (unsigned char *)"size");
    if(arg){ 
	int xx=0, yy=0;
	sscanf(arg, "%i, %i", &xx, &yy);
	gtk_widget_set_size_request(GTK_WIDGET(tmp1), xx, yy);
    }

    arg = (char *)xmlGetProp(cur, (unsigned char *)"sensitive");
    if(arg){ 
	if(!strcmp(arg, "false"))
	    gtk_widget_set_sensitive(GTK_WIDGET(tmp1), 0);
    }
    gui_xml_signal_register(g, tmp1, id, "value_changed", GUI_XML_SPIN_BUTTON);
    return tmp1;
}

static GtkWidget *gui_xml_entry(gui_xml *g, xmlNode *cur)
{
    GtkWidget *tmp;
    char *arg, *id;
    
    if(!(id = (char *)xmlGetProp(cur, (unsigned char *)"id"))){
	return NULL;
    };

    tmp = gtk_entry_new();
    arg = (char *)xmlGetProp(cur, (unsigned char *)"sensitive");
    if(arg){ 
	if(!strcmp(arg, "false"))
	    gtk_widget_set_sensitive(GTK_WIDGET(tmp), 0);
    }
    
    arg = (char *)xmlGetProp(cur, (unsigned char *)"editable");
    if(arg){ 
	if(!strcmp(arg, "false"))
	    gtk_editable_set_editable(GTK_EDITABLE(tmp), 0);
    }
    
    arg = (char *)xmlGetProp(cur, (unsigned char *)"size");
    if(arg){ 
	int xx;
	sscanf(arg, "%i", &xx);
	gtk_entry_set_width_chars(GTK_ENTRY(tmp), xx);
    }

    gui_xml_signal_register(g, tmp, id, "changed", GUI_XML_ENTRY);
    return tmp;
}

static GtkWidget *gui_xml_image(gui_xml *g, xmlNode *cur)
{
    char imagefile[256];
    unsigned int x = strlen(shared_geepro_dir);

    if( x > 255) x = 255;
    memset(imagefile, 0, 256);
    strncat(imagefile, shared_geepro_dir, x);
    strncat(imagefile, (char *)xmlGetProp(cur, (unsigned char *)"src"), 256 - strlen(imagefile));
    
    return gtk_image_new_from_file(imagefile);
}

static GtkWidget *gui_xml_label(gui_xml *g, xmlNode *cur)
{
    GtkWidget *tmp;
    char *arg;
    
    tmp = gtk_label_new((char *)xmlGetProp(cur, (unsigned char *)"text"));
    arg = (char *)xmlGetProp(cur, (unsigned char *)"align");
    if(arg){ 
	int xx, yy;
	sscanf(arg, "%i, %i", &xx, &yy);
	gtk_misc_set_alignment(GTK_MISC(tmp), (float)xx / 100, (float)yy / 100);
    }
    return tmp;
}

static void gui_xml_parse_element(gui_xml *g, GtkWidget *wg, xmlDocPtr doc, xmlNode *cur, gui_xml_ifattr *parm)
{
    char *arg0;
    int n, q, i;

    for(; cur != NULL; cur = cur->next){
	/* tagi kluczowe */
	if(!strcmp((char*)cur->name,"if")){
	    n = q = 0;
	    for(i = 0; parm[i].attr && parm[i].val; i++){
    		if((arg0 = (char *)xmlGetProp(cur, (unsigned char*)parm[i].attr))){
		    n++;
		    if(strstr(arg0, parm[i].val)) q++; 
		    /* jesli atrybut "", warunek nigdy nie spelniony */
		    if(!parm[i].val[0]){
			 q = n + 1;
			 break;
		    }
		}
	    }
	    if(n == q) gui_xml_parse_element(g, wg, doc, cur->xmlChildrenNode, parm );
	} 
	else if(!strcmp((char*)cur->name,"description")){
	    arg0 = (char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
	    gtk_label_set_text(GTK_LABEL(g->description), arg0);
	} 
	/**** elementy GUI ****/
	/* kontenery */
	else if(!strcmp((char*)cur->name,"frame"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_frame_new(cur), 1, parm);
	else if(!strcmp((char*)cur->name,"vbox"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_box_new(cur, 0), 1, parm);
	else if(!strcmp((char*)cur->name,"hbox"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_box_new(cur, 1), 1, parm);
	else if(!strcmp((char*)cur->name, "table"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_table(g, cur), 1, parm);
	/* kontrolki */
	else if(!strcmp((char*)cur->name,"jumper"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_jumper(g, cur), 0, NULL);
	else if(!strcmp((char*)cur->name, "dipswitch"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_dipsw(g, cur), 0, NULL);
	else if(!strcmp((char*)cur->name,"image"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_image(g, cur), 0, NULL);
	else if(!strcmp((char*)cur->name,"label"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_label(g, cur), 0, NULL);
	else if(!strcmp((char*)cur->name, "button"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_button(g, cur), 0, NULL);
	else if(!strcmp((char*)cur->name, "chbutton"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_chbutton(g, cur), 0, NULL);
	else if(!strcmp((char*)cur->name, "spinbutton"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_spinbutton(g, cur), 0, NULL);
	else if(!strcmp((char*)cur->name, "entry"))
	    gui_xml_container_add(g, cur, doc, wg, gui_xml_entry(g, cur), 0, NULL);
/* w przyszlosci dodac pozostale elementy w razie potrzeby */	

    }    
}

/* parsowanie glównego poziomu */
static void gui_xml_parser(gui_xml *g, xmlDocPtr doc, gui_xml_ifattr *parm, const char *section )
{
    xmlNode *cur;
    GtkWidget *tmp, *lab;

    if(!(cur = xmlDocGetRootElement(doc))) return;
    
    for(cur = cur->xmlChildrenNode; cur != NULL; cur = cur->next){
	if(!strcmp((char*)cur->name,"info") && strstr(section, "info")){
	    g_return_if_fail(g->info != NULL);
	    tmp = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	    gui_xml_signal_register(g, tmp, NULL, NULL, GUI_XML_INFO_ROOT);    
	    gui_xml_parse_element(g, GTK_WIDGET(tmp), doc, cur->xmlChildrenNode, parm);
	    gtk_table_attach_defaults(GTK_TABLE(g->info), tmp, 1,2, 0, 2);
	    gtk_widget_show_all(GTK_WIDGET(g->info));
	}
	if(!strcmp((char*)cur->name,"notebook") && strstr(section, "notebook")){
	    lab = gtk_label_new((char *)xmlGetProp(cur, (unsigned char *)"name"));
	    tmp = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	    gui_xml_signal_register(g, tmp, NULL, NULL, GUI_XML_NOTEBOOK_ROOT);
	    gui_xml_parse_element(g, tmp, doc, cur->xmlChildrenNode, parm);
	    gtk_notebook_append_page(GTK_NOTEBOOK(g->notebook), tmp, lab);
	    gtk_widget_show_all(GTK_WIDGET(g->notebook));
	}
    }    
}

/************************************************************************************************************************/

void gui_xml_destroy(gui_xml *g)
{
    gui_xml_ev *tmp;
    gtk_label_set_text(GTK_LABEL(g->description), "None");
    /* usuniecie drzewa widgetów - wywołąnie destroy dla kazdego korzenia */
    for(tmp = g->event; tmp; tmp = tmp->next)
				if(!tmp->id) gtk_widget_destroy(GTK_WIDGET(tmp->widget));
    /* usuniecie kolejki zarejestrowanych zdarzen */
    gui_xml_event_destroy(g);
}

int gui_xml_build(gui_xml *g, char *xml, const char *section, gui_xml_ifattr *parm)
{
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;

    if(!g) return -1;
    /* usuniecie istniejacego GUI */
    gui_xml_destroy(g);
    
    LIBXML_TEST_VERSION
    
    if(!(ctxt = xmlNewParserCtxt())){
	printf("Error {gui_xml.c} --> gui_xml_create(): Failed to allocate xml parser context.\n");
	return -1;
    };
    
    if(!strncmp(xml, "file://", 7))
	doc = xmlCtxtReadFile(ctxt, xml + 7, NULL, XML_PARSE_DTDVALID);
    else
	doc = xmlCtxtReadMemory(ctxt, xml, strlen(xml), "noname.xml", NULL, XML_PARSE_DTDVALID);

    if(doc == NULL){
	printf("Error {gui_xml.c} --> gui_xml_create(): Failed to parse xml string\n");
    } else {
	if(ctxt->valid)
	    gui_xml_parser(g, doc, parm, section);
	else 
	    printf("Error {gui_xml.c} --> gui_xml_create(): Failed to validate xml.\n");
	xmlFreeDoc(doc);
    }
    xmlFreeParserCtxt(ctxt);
    xmlCleanupParser();
// ?
//    xmlMemoryDump();
    return 0;
}

void *gui_xml_new(gui *g)
{
    gui_xml *tmp;

    g->xml = NULL;
    if(!(tmp = (gui_xml*)malloc(sizeof(gui_xml)))){
	printf("Error {gui_xml.c} --> gui_xml_new(): out of memory.\n");
	return NULL;
    };
    g->xml = tmp;
    tmp->parent = (void *)g;
    tmp->notebook = (void *)g->notebook;
    tmp->info = (void *)g->main_table;
    tmp->description = (void *)g->chip_desc;
    tmp->sw_size = g->icon_size;
    tmp->event = NULL;
    tmp->ev = gui_xml_event_default;
    tmp->suppress = 0;
    return tmp;
}

void gui_xml_register_event_func(gui_xml *g, gui_xml_event ev)
{
    if(!ev){
	printf("Error: gui_xml_register_event_func() ---> ev == NULL ");
	return;
    }
    g->ev = ev;
}

int gui_xml_trans_id(gui_xml_ev *ev, const gui_xml_lt *lt, int size_lt)
{
    int ret = -1, i;

    if(!ev){
	printf("Error: {gui_xml.c} gui_xml_trans_id() ---> event == NULL !\n");
	return -1;
    };

    for(i = 0; i < size_lt; i++)
	if(!strcmp(ev->id, lt[i].name)) return lt[i].id;

    return ret;
}

gui_xml_ev *gui_xml_set_widget_value(gui_xml *g, gui_xml_ev_wg wg, const char *id, int *val)
{
    gui_xml_ev *event;

    if(!val){
	printf("Error: {gui_xml.c} gui_xml_set_widget_value() ---> val == NULL !\n");
	return NULL;
    }

    if(!(event = gui_xml_event_lookup(g, id, wg))){
	printf("Error: {gui_xml.c} gui_xml_set_widget_value() ---> event == NULL !\n");
	return NULL;
    };

    g->suppress = 1;
    switch(wg){
	case (int)GUI_XML_SPIN_BUTTON: gtk_spin_button_set_value(GTK_SPIN_BUTTON(event->widget), *val); break;
	case GUI_XML_CHECK_BUTTON: gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(event->widget), *val ? 1:0); break;
	case GUI_XML_ENTRY: gtk_entry_set_text(GTK_ENTRY(event->widget), (char*)val); break;
	default: break;
    }
    g->suppress = 0;
    return event;
}

void gui_xml_get_widget_value(gui_xml *g, gui_xml_ev_wg wg, const char *id, gui_xml_val_str *ret)
{
    if( !ret ) return;
    ret->ival = -1;
    ret->sval = NULL;
    gui_xml_ev *event;
    
    if(!(event = gui_xml_event_lookup(g, id, wg))){
	printf("Error: {gui_xml.c} gui_xml_get_widget_value() ---> event == NULL !\n");
	return;
    };

    switch(wg){
	case GUI_XML_CHECK_BUTTON: ret->ival = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(event->widget)); break;
	case GUI_XML_SPIN_BUTTON: ret->ival = gtk_spin_button_get_value(GTK_SPIN_BUTTON(event->widget)); break;
	case GUI_XML_ENTRY: ret->sval = (char *)gtk_entry_get_text(GTK_ENTRY(event->widget)); break;
	default: break;
    }
}

gui_xml_ev *gui_xml_event_lookup(gui_xml *g, const char *id, gui_xml_ev_wg type)
{
    gui_xml_ev *ev;
    
    if(!g){
	printf("Error: {gui_xml.c} gui_xml_event_lookup() ---> g == NULL !\n");
	return NULL;
    }
    ev = g->event;
    for(; ev; ev = ev->next){
	if(ev->id && ev->type){
	    if(!strcmp(ev->id, id) && (ev->type == type)) return ev;
	}
    }
    return NULL;
}
