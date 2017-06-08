/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2011 Krzysztof Komarnicki
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

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "../src/cfp.h"
#include "be_stencil.h"
#include "index_stc.h"

typedef struct
{
    GtkWidget *combo, *vbox, *table, 
	      *min, *max, *deflt, *box, 
	      *editable, *clear, 
	      *length, *names, 
	      *values, *offset, *range,
	      *seconds, *format,
	      *script, *description,*settings;
    gint sel;
    s_cfp *cfp;
    char  changed;
} gui_stencil_edit_str;

enum{
    GCBT_INTEGER,
    GCBT_STRING,
    GCBT_CHECK,
    GCBT_SELECTION,
    GCBT_DATE,
    GCBT_TIME,
};

static void gui_bineditor_stencil_edit_add_entries( GtkWidget *list, gui_stencil_edit_str *gses );
static char gui_bineditor_stencil_new(GuiBineditor *be);
static char gui_bineditor_stencil_edit(GuiBineditor *be, const char *fname);
static void gui_bineditor_stencil_edit_desc_entry_ev(GtkTextBuffer *txt, gui_stencil_edit_str *gses);
static void gui_bineditor_stencil_edit_fname_entry_ev();
static void gui_bineditor_stencil_edit_menu_entry_ev();
static void gui_bineditor_stencil_edit_device_entry_ev();
static void gui_bineditor_stencil_edit_new_button_ev(GtkButton *bt, gui_stencil_edit_str *gses);
static void gui_bineditor_stencil_edit_del_button_ev(GtkButton *bt, gui_stencil_edit_str *gses);

/*************************************************************************************************************************************/

static char *gui_be_get_path(const char *fpath)
{
    char *tmp;
    int i;
    
    i =  strlen(fpath);
    if(!(tmp = (char *)malloc( i + 1 ))) return NULL;
    strcpy(tmp, fpath);
    i--;
    for( ; fpath[i] && fpath[i] != '/' && i != 0; i--);    
    if( i == 0 ) return NULL;
    tmp[i] = 0;    
    return tmp;
}

static inline char gui_be_stencil_check_ext(const char *name)
{
    char *x;

    if(!(x = strchr((char *)name, '.'))) return 0;
    return !strcmp(x, GUI_BINEDITOR_STENCIL_FILE_EXTENSION);
}

static inline void gui_be_stencil_add_position(GuiBineditor *be, const char *path, const char *f_name, FILE *f)
{    
    char *tree, *name, *desc, *fname;
    s_cfp *cfp;
    
    if(!(fname = (char *) malloc( strlen(path) + strlen(f_name) + 2))){
	ERROR(E_ERR, E_T_MALLOC);
	return;
    }
    sprintf(fname, "%s/%s", path, f_name);
    cfp = cfp_init();
    if(!cfp_load( cfp, fname)) return ;

    tree = cfp_get_val_stringp( cfp, "/device/tree");
    desc = cfp_get_val_stringp( cfp, "/device/description");
    name = cfp_get_val_stringp( cfp, "/device/name");    
    cfp_free( cfp );
    if( !tree ){
	ERROR( E_WRN, "syntax error in file %s: missing key /device/tree ", fname);
	free( fname );
	return;
    }    

    if( !desc ){
	ERROR( E_WRN, "syntax error in file %s: missing key /device/description ", fname);
	free( tree );
	free( fname );
	return;
    }    
    if( !name ){
	ERROR( E_WRN, "syntax error in file %s: missing key /device/name ", fname);
	free( tree );
	free( desc );
	free( fname );
	return;
    }    
    fprintf(f, "%s:%s/$%s:\"%s\"\n", fname, tree, name, desc);    
    free( tree );
    free( desc );
    free( name );
    free( fname );
}

