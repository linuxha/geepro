/* $Revision: 1.22 $ */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "../intl/lang.h"
#include "../src/files.h"
#include "../src/buffer.h"
#include "../src/main.h"
#include "gui.h"
#include "gui_xml.h"
#include "../src/chip.h"
#include "../pixmaps/img_idx.h"
#include "../src/parport.h"
#include "../pixmaps/xpms.c"
#include "../drivers/hwdriver.h"
#include "../src/iface.h"
#include "bineditor.h"
#include "icons_xpm.c"
#include "../src/checksum.h"

typedef struct 
{
    GtkWidget *sb0, *sb1, *sb2;
    long size;
} gui_ins_file_str;


void gui_refresh_button(GtkWidget *, geepro *);
static void gui_checksum_recalculate( geepro * );

/***************************************************************************************************/
void gui_action_icon_set()
{
    GtkIconFactory *ifact = gtk_icon_factory_new();

//    gtk_icon_factory_add(ifact, "geepro-logo", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( LOGO_ICON )));
    gtk_icon_factory_add(ifact, "geepro-read-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( READ_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-read-eeprom-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( READ_EEPROM_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-sign-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( SIGN_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-write-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( WRITE_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-write-eeprom-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( WRITE_EEPROM_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-erase-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( ERASE_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-testblank-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( TESTBLANK_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-verify-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( VERIFY_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-verify-eeprom-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( VERIFY_EEPROM_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-lockbit-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( LOCKBIT_ACTION_ICON )));
    gtk_icon_factory_add(ifact, "geepro-lockbreak-action", gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( LOCKBREAK_ACTION_ICON )));

    gtk_icon_factory_add_default(ifact);
}

char gui_test_connection(geepro *gep)
{
    if(hw_test_conn()) return 0;
    gui_dialog_box(gep, "[ER][TEXT]Programmer unplugged![/TEXT][BR]OK", NULL, NULL);
    return -1;
}

static void gui_checksum_rfsh(geepro *gep)
{
    char tmp_str[40];
    sprintf(tmp_str, "0x%x", (unsigned int)gep->chp->checksum); 
    gtk_entry_set_text(GTK_ENTRY(GUI(gep->gui)->crc_entry), tmp_str);  
}

static void gui_checksum_recalculate(geepro *gep)
{
    if( !gep->chp ) return;
    if(gep->chp->dev_size == 0) return;
// to add algorithm selection
    gep->chp->checksum = checksum_calculate( CHECKSUM_ALG_LRC, gep->chp->dev_size, (unsigned char *)gep->chp->buffer, 0, gep->chp->dev_size - 1, 0, 0, 0, 0);
    gui_checksum_rfsh( gep );
}

void gui_stat_rfsh(geepro *gep)
{
    char tmp_str[40];

    if(gep->chp){
	gtk_entry_set_text(GTK_ENTRY(GUI(gep->gui)->dev_entry), gep->chp->chip_name);
	sprintf(tmp_str, "0x%x", (unsigned int)gep->chp->dev_size); 
	gtk_entry_set_text(GTK_ENTRY(GUI(gep->gui)->buffer_entry), tmp_str);  
	gui_checksum_rfsh( gep );
    } else {
	gtk_entry_set_text(GTK_ENTRY(GUI(gep->gui)->dev_entry), "--------");
	sprintf(tmp_str,"0x%x", 0); gtk_entry_set_text(GTK_ENTRY(GUI(gep->gui)->buffer_entry), tmp_str);  
	sprintf(tmp_str,"0x%x", 0); gtk_entry_set_text(GTK_ENTRY(GUI(gep->gui)->crc_entry), tmp_str);  
    }
}

static int gui_exit_program(GtkWidget *wg, geepro *gep, geepro *gep1)
{ 

    if(GTK_IS_WINDOW(wg) == TRUE) 
			    gep = gep1;

    if(gui_dialog_box(gep, "[QS][TEXT]"QUIT_MSG"[/TEXT][BR]  NO  [BR]  YES  ") == 2){
	 gui_kill_me( gep );
	 return FALSE;
    }
    return TRUE;
}

void gui_exit(geepro *gep)
{
    gui_exit_program(NULL, gep, gep);
}

static void gui_load_error_msg(geepro *gep, const char *fname, const char *err)
{
    if(err[0] != '!') 
	gui_error_box(gep, "Error loading file:\n%s\n%s", fname, err);
    else
	gui_dialog_box(gep, "[WN][TEXT]%s[/TEXT][BR]OK", err + 1); 
}

// Buffer offset
static void gui_insert_file_ranger0(GtkSpinButton *s, gui_ins_file_str *sb)
{
    double val = gtk_spin_button_get_value( s );
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(sb->sb1), 0, sb->size - val);
}
// Bytes count
static void gui_insert_file_ranger1(GtkSpinButton *s, gui_ins_file_str *sb)
{
//    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sb.sb1), sb.size);
//    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sb.sb1), sb.size);
}
// File offset
static void gui_insert_file_ranger2(GtkSpinButton *s, gui_ins_file_str *sb)
{
//    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sb.sb1), sb.size);
//    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sb.sb1), sb.size);
}