char gui_bineditor_stencil_generate_index_file(GuiBineditor *be, const char *fname)
{
    FILE *f;
    DIR  *d;
    struct dirent *dir;
    char *path;
        
    if(!(f = fopen(fname, "w"))){
	printf("ERROR:gui_bineditor_stencil_generate_index_file() -> creating index file error\n");
	return 0;
    }
    fprintf(f, "#Automaticaly generated - don't edit!\n#file path:menu path:\"description\"\n");
    
    if(!(path = gui_be_get_path( fname ))){
	path = (char *)".";
    }
    
    if(!(d = opendir(path))){
	printf("ERROR:gui_bineditor_stencil_generate_index_file() -> open directory %s\n", path);
	return 0;
    }
    while((dir = readdir(d))){
	if( gui_be_stencil_check_ext( dir->d_name) ) 
		gui_be_stencil_add_position(be, path, dir->d_name, f);
    }
    closedir( d );
    fclose(f);    
    free( path );    
    return 1;
}

extern void gui_bineditor_stencil_update(  GuiBineditor *be );	

static inline char gui_bineditor_stencil_update_all( GuiBineditor *be )
{    
    char x = !gui_bineditor_stencil_generate_index_file(be, "./stencils/stencil.idx"); // !!! path from config !!!!!
    if(!x) gui_bineditor_stencil_update( be );	
    return x;
}

static inline void gui_bineditor_stencil_add_file(GuiBineditor *be, char *fname)
{
    GtkWidget *dlg;
    char *tree, *name, *desc, ch;
    FILE *f1, *f2;
    s_cfp *cfp;
    index_stc_str *idx;
    char *bn;
    char path1[PATH_MAX + 1], path2[PATH_MAX + 1];
    if(!realpath("./stencils", path1)){ // from config !!
	ERROR(E_ERR, "real path 1");
	return;    
    }
    if(!realpath(fname, path2)){
	ERROR(E_ERR, "real path 2");
	return;    
    }
    bn = basename( fname );
    if( strcmp( path1, dirname( path2 )) ){ // if different location then copy to ./stencils
	char tmp[ strlen(path1) + strlen(bn) + 1 ];
        sprintf(tmp, "%s/%s", path1, bn);
	if( !access(tmp, R_OK) != 0 ){
	    char yes;
	    dlg = gtk_message_dialog_new( GTK_WINDOW(be->priv->wmain), 
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO, 
		"File %s already exist in %s. Overwrite ?", bn, path1
	    ); 
	    yes = gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_YES;
	    gtk_widget_destroy(dlg);
	    if( !yes ) return;
	    if(unlink( tmp ) != 0){
		ERROR(E_ERR, "Cannot delete file %s", tmp);
		return;
	    }
	}
	if(!(f1 = fopen(fname, "r"))){
	    ERROR(E_ERR, "open source file error %s\n", fname);
	    return;
	}
	if(!(f2 = fopen(tmp, "w"))){
	    ERROR(E_ERR, "creating destination file error '%s'\n", tmp);
	    fclose(f1);
	    return;
	}
	printf("MSG(0) Copy file '%s' to '%s'\n", bn, path1);
	// slow method, but simple. Good for short files.
	while( !feof(f1) ){
	    ch = fgetc(f1);
	    if(!feof(f1)) fputc(ch, f2);
	}
	fclose( f1 );
	fclose( f2 );
    }        
// get path, name and description from stencil file
    cfp = cfp_init();
    if(!cfp_load( cfp, fname)) return ;
    tree = cfp_get_val_stringp( cfp, "/device/tree");
    desc = cfp_get_val_stringp( cfp, "/device/description");
    name = cfp_get_val_stringp( cfp, "/device/name");    
    cfp_free( cfp );
    if( !tree ){
	ERROR( E_WRN, "syntax error in file %s: missing key /device/tree ", fname);
	return;
    }    

    if( !desc ){
	ERROR( E_WRN, "syntax error in file %s: missing key /device/description ", fname);
	free( tree );
	return;
    }    
    if( !name ){
	ERROR( E_WRN, "syntax error in file %s: missing key /device/name ", fname);
	free( tree );
	free( desc );
	return;
    }    
    // check index file 
    if(!(idx = index_stc_open("./stencils/stencil.idx"))) return; // from config !!!
    if(!index_stc_path_lookup( idx, tree, name)){
	index_stc_add( idx, tree, name, desc, fname );    
    } else {
	char yes;
	dlg = gtk_message_dialog_new( GTK_WINDOW(be->priv->wmain), 
	    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
	    GTK_BUTTONS_YES_NO, 
	    "Path '%s/%s' already indexed. Reindex ?", tree, name
	); 
	yes = gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_YES;
	gtk_widget_destroy(dlg);
	if(yes){
	    if(index_stc_del(idx, tree, name)){
	       index_stc_add( idx, tree, name, desc, fname );
	    } else {
		ERROR(E_ERR, "delete path '%s/%s'", tree, name);
	    }
	}
    }
    index_stc_save( idx );
    index_stc_close( idx );
    free( tree );
    free( desc );
    free( name );
    gui_bineditor_stencil_update( be );	
}