static void gui_load_file_(GtkWidget *w, geepro *gep, gboolean flag)
{ 
    char *tmp;
    GtkWidget *wg;    
    GtkFileFilter *filter;
    GtkWidget *wgi, *ca, *grid, *lb;
    gui_ins_file_str sb;
    gint result;
    long offset, size, fofs, flen;

    wg = gtk_file_chooser_dialog_new(
	    "Open file", GUI(gep->gui)->wmain, 
	    GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
	    GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, 
	    GTK_RESPONSE_ACCEPT, NULL
	);
    // file filters
    if(!flag){
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "hex");
	gtk_file_filter_add_pattern(filter, "*.hex");
	gtk_file_filter_add_pattern(filter, "*.HEX");
	gtk_file_filter_add_pattern(filter, "*.Hex");    
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wg), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "srec");
	gtk_file_filter_add_pattern(filter, "*.srec");
	gtk_file_filter_add_pattern(filter, "*.SREC");
	gtk_file_filter_add_pattern(filter, "*.Srec");
	gtk_file_filter_add_pattern(filter, "*.s19");
	gtk_file_filter_add_pattern(filter, "*.S19");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wg), filter);
    }

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "bin");
    gtk_file_filter_add_pattern(filter, "*.bin");
    gtk_file_filter_add_pattern(filter, "*.BIN");
    gtk_file_filter_add_pattern(filter, "*.Bin");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wg), filter);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "ALL");
    gtk_file_filter_add_pattern(filter, "*.*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wg), filter);

    if(!flag){
	tmp = NULL;
	if(store_get(&store, "LAST_OPENED_PATH", &tmp) == 0){
	    if( tmp ){
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wg), tmp);
		free(tmp);
	    }
	}

      tmp = NULL;
	if(store_get(&store, "LAST_OPENED_FILE", &tmp) == 0){
	    if( tmp ){
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(wg), tmp);
		free(tmp);
	    }
	}
    }
    if(gtk_dialog_run(GTK_DIALOG(wg)) == GTK_RESPONSE_ACCEPT){
	char *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wg));
	const char *err;

        if(!flag){
	    store_set(&store, "LAST_OPENED_PATH", gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wg)));
    	    store_set(&store, "LAST_OPENED_FILE", gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wg)));
	    gtk_entry_set_text(GTK_ENTRY(GUI(gep->gui)->file_entry), fname);
	    gtk_editable_set_position(GTK_EDITABLE(GUI(gep->gui)->file_entry), -1);
	}

	gtk_widget_destroy(wg);    

	fofs = -1; offset = -1; size = -1;
	if(flag){

	    flen = file_length( fname );
	    if( flen < 0){
		gui_load_error_msg(gep, fname, "");
		g_free( fname );
		return;
	    }
	    
	    wgi = gtk_dialog_new_with_buttons("Insert file to buffer", GUI(gep->gui)->wmain, 
		    GTK_DIALOG_DESTROY_WITH_PARENT, 
		    GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
		    GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
		    NULL
		  );
	    ca = gtk_dialog_get_content_area(GTK_DIALOG( wgi ));
	    grid = gtk_grid_new();
	    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
	    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
	    gtk_container_add(GTK_CONTAINER(ca), grid);

	    lb = gtk_label_new( DLG_INS_FILE_BUFFER_OFFSET );
	    gtk_grid_attach(GTK_GRID(grid), lb, 0, 0, 1, 1);
	    lb = gtk_label_new( DLG_INS_FILE_SIZE );
	    gtk_grid_attach(GTK_GRID(grid), lb, 0, 1, 1, 1);
	    lb = gtk_label_new( DLG_INS_FILE_OFFSET );
	    gtk_grid_attach(GTK_GRID(grid), lb, 0, 2, 1, 1);

	    sb.size = gep->chp->dev_size;
	    sb.sb0 = gtk_spin_button_new_with_range(0.0, (double)gep->chp->dev_size, 1.0);
	    gtk_grid_attach(GTK_GRID(grid), sb.sb0, 1, 0, 1, 1);
	    g_signal_connect(G_OBJECT(sb.sb0), "value-changed", G_CALLBACK(gui_insert_file_ranger0), &sb);
	    sb.sb1 = gtk_spin_button_new_with_range(0.0, (double)gep->chp->dev_size, 1.0);
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sb.sb1), (double)gep->chp->dev_size);
	    gtk_grid_attach(GTK_GRID(grid), sb.sb1, 1, 1, 1, 1);
	    g_signal_connect(G_OBJECT(sb.sb1), "value-changed", G_CALLBACK(gui_insert_file_ranger1), &sb);
	    sb.sb2 = gtk_spin_button_new_with_range(0.0, (double)flen, 1.0);
	    gtk_grid_attach(GTK_GRID(grid), sb.sb2, 1, 2, 1, 1);
	    g_signal_connect(G_OBJECT(sb.sb2), "value-changed", G_CALLBACK(gui_insert_file_ranger2), &sb);
	    gtk_widget_show_all( ca );
    
	    result = gtk_dialog_run(GTK_DIALOG( wgi ) );
	    offset = gtk_spin_button_get_value(GTK_SPIN_BUTTON( sb.sb0 ));
	    size = gtk_spin_button_get_value(GTK_SPIN_BUTTON( sb.sb1 ));
	    fofs = gtk_spin_button_get_value(GTK_SPIN_BUTTON( sb.sb2 ));

	    gtk_widget_destroy( wgi );
	    if( result != GTK_RESPONSE_ACCEPT ){
		g_free( fname );
		return;
	    }
	}

	err = file_load(gep, fname, fofs, offset, size);
	if(err)
	    gui_load_error_msg(gep, fname, err);
	if(!flag) file_get_time(gep, &GUI(gep->gui)->fct, fname);
	g_free(fname);
    } else
	gtk_widget_destroy(wg);    

    gui_checksum_recalculate( gep );
    gtk_widget_queue_draw( GUI(gep->gui)->wmain );
    gui_bineditor_redraw( GUI(gep->gui)->bineditor );
}

static void gui_load_file(GtkWidget *w, geepro *gep)
{
    gui_load_file_( w, gep, FALSE);
}

static void gui_load_file_at(GtkWidget *w, geepro *gep)
{
    gui_load_file_( w, gep, TRUE);    
}


static void gui_save_file(GtkWidget *w, geepro *gep)
{ 
    char *tmp;
    GtkWidget *wg;    
    GtkFileFilter *filter;

    wg = gtk_file_chooser_dialog_new(
	    "Save file", GUI(gep->gui)->wmain, 
	    GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_CANCEL,
	    GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
	    GTK_RESPONSE_ACCEPT, NULL
	);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wg), TRUE);

    // file filters
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "hex");
    gtk_file_filter_add_pattern(filter, "*.hex");
    gtk_file_filter_add_pattern(filter, "*.HEX");
    gtk_file_filter_add_pattern(filter, "*.Hex");    
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wg), filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "bin");
    gtk_file_filter_add_pattern(filter, "*.bin");
    gtk_file_filter_add_pattern(filter, "*.BIN");
    gtk_file_filter_add_pattern(filter, "*.Bin");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wg), filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "srec");
    gtk_file_filter_add_pattern(filter, "*.srec");
    gtk_file_filter_add_pattern(filter, "*.SREC");
    gtk_file_filter_add_pattern(filter, "*.Srec");
    gtk_file_filter_add_pattern(filter, "*.s19");
    gtk_file_filter_add_pattern(filter, "*.S19");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wg), filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "ALL");
    gtk_file_filter_add_pattern(filter, "*.*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wg), filter);

    tmp = NULL;
    if(store_get(&store, "LAST_SAVED_PATH", &tmp) == 0){
	if( tmp ){
	    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wg), tmp);
	    free(tmp);
	}
    }

    if(gtk_dialog_run(GTK_DIALOG(wg)) == GTK_RESPONSE_ACCEPT){
	char *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wg));
	store_set(&store, "LAST_SAVED_PATH", gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wg)));
	if(file_save(gep, fname)){
	    gui_error_box(gep, "Error saving file");
	}
	g_free(fname);
    }

    gtk_widget_destroy(wg);    

}

/***************************************************************************************************************************/
/* dodawanie przyciskow akcji do paska narzedziowego */

static void gui_test_file( geepro *gep )
{
    long long time;

    const char *fname = gtk_entry_get_text(GTK_ENTRY(GUI(gep->gui)->file_entry));    

    if( !fname ) return;
    if( !fname[0] ) return;

    file_get_time(gep, &time, fname);

    if( GUI(gep->gui)->fct < 0){
	GUI(gep->gui)->fct = time;
	return;
    }
    
    if( GUI(gep->gui)->fct != time){
	if(gui_dialog_box(gep, "[QS][TEXT]"RELOAD_QUESTION"[/TEXT][BR]  NO  [BR]  YES  ", fname) == 2){
	    gui_refresh_button(NULL, gep);
	    return;
	}
    }
}

static void gui_invoke_action(GtkWidget *wg, gui_action *ga)
{
    int x;
    geepro *gep = (geepro*)(ga->root);
    if(!ga->action){
	gui_dialog_box(
	    gep, 
	    "[ER][TEXT]"
	    "Plugin internal error. Action button registered, but action function = NULL."
	    "[/TEXT][BR] OK "
	);
	return;
    }
// program, compare -> test bin file for changes
    if( !strcmp(ga->name, "geepro-write-action") ) gui_test_file( gep );
    if( !strcmp(ga->name, "geepro-write-eeprom-action") ) gui_test_file( gep );
    if( !strcmp(ga->name, "geepro-verify-action") ) gui_test_file( gep );
    if( !strcmp(ga->name, "geepro-verify-eeprom-action") ) gui_test_file( gep );

    if( !gui_test_connection( gep ) ){
	x = ((chip_act_func)ga->action)(ga->root);
    } else 
	x = -1;
	
    if( x ) gui_dialog_box( gep, 
		"[ER][TEXT]"
	        "Error occured during performing action.\n Returned error: %i"
		"[/TEXT][BR] OK ", x
	    );
    gui_bineditor_redraw( ((gui *)(gep->gui))->bineditor );
    gui_checksum_recalculate( gep );
}

static int gui_add_bt_action(geepro *gep, const char *stock_name, const char *tip, chip_act_func action)
{
    gui_action *tmp, *new_tie;
    
    if(!(new_tie = malloc(sizeof(gui_action)))){
	printf("{gui.c} gui_add_br_action() --> out of memory.\n");
	return -1;
    }
    
    new_tie->next = NULL;

    new_tie->root = gep;
    new_tie->action = action;

    if(!(new_tie->name = malloc(strlen(stock_name) + 1))){
	printf("{gui.c} gui_add_br_action() --> out of memory.\n");
    } else {
	strcpy(new_tie->name, stock_name);
    }    

    new_tie->widget = gtk_tool_button_new_from_stock( stock_name );
    g_signal_connect(G_OBJECT(new_tie->widget), "clicked", G_CALLBACK(gui_invoke_action), new_tie);
    gtk_tool_item_set_tooltip_text( new_tie->widget, tip );
    gtk_toolbar_insert(GTK_TOOLBAR(GUI(gep->gui)->toolbox), new_tie->widget, -1);
    
    if(!GUI(gep->gui)->action){
	GUI(gep->gui)->action = new_tie;
	return 0;
    }    
    
    for(tmp = GUI(gep->gui)->action; tmp->next; tmp = tmp->next);
    
    tmp->next = new_tie;
    return 0;
}

static void gui_rem_bt_action(gui *g)
{
    gui_action *tmp, *x;
    tmp = g->action;
    while( tmp ){
	x = tmp->next;
	free( tmp->name );
	gtk_widget_destroy(tmp->widget);
	free(tmp);
	tmp = x;
    }
    g->action = NULL;
}

static int gui_add_action_list(chip_desc *desc, chip_action *act, void *ptr)
{
    gui_add_bt_action((geepro*)ptr, act->name, act->tip, act->action);
    return 0;
}

static int gui_add_action(geepro *gep, void *chip_str)
{
    int x = chip_list_action(gep->chp, gui_add_action_list, (void *)gep);
    gtk_widget_show_all(GTK_WIDGET(GUI(gep->gui)->toolbox));
    return x;
}

static void gui_chip_free(geepro *gep)
{
    if(gep->chp){
	gui_rem_bt_action(GUI(gep->gui));
	buffer_free(gep->chp);
    }
    gep->chp = NULL;
}

/***************************************************************************************************************************/

static void gui_chip_select(geepro *gep, const char *name)
{
    chip_desc *tmp, *old_chp;
    chip_plugins *plg = gep->ifc->plugins;

    /* Ustawienie nowego biezacego ukladu */
    if(!(tmp = chip_lookup_chip(plg, name))){
	gui_dialog_box( gep,
	    "[ER][TEXT]Missing chip description in queue.[/TEXT][BR] OK "
	);
	return;
    }
    old_chp = gep->chp;
    gep->chp = tmp; 

    /* ustawienie programatora pod wybrany uklad, test czy programator go obsluguje */
    if(hw_set_chip(gep) < 0){
	gui_dialog_box( gep,
	    "[ER][TEXT]Chip %s not supported by current programmer.[/TEXT][BR] OK ",
	    tmp->chip_name
	);
	gep->chp = old_chp;
	return;
    }

    /* wykasowanie menu, zwolnienie pamieci przez bufor */
    gui_chip_free(gep);
    gep->chp = tmp; 

    /* alokacja pamieci na bufor */
    if(buffer_alloc(gep->chp)){
	gui_dialog_box( gep,
	    "[ER][TEXT]Out of memory.[/TEXT][BR] OK "
	);
	gep->chp = NULL;
	return;
    }

    /* aktualizacja edycji bufora */
    gui_bineditor_set_buffer(GUI(gep->gui)->bineditor, tmp->dev_size, (unsigned char*)tmp->buffer);

    /* ustawienie przyciskow akcji na menu */    
    gui_add_action(gep, gep->chp );

    /* wykonanie autostartu dla danego ukladu, jesli zdefiniowano */
    if(gep->chp->autostart)
	     gep->chp->autostart(gep);

    gui_stat_rfsh(gep);
}


static void gui_device_sel(GtkWidget *wg, geepro *gep) 
{ 
    const char *name = NULL;
    
    /* jesli nie mozna pobrac nazwy ukladu to wyjdz */
    if( !gtk_bin_get_child(GTK_BIN(wg)) ) return;
    if(!GTK_IS_LABEL(gtk_bin_get_child(GTK_BIN(wg)))) return;
    /* pobierz nazwe wybranego ukladu */
    name = gtk_label_get_text( GTK_LABEL(gtk_bin_get_child(GTK_BIN(wg))) );

    store_set(&store, "LAST_CHIP_SELECTED", name);
    gui_chip_select(gep, name);
}

static void gui_rfsh_gtk(void)
{
    while(gtk_events_pending()) gtk_main_iteration();
}

static void gui_about(GtkWidget *wg, geepro *gep)
{ 
    GtkWidget *a;
    a = gtk_about_dialog_new();
    
    gtk_about_dialog_set_program_name( GTK_ABOUT_DIALOG( a ), EPROGRAM_NAME );    
    gtk_about_dialog_set_version( GTK_ABOUT_DIALOG( a ), EVERSION );    
    gtk_about_dialog_set_license_type( GTK_ABOUT_DIALOG( a ), GTK_LICENSE_GPL_2_0 );    
    gtk_about_dialog_set_website( GTK_ABOUT_DIALOG( a ), "http://"ESRCURL );    
    gtk_about_dialog_set_authors( GTK_ABOUT_DIALOG( a ), (const char *[])EAUTHORS );
    gtk_about_dialog_set_logo( GTK_ABOUT_DIALOG( a ), NULL );

    gtk_dialog_run( GTK_DIALOG(a) );    
    gtk_widget_destroy( a );
    
    
}

/************************************************************************************************************************/

static void *gui_submenu_add(void *op, char *name, void *gep)
{
    GtkWidget *p, *grp;
    
    grp = gtk_menu_item_new_with_label(name);
    gtk_menu_shell_append(GTK_MENU_SHELL(op), grp);
    p = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(grp), p);
    return p;
}

static void gui_menu_chip_add(chip_plugins *plg, void *sm, void *gep)
{
    GtkWidget *tt;
    tt = gtk_menu_item_new_with_label(plg->menu_sel->chip_name);
    gtk_menu_shell_append(GTK_MENU_SHELL(sm), tt);
    g_signal_connect(G_OBJECT(tt), "activate", G_CALLBACK(gui_device_sel), gep);
}

static void gui_device_menu_create(chip_plugins *plg, GtkWidget *wg, geepro *gep)
{
    chip_menu_create(plg, wg, gui_submenu_add, gui_menu_chip_add, gep);
}

/***************************************************************************************************************************/

static void gui_build_iface_menu(iface *ifc, int cl, char *name, char *dev, GtkWidget *wg)
{
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(wg), name);
}