static inline char gui_bineditor_stencil_add(GuiBineditor *be)
{
    GtkWidget *dlg;
    GtkFileFilter *ff;
        
    dlg = gtk_file_chooser_dialog_new("Geepro - add stencil", GTK_WINDOW(be->priv->wmain), GTK_FILE_CHOOSER_ACTION_OPEN, 
	    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	    NULL
	  );
    // filter
    ff = gtk_file_filter_new();
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), ff);
    gtk_file_filter_set_name(ff, "*.stc");
    gtk_file_filter_add_pattern(ff, "*.stc");    
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dlg), ff);

    if( gtk_dialog_run( GTK_DIALOG(dlg) ) ){
	char *fname;
	fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
	gui_bineditor_stencil_add_file(be, fname);
	g_free( fname );
    }
    gtk_widget_destroy( dlg );
    return 0;
}

static inline char gui_bineditor_stencil_remove(GuiBineditor *be, const char *device, const char *path)
{
    GtkWidget *dlg;
    index_stc_str *idx;

    if( !be || !device || !path ) return 0;
    if(!(idx = index_stc_open("./stencils/stencil.idx"))) return 0; // from config !!!
    if(index_stc_path_lookup( idx, path, device)){
	if(index_stc_del(idx, path, device)){
	    index_stc_save( idx );
	    if( idx->fnm ){
		dlg = gtk_message_dialog_new( GTK_WINDOW(be->priv->wmain), 
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO, 
			"Device removed from list.\nDelete also file '%s' from stencils directory ?", idx->fnm
		  ); // intl
		if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_YES){
		    if(unlink( idx->fnm ) != 0){
			ERROR(E_ERR, "Cannot delete file %s", idx->fnm);
		    }
		}
		gtk_widget_destroy(dlg);
	    }
	    index_stc_close( idx );    
	    gui_bineditor_stencil_update( be );	
	    return 0;
	}
    }
    index_stc_close( idx );    
    return 0;
}

char gui_bineditor_stencil_operation(GuiBineditor *be, const char *fname, const char *device, char *path, int operation)
{
    switch( operation ){
	case GUI_BE_OPERATION_UPDATE_ALL : return gui_bineditor_stencil_update_all( be );
	case GUI_BE_OPERATION_ADD : return gui_bineditor_stencil_add( be );
	case GUI_BE_OPERATION_REMOVE : return gui_bineditor_stencil_remove( be, device, path );
	case GUI_BE_OPERATION_NEW: return 0; //return gui_bineditor_stencil_new( be );
	case GUI_BE_OPERATION_EDIT: return gui_bineditor_stencil_edit( be, fname);
    }
    return 0;
}

static char gui_bineditor_stencil_new(GuiBineditor *be)
{
    GtkWidget *dlg, *ctx, *tb;
    GtkWidget *w_dev;
    GtkWidget *w_pth;
    GtkWidget *w_fnm;
    const char *dev, *pth, *fnm;
    char acc;
    
    dlg = gtk_dialog_new_with_buttons("Geepro - new stencil", GTK_WINDOW(be->priv->wmain),
	GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
	GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
	NULL
    );
    ctx = gtk_dialog_get_content_area( GTK_DIALOG(dlg) );    
    tb = gtk_table_new(2, 3, FALSE);
    gtk_container_add( GTK_CONTAINER(ctx), tb );
    gtk_table_attach( GTK_TABLE(tb), gtk_label_new("Device name:"), 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach( GTK_TABLE(tb), gtk_label_new("Menu path  :"), 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach( GTK_TABLE(tb), gtk_label_new("File name  :"), 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);    
    w_dev = gtk_entry_new();
    w_pth = gtk_entry_new();
    w_fnm = gtk_entry_new();
    gtk_table_attach( GTK_TABLE(tb), w_dev, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach( GTK_TABLE(tb), w_pth, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach( GTK_TABLE(tb), w_fnm, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show_all( ctx );    
    do{
	acc = gtk_dialog_run( GTK_DIALOG(dlg) ) == GTK_RESPONSE_ACCEPT;
	dev = gtk_entry_get_text( GTK_ENTRY( w_dev ) );
	pth = gtk_entry_get_text( GTK_ENTRY( w_pth ) );
	fnm = gtk_entry_get_text( GTK_ENTRY( w_fnm ) );
    } while( acc && (!*dev || !*pth || !*fnm));
    gtk_widget_destroy( dlg );
    if( !acc ) return 0;
    gui_bineditor_stencil_edit( be, fnm );
    return 0;
}

static GtkWidget *gui_bineditor_stencil_edit_list()
{
    GtkWidget *list, *wg;
    GtkTreeViewColumn *col;    
    list = gtk_tree_view_new();
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);    
    col = gtk_tree_view_column_new_with_attributes("Entries", gtk_cell_renderer_text_new(), "text", 0, NULL);
    gtk_tree_view_append_column( GTK_TREE_VIEW(list), col );
    wg = (GtkWidget *)gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model( GTK_TREE_VIEW(list), GTK_TREE_MODEL(wg));
    g_object_unref(wg);
    return list;
}

static void gui_bineditor_stencil_edit_add_entry(GtkWidget *list, const char *str, const char *key)
{
    GtkTreeIter iter;
    GtkListStore *store;
    
    store = GTK_LIST_STORE( gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, str, 1, key, -1);    
}

/*****************************************************************************************************************************************************/


static void gui_bineditor_stencil_edit_combo(GtkComboBox *cbo, gui_stencil_edit_str *gses)
{
    int x = gtk_combo_box_get_active( cbo );
    
    if( gses->sel == x ) return;
    gses->sel = x;
    // delete current widgets
    if( gses->table ) gtk_widget_destroy( gses->table );
    gses->table = NULL;
    gses->table = gtk_table_new( 5, 2, FALSE);
    gtk_box_pack_start(GTK_BOX( gses->vbox ), gses->table, TRUE, TRUE, 3);
    switch( x ){
	case GCBT_INTEGER: 
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Editable:"), 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Default value:"), 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Minimum value:"), 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Maximum value:"), 0, 1, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->editable = gtk_check_button_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->editable, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->deflt = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->deflt, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->min = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->min, 1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->max = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->max, 1, 2, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    break;
	case GCBT_STRING:
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Editable:"), 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Clear:"), 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Default:"), 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Total length:"), 0, 1, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->editable = gtk_check_button_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->editable, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->clear = gtk_check_button_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->clear, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->deflt = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->deflt, 1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->length = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->length, 1, 2, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    break;
	case GCBT_CHECK:
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Editable:"), 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Names:"), 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Default:"), 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->editable = gtk_check_button_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->editable, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->names = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->names, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->deflt = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->deflt, 1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    break;
	case GCBT_SELECTION:
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Default:"), 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Names:"), 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Values:"), 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->deflt = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->deflt, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->names = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->names, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->values = gtk_entry_new();
    	    gtk_table_attach(GTK_TABLE( gses->table ), gses->values, 1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    break;
	case GCBT_DATE:
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Year offset:"), 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Year range:"), 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Default (dd.mm.yy):"), 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->offset = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->offset, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->range = gtk_entry_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->range, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->deflt = gtk_entry_new();
    	    gtk_table_attach(GTK_TABLE( gses->table ), gses->deflt, 1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    break;
	case GCBT_TIME:
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("Seconds"), 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gtk_table_attach(GTK_TABLE( gses->table ), gtk_label_new("12h format:"), 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->seconds = gtk_check_button_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->editable, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    gses->format = gtk_check_button_new();
	    gtk_table_attach(GTK_TABLE( gses->table ), gses->clear, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	    break;
    }
//    gui_bineditor_stencil_edit_fill_entries();
    gtk_widget_show_all( gses->vbox );
}

typedef struct 
{
    const char *name;
    char type;
    char *deflt;
    int  minimum;
    int  maximum;
    int  length;
    char *names;
    char *values;
    int  offset;
    int  range;
    char editable:1;
    char clear:1;
    char seconds:1;
    char mode12h:1;
} entry_str;

void gui_bineditor_stencil_edit_get_entry(s_cfp *cfp, entry_str *en, const char *name)
{
    char tmp[ strlen(name) + 16];
    s_cfpq *p = NULL;
    // get entry content
    sprintf(tmp, "/entries/%s", name);
    if( cfp && name)
	p = cfp_get( cfp, tmp);
    p = cfp_block( p );
    // get all possible entry values
    en->name = name;
return;
//    en->type = cfp_get_val_select(p,  "type", "integer,string,check,selection,date,time");
    en->deflt = cfp_get_val_string(p, "default");
    en->minimum = cfp_get_val_int(p,  "minimum");
    en->maximum = cfp_get_val_int(p,  "maximum");
    en->length = cfp_get_val_int(p,   "length");
    en->names = cfp_get_val_string(p, "names");
    en->values = cfp_get_val_string(p,"values");
    en->offset = cfp_get_val_int(p,   "offset");
    en->range = cfp_get_val_int(p,    "range");
    en->editable = cfp_get_val_bool(p,"editable");
    en->clear = cfp_get_val_bool(p,   "clear");
    en->seconds = cfp_get_val_bool(p, "seconds");
    en->mode12h =cfp_get_val_bool(p,  "mode12h");
printf(" --> %s\n", en->deflt);
}

static void gui_bineditor_stencil_edit_select(GtkTreeSelection *tree, gui_stencil_edit_str *gses)
{
    GtkWidget *wg, *wgx;
    char *tmp = NULL, *value = NULL;
    GtkTreeIter iter;
    GtkTreeModel *model;
    entry_str en;

    // destroy previous selection
    if( gses->box != NULL ) gtk_widget_destroy( gses->box );
    // create menu path
    if(gtk_tree_selection_get_selected( tree, &model, &iter))
		    gtk_tree_model_get( model, &iter, 1, &value, -1);

    gui_bineditor_stencil_edit_get_entry( gses->cfp, &en, value);
    if( value )g_free( value );
    
    gses->box = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add( GTK_CONTAINER(gses->settings), gses->box);    
    
    // entry settings
    wg = gtk_frame_new( "Entry type" );
    gtk_box_pack_start(GTK_BOX(gses->box), wg, FALSE, FALSE, 5);
    gtk_frame_set_label_align(GTK_FRAME(wg), 0.5, 0.5);
    gses->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER( wg ), gses->vbox);
    gses->sel = -1;
    gses->table = NULL;
    wg = gtk_combo_box_text_new();
    gses->combo = wg;
    gtk_box_pack_start(GTK_BOX(gses->vbox), wg, TRUE, FALSE, 5);
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(wg), "integer");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(wg), "string");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(wg), "check");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(wg), "selection");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(wg), "date");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(wg), "time");
    g_signal_connect(G_OBJECT(wg), "changed", G_CALLBACK(gui_bineditor_stencil_edit_combo), gses);