static int gui_iface_sel(GtkWidget *wg, geepro *gep)
{
    char tmp[256];
    char *name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(wg));

    if(!name) return 0;
    gep->forbid = 0;
    if(iface_select_iface(gep->ifc, name)){
	gep->forbid = 1;
	gui_error_box(gep,"Open device error !!!\n Device inaccesible.\n");
	gtk_combo_box_set_active(GTK_COMBO_BOX(wg), gep->ifc->ifc_sel);
	return -1;
    }
    gep->ifc->ifc_sel = gtk_combo_box_get_active(GTK_COMBO_BOX(wg));
    gui_stat_rfsh(gep);
    test_hw(NULL, gep);
    sprintf(tmp,"%i", gep->ifc->ifc_sel);
    if(GUI(gep->gui)->gui_run) store_set(&store, "LAST_SELECTED_IFACE", tmp);
    return 0;
}

static GtkWidget *gui_iface_list(geepro *gep)
{
    GtkWidget *combox;

    combox = gtk_combo_box_text_new();
    iface_search(gep->ifc, gep->ifc->cl, (iface_cb)gui_build_iface_menu, GTK_COMBO_BOX(combox));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combox), gep->ifc->ifc_sel);
    g_signal_connect(G_OBJECT(combox), "changed", G_CALLBACK(gui_iface_sel), gep);
    return combox;
}

static void gui_add_iface_combox(geepro *gep)
{    
    GtkWidget *tmp = gui_iface_list(gep);
    gtk_table_attach(GTK_TABLE(GUI(gep->gui)->table), tmp, 2, 4, 1, 2, GTK_FILL | GTK_EXPAND, 0, 5, 5);
    GUI(gep->gui)->iface = tmp;


    gtk_widget_show(tmp);
}

static void gui_prog_sel(GtkWidget *wg, geepro *gep)
{
    char tmp[256];
    char *name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(wg));
    iface_prg_api api;    

    if(!name) return;
    if(!(api = iface_get_func(gep->ifc, name))){
	gui_error_box(NULL,MISSING_PROG_PLUGIN);
	gtk_combo_box_set_active(GTK_COMBO_BOX(wg), gep->ifc->prog_sel);
	return;
    }
    gep->ifc->prog_sel = gtk_combo_box_get_active(GTK_COMBO_BOX(wg));
    /* usuniecie listy interfejsów*/
    gtk_widget_destroy(GUI(gep->gui)->iface);

    hw_destroy(gep);
    ___hardware_driver___ = api;
    gep->ifc->cl = hw_get_iface();
    /* utworzenie wyboru interfaców */
    gui_add_iface_combox(gep);
    /* usuniecie menu */
    gui_xml_destroy(GUI(gep->gui)->xml);    
    /* wywolanie gui dla programatora */
    hw_gui_init(gep);

    /* inicjowanie portu, trzeba wysłac sygnał do interfejsu, ze został zmieniony i wymusiś zainicjowanie */
    g_signal_emit_by_name(G_OBJECT(GUI(gep->gui)->iface), "changed");    
    sprintf(tmp,"%i", gtk_combo_box_get_active(GTK_COMBO_BOX(wg)));
    if(GUI(gep->gui)->gui_run) store_set(&store, "LAST_SELECTED_PROGRAMMER", tmp);
}


static void gui_build_prg_menu(iface *ifc, char *name, GtkWidget *wg)
{
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(wg), name);
}


static GtkWidget *gui_prog_list(geepro *gep)
{
    GtkWidget *combox;

    combox = gtk_combo_box_text_new();

    iface_list_prg(gep->ifc, (iface_prg_func)gui_build_prg_menu, GTK_COMBO_BOX(combox));

    gtk_combo_box_set_active(GTK_COMBO_BOX(combox), gep->ifc->prog_sel);
    g_signal_connect(G_OBJECT(combox), "changed", G_CALLBACK(gui_prog_sel), gep);
    GUI(gep->gui)->prog_combox = combox;

    return combox;
}

/********************************************************************************************************************/
static void gui_config(GtkWidget *wg, geepro *gep)
{
    printf("config\n");
}

static void gui_set_default(geepro *gep)
{
    gtk_widget_show_all(GUI(gep->gui)->wmain);
    gui_stat_rfsh(gep);
}

#define GUI_STATUSBAR_ID_BUFFER 1

//void gui_set_statusbar(geepro *gep, char *tmp, char *str, ...)
//{	
//    va_list v;
//    
//    va_start(v, str);
//    vsprintf(tmp, str, v);
//    va_end(v);

//    gtk_statusbar_pop(GUI(gep->gui)->status_bar, GUI_STATUSBAR_ID_BUFFER);
//    gtk_statusbar_push(GUI(gep->gui)->status_bar, GUI_STATUSBAR_ID_BUFFER, tmp);
//}

/***********************************************************************************************************************/
/* dodaje ikonki do listy ikon i obrazkow aplikacji */
#define GUI_ICON_NEW(name, xpm)\
    pixb = gdk_pixbuf_new_from_xpm_data((const char **)xpm);\
    gis = gtk_icon_set_new_from_pixbuf(pixb);\
    gtk_icon_factory_add(gifac, name, gis);\
    gtk_icon_factory_add_default(gifac);

static void gui_add_images(geepro *gep)
{
    GtkIconFactory *gifac = gtk_icon_factory_new();
    GtkIconSet *gis;
    GdkPixbuf *pixb;    
    
    /* pozycje przelaczników */
    
    GUI(gep->gui)->icon_size = gtk_icon_size_register("dupa", 10, 25);

    GUI_ICON_NEW(GUI_DIPSW_ON, sw_on_xpm)
    GUI_ICON_NEW(GUI_DIPSW_OFF, sw_off_xpm)
    GUI_ICON_NEW(GUI_JUMPER_UP, sw_on_xpm)
    GUI_ICON_NEW(GUI_JUMPER_DN, sw_off_xpm)
    GUI_ICON_NEW(GUI_JUMPER_CLOSE, sw_on_xpm)
    GUI_ICON_NEW(GUI_JUMPER_OPEN, sw_off_xpm)

}

void gui_refresh_button(GtkWidget *wg, geepro *gep)
{
    const char *fname = gtk_entry_get_text(GTK_ENTRY(GUI(gep->gui)->file_entry));
    const char *err;


    err = file_load(gep, (char *)fname, -1, -1, -1);
    if( err ) 
	    gui_load_error_msg(gep, fname, err);
    else {
	gui_dialog_box(gep, "[IF][TEXT]File reloaded[/TEXT][BR]OK");
	
    }

    err = file_get_time(gep, &GUI(gep->gui)->fct, fname);
    if( err ) 
        gui_error_box(gep, "Error get creation time of file :\n%s\n%s", fname, err);    
    gui_checksum_recalculate( gep );
    gui_bineditor_redraw( ((gui *)(gep->gui))->bineditor );
}

void gui_help(geepro *gep)
{
printf("Help\n");
}

void gui_menu_setup(geepro *gep)
{
    char *tmp;
    GtkWidget *wg0, *wg1, *wg2, *wg3, *wg4;
    GtkToolItem *ti0;


    GUI(gep->gui)->fct = -1;
    gtk_init(&gep->argc, &gep->argv);
    gui_add_images(gep);

    GUI(gep->gui)->action = NULL;
    GUI(gep->gui)->wmain = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_icon(GTK_WINDOW(GUI(gep->gui)->wmain), gdk_pixbuf_new_from_xpm_data( LOGO_ICON ));
    g_signal_connect(G_OBJECT(GUI(gep->gui)->wmain), "delete_event", G_CALLBACK(gui_exit_program), gep);
    gtk_container_set_border_width(GTK_CONTAINER(GUI(gep->gui)->wmain), 1);
    gtk_window_set_title(GTK_WINDOW(GUI(gep->gui)->wmain), EPROGRAM_NAME " ver " EVERSION);
    gtk_widget_set_size_request(GUI(gep->gui)->wmain, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    gtk_widget_realize(GUI(gep->gui)->wmain);

    wg0 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);    
    gtk_container_add(GTK_CONTAINER(GUI(gep->gui)->wmain), wg0);

/* pasek Menu Bar */
    wg1 = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(wg0), wg1, FALSE, FALSE, 0);

/* menu File */
    wg2 = gtk_menu_item_new_with_label(MB_FILE);
    gtk_menu_shell_append(GTK_MENU_SHELL(wg1), wg2);
    wg3 = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(wg2), wg3);
    /* load file */
    wg2 = gtk_menu_item_new_with_label(MB_LOAD_FILE);
    gtk_menu_shell_append(GTK_MENU_SHELL(wg3), wg2);    
    g_signal_connect(G_OBJECT(wg2), "activate", G_CALLBACK(gui_load_file), gep);

    /* load file at specified address */
    wg2 = gtk_menu_item_new_with_label(MB_LOAD_FILE_AT);
    gtk_menu_shell_append(GTK_MENU_SHELL(wg3), wg2);    
    g_signal_connect(G_OBJECT(wg2), "activate", G_CALLBACK(gui_load_file_at), gep);

    /* save file */
    wg2 = gtk_menu_item_new_with_label(MB_SAVE_FILE);
    gtk_menu_shell_append(GTK_MENU_SHELL(wg3), wg2);    
    g_signal_connect(G_OBJECT(wg2), "activate", G_CALLBACK(gui_save_file), gep);
    /* spacer */
    wg2 = gtk_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(wg3), wg2);    
    /* exit */
    wg2 = gtk_menu_item_new_with_label(MB_EXIT_FILE);
    gtk_menu_shell_append(GTK_MENU_SHELL(wg3), wg2);    
    g_signal_connect(G_OBJECT(wg2), "activate", G_CALLBACK(gui_exit_program), gep);

/* Menu device */
    wg2 = gtk_menu_item_new_with_label(MB_DEVICE);    
    gtk_menu_shell_append(GTK_MENU_SHELL(wg1), wg2);
    wg3 = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(wg2), wg3);
    GUI(gep->gui)->mb_dev = wg3;

/* Menu Help */
    wg2 = gtk_menu_item_new_with_label( MB_HELP );    
    gtk_menu_shell_append(GTK_MENU_SHELL(wg1), wg2);
    wg3 = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(wg2), wg3);
    /* help */
    wg2 = gtk_menu_item_new_with_label(MB_DOCUMENTATION);
    gtk_menu_shell_append(GTK_MENU_SHELL(wg3), wg2);    
    g_signal_connect(G_OBJECT(wg2), "activate", G_CALLBACK(gui_help), gep);
    /* spacer */
    wg2 = gtk_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(wg3), wg2);    
    /* about */
    wg2 = gtk_menu_item_new_with_label(MB_ABOUT_FILE);
    gtk_menu_shell_append(GTK_MENU_SHELL(wg3), wg2);    
    g_signal_connect(G_OBJECT(wg2), "activate", G_CALLBACK(gui_about), gep);


/* toolbar */
    wg1 = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(wg1), GTK_TOOLBAR_ICONS);
    gtk_box_pack_start(GTK_BOX(wg0), wg1, FALSE, FALSE, 0);
    GUI(gep->gui)->toolbox = wg1;

    // static toolbar items
    ti0 = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
    g_signal_connect(G_OBJECT(ti0), "clicked", G_CALLBACK(gui_load_file), gep);
    gtk_tool_item_set_tooltip_text( ti0, OPEN_FILE_TIP);
    gtk_toolbar_insert(GTK_TOOLBAR(wg1), ti0, -1);
    ti0 = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    g_signal_connect(G_OBJECT(ti0), "clicked", G_CALLBACK(gui_save_file), gep);
    gtk_tool_item_set_tooltip_text( ti0, SAVE_FILE_TIP);
    gtk_toolbar_insert(GTK_TOOLBAR(wg1), ti0, -1);
    ti0 = gtk_tool_button_new_from_stock( GTK_STOCK_PREFERENCES );
    g_signal_connect(G_OBJECT(ti0), "clicked", G_CALLBACK(gui_config), gep);
gtk_widget_set_sensitive(GTK_WIDGET(ti0), FALSE);
    gtk_tool_item_set_tooltip_text( ti0, CONFIG_TIP);
    gtk_toolbar_insert(GTK_TOOLBAR(wg1), ti0, -1);
    ti0 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(wg1), ti0, -1);

/* Notebook */
    wg1 = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(wg1), GTK_POS_TOP);
    gtk_box_pack_start(GTK_BOX(wg0), wg1, TRUE, TRUE, 0);
    GUI(gep->gui)->notebook = wg1;

/* ------------------------------------------- strony NOTEBOOKA ----------------------------------------------------------- */
/* ======================================== */
/* --> notebook page 1 'strona glowna' <--- */
/* ======================================== */
    wg2 = gtk_table_new(2, 2, FALSE); /* tabela pakujaca karty glownej */
    GUI(gep->gui)->main_table = wg2;
    wg3 = gtk_label_new(LAB_NOTE_1);
    gtk_notebook_append_page(GTK_NOTEBOOK(wg1), wg2, wg3);