//    gtk_combo_box_set_active( GTK_COMBO_BOX(wg), 0); // ust !!

    // script
    wg = gtk_frame_new( "Execute script" );
    gtk_frame_set_label_align(GTK_FRAME(wg), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(gses->box), wg, FALSE, FALSE, 5);
    wgx = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER( wg ), wgx);
    gses->script = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER( wgx ), gses->script);

    // description
    wg = gtk_frame_new( "Description" );
    gtk_frame_set_label_align(GTK_FRAME(wg), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(gses->box), wg, FALSE, FALSE, 5);
    wgx = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER( wg ), wgx);
    gses->description = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER( wgx ), gses->description);
//    gui_bineditor_stencil_edit_fill_settings();
    gtk_widget_show_all( gses->settings );    
    if( tmp ) free(tmp);
}

static void gui_bineditor_stencil_edit_add_entries( GtkWidget *list, gui_stencil_edit_str *gses )
{
    char key[64], rr, *tmp;
    int i;
    const s_cfpq *entries, *cr, *xx;

    entries = cfp_get( gses->cfp, "/entries");
    if( !entries ) return;    
    xx = cfp_block( entries );
    for( i = 0, rr = 1; rr; i++){
	sprintf(key, "entry[%i]", i);
	entries = xx; 
	rr = 0;   
	for( ;entries; entries = cfp_next( entries )){
	    if( strcmp(cfp_get_key(entries), key) ) continue;	
	    if( !(cr = cfp_block( entries )) ){
		ERROR(E_WRN,"syntax error");
		continue;
	    }
	    tmp = cfp_get_val_string( (s_cfpq *)cr, "name" );
	    if( !tmp ) continue;
	    gui_bineditor_stencil_edit_add_entry( list, tmp, key);
	    free( tmp );
	    rr = 1;
	}
    }    
}

static inline void gui_bineditor_stencil_edit_build(GuiBineditor *be, GtkWidget *ctx, gui_stencil_edit_str *gses, const char *fname )
{
    GtkWidget *hbox, *wg, *list, *wgx, *tb, *tx;
    char *tmp;

    gtk_container_set_border_width( GTK_CONTAINER( ctx ), 5);

    // ************* DEVICE MENU *****************
    wg = gtk_frame_new("Device");    
    gtk_box_pack_start(GTK_BOX(ctx), wg, TRUE, TRUE, 10);
    wgx = gtk_table_new(4, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(wg), wgx); 
    gtk_table_attach(GTK_TABLE( wgx ), gtk_label_new("Device name:"), 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE( wgx ), gtk_label_new("Menu path:"), 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE( wgx ), gtk_label_new("File name:"), 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);        
    wg = gtk_entry_new();
    gtk_table_attach(GTK_TABLE( wgx ), wg, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    if((tmp = cfp_get_val_stringp(gses->cfp, "/device/name"))){
	gtk_entry_set_text(GTK_ENTRY(wg), tmp);
	free( tmp );
    }
    g_signal_connect(G_OBJECT(wg), "changed", G_CALLBACK(gui_bineditor_stencil_edit_device_entry_ev), gses);
    wg = gtk_entry_new();
    gtk_table_attach(GTK_TABLE( wgx ), wg, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    if((tmp = cfp_get_val_stringp(gses->cfp, "/device/tree"))){
	gtk_entry_set_text(GTK_ENTRY(wg), tmp);
	free( tmp );
    }
    g_signal_connect(G_OBJECT(wg), "changed", G_CALLBACK(gui_bineditor_stencil_edit_menu_entry_ev), gses);
    wg = gtk_entry_new();
    gtk_table_attach(GTK_TABLE( wgx ), wg, 1, 2, 2, 3, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_entry_set_text(GTK_ENTRY(wg), fname);
    g_signal_connect(G_OBJECT(wg), "changed", G_CALLBACK(gui_bineditor_stencil_edit_fname_entry_ev), gses);
    wg = gtk_frame_new("Description");
    gtk_frame_set_label_align( GTK_FRAME(wg), 0.5, 0.5);
    gtk_table_attach(GTK_TABLE( wgx ), wg, 0, 2, 3, 4, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 5, 5);    
    wgx = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER( wg ), wgx);
    wg = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER( wgx ), wg);
    tx = (GtkWidget *)gtk_text_view_get_buffer(GTK_TEXT_VIEW( wg ));
    if((tmp = cfp_get_val_stringp(gses->cfp, "/device/description"))){
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tx), tmp , -1);
	free( tmp );
    }
    g_signal_connect(G_OBJECT(tx), "changed", G_CALLBACK(gui_bineditor_stencil_edit_desc_entry_ev), gses);

    // ************ SETTINGS **************
    hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start( GTK_BOX( ctx ), hbox, TRUE, TRUE, 0);
    // list
    wgx = gtk_frame_new(NULL);
    gtk_box_pack_start( GTK_BOX( hbox ), wgx, TRUE, TRUE, 0);
    // preferences
    gses->box = NULL;
    gses->settings = gtk_frame_new( "Settings" );    
    gtk_box_pack_start( GTK_BOX( hbox ), gses->settings, TRUE, TRUE, 10);    

    // *************** LIST ******************
    tb = gtk_table_new( 2, 2, FALSE );
    gtk_container_add(GTK_CONTAINER(wgx), tb);
    wg = gtk_scrolled_window_new(NULL, NULL);
    gtk_table_attach(GTK_TABLE( tb ), wg, 0, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
    list = gui_bineditor_stencil_edit_list();
    gtk_container_add(GTK_CONTAINER(wg), list);
    gui_bineditor_stencil_edit_add_entries( list, gses );
    g_signal_connect(G_OBJECT( gtk_tree_view_get_selection(GTK_TREE_VIEW(list)) ), "changed", G_CALLBACK(gui_bineditor_stencil_edit_select), gses);    
    wg = gtk_button_new_with_label("New");
    gtk_table_attach(GTK_TABLE( tb ), wg, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);        
    g_signal_connect(G_OBJECT(wg), "pressed", G_CALLBACK(gui_bineditor_stencil_edit_new_button_ev), gses);
    wg = gtk_button_new_with_label("Delete");
    gtk_table_attach(GTK_TABLE( tb ), wg, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);        
    g_signal_connect(G_OBJECT(wg), "pressed", G_CALLBACK(gui_bineditor_stencil_edit_del_button_ev), gses);
}