/* Ramka ukladu */
    wg1 = gtk_frame_new(MB_DEVICE);
    gtk_container_set_border_width(GTK_CONTAINER(wg1), 3);
    gtk_table_attach_defaults(GTK_TABLE(wg2), wg1,  0, 1, 0, 1);
    /* tabela pakujaca opis ukladu i bufor */
    wg3 = gtk_table_new( 2, 6, FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(wg3), 3);
    gtk_container_add(GTK_CONTAINER(wg1), wg3);
    /* Nazwa wybranego ukladu */    
    wg1 = gtk_label_new(DEVICE_ENTRY_LB);
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach(GTK_TABLE(wg3), wg1,  0,1,0,1, GTK_FILL, 0, 0,0);
    wg1 = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(wg1), FALSE);
    gtk_table_attach(GTK_TABLE(wg3), wg1,  1,2,0,1, GTK_FILL | GTK_EXPAND, 0, 10,0);
    GUI(gep->gui)->dev_entry = wg1;
    /* Rozmiar bufora/pamieci */    
    wg1 = gtk_label_new(SIZE_HEX_LB);
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach(GTK_TABLE(wg3), wg1,  0,1,1,2, GTK_FILL, 0, 0,0);
    wg1 = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(wg1), FALSE);
    gtk_table_attach(GTK_TABLE(wg3), wg1,  1,2,1,2, GTK_FILL | GTK_EXPAND, 0, 10,0);
    GUI(gep->gui)->buffer_entry = wg1;
    /* Suma CRC */    
    wg1 = gtk_label_new(CHECKSUM_LB);
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach(GTK_TABLE(wg3), wg1,  0,1,2,3, GTK_FILL, 0, 0,0);
    wg1 = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(wg1), FALSE);
    GUI(gep->gui)->crc_entry = wg1;
    gtk_table_attach(GTK_TABLE(wg3), wg1,  1,2,2,3, GTK_FILL | GTK_EXPAND, 0, 10,0);

    /* Nazwa pliku */    
    wg1 = gtk_label_new(FILE_LB);
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach(GTK_TABLE(wg3), wg1,  0,1,3,4, GTK_FILL, 0, 0,0);
    wg1 = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(wg1), FALSE);
    GUI(gep->gui)->file_entry = wg1;
    tmp = NULL;
    if(!store_get(&store, "LAST_OPENED_FILE", &tmp)){
	if(tmp){
	    gtk_entry_set_text(GTK_ENTRY(wg1), tmp);
	    gtk_editable_set_position(GTK_EDITABLE(wg1), -1);
	}
    }
    wg4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(wg4), wg1,  TRUE, TRUE, 0);
    wg1 = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(wg4), wg1, FALSE ,FALSE, 0);    
    gtk_table_attach(GTK_TABLE(wg3), wg4,  1,2,3,4, GTK_FILL | GTK_EXPAND, 0, 10,0);
    wg4 = gtk_image_new_from_stock("gtk-refresh", GTK_ICON_SIZE_BUTTON);    
    gtk_container_add(GTK_CONTAINER(wg1), wg4);
    gtk_widget_set_tooltip_text(wg1, TEXT(GUI_RELOAD));
    g_signal_connect(G_OBJECT(wg1), "pressed", G_CALLBACK(gui_refresh_button), gep);

    /* opis ukladu */
    wg1 = gtk_frame_new(CHIP_DESCRIPTION);
    gtk_table_attach_defaults(GTK_TABLE(wg3), wg1,  0, 3, 4, 5);
    wg3 = gtk_label_new(TXT_MISSING);
    gtk_container_add(GTK_CONTAINER(wg1), wg3);
    GUI(gep->gui)->chip_desc = wg3;

/* Ramka programatora */
    /* opcje programatora */
    wg1 = gtk_frame_new(FR_NB_04_TITLE);
    gtk_container_set_border_width(GTK_CONTAINER(wg1), 3);
    gtk_table_attach(GTK_TABLE(wg2), wg1,  0, 1, 1, 2,  GTK_FILL | GTK_EXPAND,0, 0,0);
    wg3 = gtk_table_new(3, 4, FALSE);
    GUI(gep->gui)->table = wg3;
    gtk_container_add(GTK_CONTAINER(wg1), wg3);    
    wg1 = gtk_label_new(TXT_PROGRAMMER);
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach( GTK_TABLE(wg3), wg1, 0, 2, 0, 1,  GTK_FILL, 0, 5, 5);
    wg1 = gtk_label_new(TXT_INTERFACE);
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach( GTK_TABLE(wg3), wg1, 0, 2, 1, 2,  GTK_FILL, 0, 5, 5);
    wg1 = gtk_button_new_with_label("Test Connection");
    g_signal_connect(G_OBJECT(wg1), "clicked", G_CALLBACK(test_hw), gep);
    gtk_table_attach(GTK_TABLE(wg3), wg1,  3, 4, 2, 3,  GTK_FILL, 0 , 5, 5);
    gtk_table_attach(GTK_TABLE(wg3), wg1 = gui_prog_list(gep),  2, 4, 0, 1, GTK_FILL | GTK_EXPAND, 0, 5, 5);
    gui_add_iface_combox(gep);

/* ======================================= */
/* -----> notebook page 2 'bufor' <------- */
/* ======================================= */
    wg1 = GUI(gep->gui)->notebook;
    wg0 = gui_bineditor_new(GUI(gep->gui)->wmain);
    gui_bineditor_set_icon( GUI_BINEDITOR(wg0), LOGO_ICON );
    wg3 = gtk_label_new(TXT_BUFFER);
    gtk_notebook_append_page(GTK_NOTEBOOK(wg1), wg0, wg3);
    GUI(gep->gui)->bineditor = wg0;

/* Koniec inicjowania Gui */
    gui_set_default(gep);
    gui_xml_new(GUI(gep->gui)); /* zainicjowanie struktury gui_xml */
}

void gui_run(geepro *gep)
{
    char *tmp;
    GUI(gep->gui)->gui_run = 0;

    gui_action_icon_set();

    gtk_notebook_set_current_page(GTK_NOTEBOOK(GUI(gep->gui)->notebook), 0);
    gui_device_menu_create(gep->ifc->plugins, GUI(gep->gui)->mb_dev, gep);
    gtk_widget_show_all(GUI(gep->gui)->wmain);
    test_uid(gep);
    /* inicjowanie domyślnego plugina sterownika programatora */
    g_signal_emit_by_name(G_OBJECT(GUI(gep->gui)->prog_combox), "changed");
    // default combox setting
    tmp = NULL;
    if(!store_get(&store, "LAST_CHIP_SELECTED", &tmp)){
	if( tmp ) gui_chip_select(gep, tmp);
    }
    GUI(gep->gui)->gui_run = 1;
    gtk_main(); /* jesli programator ok to startuj program inaczej wyjdź */
    
}

void gui_kill_me(geepro *gep)
{
    printf(TXT_EXIT);
    /* Usuniecie biezacego GUI zbudowanego o xml */
    gui_xml_destroy(GUI(gep->gui)->xml);
    free(GUI(gep->gui)->xml);
    gui_chip_free(gep);
    gtk_main_quit();
    if(gep->chp)
	buffer_free(gep->chp);
}

/**************************************************************************************************************************/

static char gui_progress_bar_exit = 0;

void gui_progress_break(geepro *gep)
{
    gui_progress_bar_exit = 1;
}

void gui_progress_bar_init(geepro *gep, const char *title, long range)
{
    GUI(gep->gui)->progress_win = gtk_dialog_new();
    gtk_window_set_resizable(GTK_WINDOW(GUI(gep->gui)->progress_win), FALSE);
    gtk_window_set_modal(GTK_WINDOW(GUI(gep->gui)->progress_win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(GUI(gep->gui)->progress_win), GTK_WINDOW(GUI(gep->gui)->wmain));
    g_signal_connect(G_OBJECT(GUI(gep->gui)->progress_win), "destroy", G_CALLBACK(gui_progress_break), NULL);
    gtk_window_set_title(GTK_WINDOW(GUI(gep->gui)->progress_win), title);

    GUI(gep->gui)->progress_bar = gtk_progress_bar_new();
    gtk_container_add( GTK_CONTAINER(gtk_dialog_get_action_area(GTK_DIALOG(GUI(gep->gui)->progress_win))), GUI(gep->gui)->progress_bar );
    gui_rfsh_gtk();
    gui_progress_bar_exit = 0;
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(GUI(gep->gui)->progress_bar), TRUE);
    gtk_widget_show_all(GUI(gep->gui)->progress_win);
}

char gui_progress_bar_set(geepro *gep, long value, long max)
{
    long delta;
    
    if(max == 0) return 0;
    if( gui_progress_bar_exit ) return 1;
    if( value != 1){
	if((value != 0) || (value != max)){
	    delta = max / 100;
	    if(delta < 1) delta = 1;
    	    if( value % delta ) return 0;
	}
    }
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(GUI(gep->gui)->progress_bar), (gdouble)value / max);
    gui_rfsh_gtk();
    return 0;
}