static char gui_bineditor_stencil_edit(GuiBineditor *be, const char *fname)
{
    GtkWidget *dlg, *ctx;
    gui_stencil_edit_str gses;
    s_cfp *p;
    char acc;

    gses.changed = 0;
    if( !( p = cfp_init())) return 0;
    if( fname ) cfp_load( p, fname);
    gses.cfp = p;
    
    dlg = gtk_dialog_new_with_buttons("Geepro - edit stencil", GTK_WINDOW(be->priv->wmain),
	GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
	GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
	NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(dlg), 400, 300);
    ctx = gtk_dialog_get_content_area( GTK_DIALOG(dlg) );        
    gui_bineditor_stencil_edit_build( be, ctx, &gses, fname );
    gtk_widget_show_all( ctx );
    acc = gtk_dialog_run( GTK_DIALOG(dlg) ) == GTK_RESPONSE_ACCEPT;
    gtk_widget_destroy( dlg );

    if( fname && acc && gses.changed) 
printf("ulele !\n");
//	    cfp_save(p, fname);

    cfp_free( p );    
    return 0;
}


/*****************************************************************************************************************************************/


void gui_bineditor_stencil_sheet(GuiBineditor *be, const char *device, const char *fname){ printf("stencil: %s - %s\n", fname, device); }
static void gui_bineditor_stencil_edit_desc_entry_ev(GtkTextBuffer *txt, gui_stencil_edit_str *gses){ printf("text view\n"); }
static void gui_bineditor_stencil_edit_fname_entry_ev(){ printf("fname view\n"); }
static void gui_bineditor_stencil_edit_menu_entry_ev(){ printf("menu view\n"); }
static void gui_bineditor_stencil_edit_device_entry_ev(){ printf("device view\n"); }
static void gui_bineditor_stencil_edit_new_button_ev(GtkButton *bt, gui_stencil_edit_str *gses){ printf("button new\n");}
static void gui_bineditor_stencil_edit_del_button_ev(GtkButton *bt, gui_stencil_edit_str *gses){ printf("button delete\n");}