void gui_progress_bar_free(geepro *gep)
{
    if(gui_progress_bar_exit) return;
    gtk_widget_destroy(GUI(gep->gui)->progress_win);
}

char gui_cmp_pls(geepro *gep, int a, int b)
{
    char test = (a < b) & !gui_progress_bar_exit;
    if(!test ) gui_progress_bar_free(gep);
    return test;
}

/**************************************************************************************************************************/

static int gui_dialog_exit = 0;

static void gui_dialog_box_close(GtkWidget *wg, char *p_i)
{
    gui_dialog_exit = 1; /* wyjście z petli */

    if(GTK_IS_WINDOW(wg) == TRUE){
	 gtk_widget_destroy(wg);
	 return;
    }
    
    gui_dialog_exit = *p_i;
//    gtk_widget_destroy(wg->parent->parent->parent);
    gtk_widget_destroy(
	gtk_widget_get_parent(
	    gtk_widget_get_parent(
		gtk_widget_get_parent(wg)
	    )
	)	
    );
}

/* do poprawy na gtk_dialog */
int gui_dialog_box(geepro *gp, const char *en, ...)
{
    GtkWidget *wg0, *wg1, *wgtab, *wdialog;
    char *image = NULL;
    char *markup, *title;
    char *fmt, *ft, *ex;
    char flag;
    char pbuttons[256];
    int button;
    va_list ap;

    gui_dialog_exit = 0;
    
    title = NULL;
    if(!strncmp(en, "[CR]", 4)){ image = GTK_STOCK_STOP; title = "Critical error !!!!!!"; }
    if(!strncmp(en, "[ER]", 4)){ image = GTK_STOCK_DIALOG_ERROR; title = "Error !!!"; }
    if(!strncmp(en, "[WN]", 4)){ image = GTK_STOCK_DIALOG_WARNING; title = "Warning !!!"; }
    if(!strncmp(en, "[IF]", 4)){ image = GTK_STOCK_DIALOG_INFO; title = "Information"; }
    if(!strncmp(en, "[HL]", 4)){ image = GTK_STOCK_HELP; title = "Help"; }
    if(!strncmp(en, "[QS]", 4)){ image = GTK_STOCK_DIALOG_QUESTION; title = "Question"; }
    if(!strncmp(en, "[AU]", 4)){ image = GTK_STOCK_DIALOG_AUTHENTICATION; title = "Authentication"; }
    if(!title){
	printf("{gui.c} gui_dialog_box() ---> missing message class token.\n");
	return -1;
    };

    if(!(fmt = malloc(strlen(en) + 1))){
	printf("{gui.c} gui_dialog_box() ---> memory allocation error.\n");
	return -1;
    };
    strcpy(fmt ,en);

    if(!(ft = strstr(fmt, "[TEXT]"))){
	printf("{gui.c} gui_dialog_box() ---> missing [TEXT] delimeter in format string.\n");
	free(fmt);
	return -1;
    }
    ft = strchr(ft, ']') + 1;

    if(!(ex = strstr(fmt, "[/TEXT]"))){
	printf("{gui.c} gui_dialog_box() ---> missing [/TEXT] delimeter in format string.\n");
	free(fmt);
	return -1;
    }
    *ex = '\0';

    /* utworzenie okna w pelni modalnego */
    wdialog = gtk_window_new(GTK_WINDOW_DIALOG);
    gtk_window_set_position(GTK_WINDOW(wdialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable(GTK_WINDOW(wdialog), FALSE);
    g_signal_connect(G_OBJECT(wdialog),"delete_event",G_CALLBACK(gui_dialog_box_close), NULL);
    gtk_window_set_title(GTK_WINDOW(wdialog), title);
    gtk_container_set_border_width(GTK_CONTAINER(wdialog), 10);
    gtk_window_set_modal(GTK_WINDOW(wdialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(wdialog), GTK_WINDOW(GUI(gp->gui)->wmain));

    /* tabela pakujaca */
    wgtab = gtk_table_new(2, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(wdialog), wgtab);    

    if(image){
	wg0 = gtk_image_new_from_stock( image, GTK_ICON_SIZE_DIALOG);
	gtk_table_attach(GTK_TABLE(wgtab), wg0, 0,1,0,1 ,GTK_FILL, GTK_FILL|GTK_EXPAND, 0,0);
    }

    /* dodanie tekstu */
    wg0 = gtk_label_new(NULL);
    va_start(ap, en);
    markup = g_markup_vprintf_escaped(ft, ap);
    va_end(ap);
    gtk_label_set_markup(GTK_LABEL(wg0), markup);
    g_free(markup);
    gtk_table_attach(GTK_TABLE(wgtab), wg0, 1,2,0,1 ,GTK_FILL, GTK_FILL|GTK_EXPAND, 0,0);

    /* przyciski */
    ft = strchr(ex + 1, ']') + 1; /* koniec klamerki */

    wg0 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(wg0), 3);
    gtk_table_attach(GTK_TABLE(wgtab), wg0, 0,2,1,2 ,GTK_FILL, GTK_FILL, 0,0);

    button = 0;
    do{
	flag = 0;
	if(!strncmp(ft, "[BR]", 4)) flag = 1;
	if(!strncmp(ft, "[BL]", 4)) flag = 2;
	if(!flag) break;
	ft = strchr(ft, ']') + 1;
	ex = strchr(ft, '[');
	if(ex) *ex = '\0';    

	wg1 = gtk_button_new_with_label(ft);
	
	if(flag == 1) gtk_box_pack_end(GTK_BOX(wg0), wg1, FALSE, FALSE, 0);
	if(flag == 2) gtk_box_pack_start(GTK_BOX(wg0), wg1, FALSE, FALSE, 0);
	pbuttons[ button ] = button + 2;
        g_signal_connect(G_OBJECT(wg1),"clicked",G_CALLBACK(gui_dialog_box_close), (void*)(pbuttons + button) );
	button++;
	if(ex) *ex = '[';
	ft = ex;
    } while(ex  && (button < 256) );
    
    free(fmt);
    gtk_widget_show_all(wdialog);

    for(;!gui_dialog_exit;) gtk_main_iteration();

    return gui_dialog_exit - 1;
}

/**************************************************************************************************************************/
/* generator fali prostokatnej */

static void gui_test_set_period(GtkWidget *wg,  sqw_gen *sqg)
{
    sqg->period = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sqg->wper));
}

static void gui_test_set_duty(GtkWidget *wg,  sqw_gen *sqg)
{
    sqg->duty = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sqg->wdut));
}

static void gui_test_set_length(GtkWidget *wg,  sqw_gen *sqg)
{
    sqg->len = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sqg->wlen));
}

static void gui_test_set_sequence(GtkWidget *wg,  sqw_gen *sqg)
{
    sqg->seq = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sqg->wseq));
}

void gui_clk_sqw(gui *g, gui_sqw_generator gen)
{
    GtkWidget *wgm, *wg0, *wg1;
    GtkAdjustment *adj;    
    static sqw_gen sqg;

    sqg.generator = gen;
    sqg.parent = g;
    
    wgm = gtk_dialog_new_with_buttons("Square Wave Generator", 	GTK_WINDOW(g->wmain), GTK_DIALOG_MODAL, 
	GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
	GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, 
	NULL);
    
    gtk_container_set_border_width(GTK_CONTAINER(wgm), 10);

    wg0 = gtk_table_new(2, 5, FALSE);    
    gtk_container_add(GTK_CONTAINER(  gtk_dialog_get_content_area(GTK_DIALOG(wgm))  ), wg0);

    wg1 = gtk_label_new("Period [us]:");    
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach(GTK_TABLE(wg0), wg1, 0,1, 0,1,  GTK_FILL,0,  0,0);
    wg1 = gtk_label_new("Duty cycle [%]:");    
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach(GTK_TABLE(wg0), wg1, 0,1, 1,2,  GTK_FILL,0,  0,0);
    wg1 = gtk_label_new("Length [s]:");    
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach(GTK_TABLE(wg0), wg1, 0,1, 2,3,  GTK_FILL,0,  0,0);
    wg1 = gtk_label_new("Sequence (32bit):");    
    gtk_misc_set_alignment(GTK_MISC(wg1), 0, 0);
    gtk_table_attach(GTK_TABLE(wg0), wg1, 0,1, 3,4,  GTK_FILL,0,  0,0);

    adj = GTK_ADJUSTMENT(gtk_adjustment_new(100, 0.0, 1000000, 100, 0, 0));
    wg1 = gtk_spin_button_new(adj, 1, 0);
    sqg.wper = wg1;
    gtk_table_attach(GTK_TABLE(wg0), wg1, 1,2, 0,1,  GTK_FILL | GTK_EXPAND,0,  0,0);
    g_signal_connect(G_OBJECT(adj),"value_changed",G_CALLBACK(gui_test_set_period), &sqg);

    adj = GTK_ADJUSTMENT(gtk_adjustment_new(50, 0.0, 100, 1, 0, 0));
    wg1 = gtk_spin_button_new(adj, 1, 0);
    sqg.wdut = wg1;
    gtk_table_attach(GTK_TABLE(wg0), wg1, 1,2, 1,2,  GTK_FILL | GTK_EXPAND,0,  0,0);
    g_signal_connect(G_OBJECT(adj),"value_changed",G_CALLBACK(gui_test_set_duty), &sqg);

    adj = GTK_ADJUSTMENT(gtk_adjustment_new(1, 0.0, 60, 1, 0, 0));
    wg1 = gtk_spin_button_new(adj, 1, 0);
    sqg.wlen = wg1;
    gtk_table_attach(GTK_TABLE(wg0), wg1, 1,2, 2,3,  GTK_FILL | GTK_EXPAND,0,  0,0);
    g_signal_connect(G_OBJECT(adj),"value_changed",G_CALLBACK(gui_test_set_length), &sqg);

    adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0.0, 0xffffffff, 1, 0, 0));
    wg1 = gtk_spin_button_new(adj, 1, 0);
    sqg.wseq = wg1;
    gtk_table_attach(GTK_TABLE(wg0), wg1, 1,2, 3,4,  GTK_FILL | GTK_EXPAND,0,  0,0);
    g_signal_connect(G_OBJECT(adj),"value_changed",G_CALLBACK(gui_test_set_sequence), &sqg);

    gtk_widget_show_all(wgm);

    while(gtk_dialog_run(GTK_DIALOG(wgm)) == GTK_RESPONSE_ACCEPT){
	gtk_widget_hide(wgm);
	sqg.generator(&sqg);
	gtk_widget_show_all(wgm);
    }
    gtk_widget_destroy(wgm);
}

char *gui_lookup_tag(const char *fmt, char *tmp, int size, const char *key_begin, const char *key_end)
{
    char *str, *x;
    tmp[0] = 0;
    if((str = strstr(fmt, key_begin))){
	str += strlen(key_begin);
	if(!(x = strstr(str, key_end))){
	    printf("[WARN] gui_checkbox(): missing %s in sequence %s ... %s\n", key_end, key_begin, key_end);
	    return NULL;
	};
	memset(tmp, 0, size);
	strncpy(tmp, str, x - str);
	return x + 1;
    }
    return NULL;
}

// global
static unsigned long gui_checkbox_result;

void gui_checkbox_action(GtkWidget *wg, int *value)
{
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wg)) )
	gui_checkbox_result |= *value;
    else
	gui_checkbox_result &= ~(*value);
}

unsigned long *gui_checkbox(geepro *gep, const char *fmt)
{
    GtkWidget *wd, *hbox, *vbox, *r;

    char *x = NULL, *str, t;
    char *tmp;
    int tmp_size;    
    int result = 0, lock;
    int cnt;

    tmp_size = strlen( fmt );
    if(!(tmp = (char *)malloc(tmp_size + 1))){
	printf("[WARN] gui_checkbox(): malloc error\n");
	return NULL;
    }
    gui_checkbox_result = 0;
    gui_lookup_tag(fmt, tmp, tmp_size, "[TITLE]", "[/TITLE]");
    wd = gtk_dialog_new_with_buttons(tmp, 
	GTK_WINDOW(GUI(gep->gui)->wmain), 
	GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
	GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
	NULL 
    );    
    gui_lookup_tag(fmt, tmp, tmp_size, "[TYPE:", "]");
    if(!strcmp(tmp, "QS")) x = GTK_STOCK_DIALOG_QUESTION;
    if(!strcmp(tmp, "WN")) x = GTK_STOCK_DIALOG_WARNING;
    if(!strcmp(tmp, "ER")) x = GTK_STOCK_DIALOG_ERROR;
    if(!strcmp(tmp, "HL")) x = GTK_STOCK_HELP;
    if(!strcmp(tmp, "CR")) x = GTK_STOCK_STOP;
    if(!strcmp(tmp, "IF")) x = GTK_STOCK_DIALOG_INFO;
    if(!strcmp(tmp, "AU")) x = GTK_STOCK_DIALOG_AUTHENTICATION;
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(   gtk_dialog_get_content_area(GTK_DIALOG(wd))  ), hbox, TRUE, TRUE, 0);
    if( x ) 
	gtk_box_pack_start(GTK_BOX(hbox), gtk_image_new_from_stock( x, GTK_ICON_SIZE_DIALOG), FALSE, FALSE, 10 );
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_end(GTK_BOX(hbox), vbox, FALSE, 0, 0);    
    str = (char *)fmt; 
    cnt = 0;
    while( (str = gui_lookup_tag(str, tmp, tmp_size, "[CB:", "]")) && (cnt < MAX_CB_TABLE)){
	sscanf(tmp, " %i ", &result);
	if((x = strchr(tmp, ':'))) x++;
	t = *x;
	if((x = strchr(x, ':'))) x++;
	if((lock = strncmp(x, "LOCK:", 5)) == 0){
	    if((x = strchr(x, ':'))) x++;
	}
	r = gtk_check_button_new_with_label( x );
	gtk_box_pack_start( GTK_BOX(vbox), r, TRUE, TRUE, 2 );
	GUI(gep->gui)->cbtable[ cnt ] = result;
	g_signal_connect(G_OBJECT(r), "toggled", G_CALLBACK(gui_checkbox_action), (void *)(GUI(gep->gui)->cbtable + cnt)); // ptr as integer
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r), t == '1');
	gtk_widget_set_sensitive(GTK_WIDGET(r), !((lock == 0) && (t == '1')));
	cnt++;
    }
    
    gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(wd)));
    result = gtk_dialog_run(GTK_DIALOG(wd));    
    gtk_widget_destroy(GTK_WIDGET(wd));
    free(tmp);
    if(result != GTK_RESPONSE_ACCEPT) return NULL;
    return &gui_checkbox_result;
}



