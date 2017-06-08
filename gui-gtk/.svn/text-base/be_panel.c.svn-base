/* $Revision: 1.8 $ */
/* binary editor
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
#include <stdlib.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include "bineditor.h"
#include "../src/script.h"
#include "../intl/lang.h"
#include "be_buffer.h"
#include "be_panel.h"
#include "be_bitmap.h"
#include "be_asmview.h"
#include "be_stencil.h"
#include "../src/checksum.h"

#include "../src/main.h"

typedef void (*gui_bineditor_tmpl_cb)(GuiBineditor *be, GtkWidget *ctx,  void *str, GtkWidget *wg);
#define GUI_BE_CB(x) ((gui_bineditor_tmpl_cb)(x))

typedef struct {
    char	*last_pattern;
    GtkWidget *rad0, *rad1;
    GtkWidget *from, *to;
    GtkWidget *pattern, *dlg;
    GuiBineditor *be;
} gui_clear_str;

typedef struct
{
    char *last_fstr, *last_rstr;
    char run_again;
    GtkWidget *find;
    GtkWidget *replace;
    GtkWidget *ci, *c0,*c1, *c2;
} gui_find_str;

typedef struct {
    GtkWidget *addr;
    GtkWidget *count;
    GtkWidget *arg;
    GtkWidget *sub,*add,*div,*mul,*or,*and,*xor;
    GtkWidget *shl,*sal,*shr,*sar,*rol,*ror;
    GtkWidget *bx0, *bx1, *bx2, *bx3, *bx4, *bx5, *bx6, *bx7;
    GtkWidget *bx;
} gui_be_bm_str;

typedef struct {
    GuiBineditor *be;
    GtkWidget *addr;
    GtkWidget *count;
    GtkWidget *all;
    GtkWidget *split;
    GtkWidget *merge;
    GtkWidget *xchg;
    GtkWidget *reorg;
    GtkWidget *bits;
    GtkWidget *vb;
    GtkWidget *a[32];  // address bits
    char bit_count;
} gui_be_org_str;

typedef struct {
    unsigned int l_width;
    unsigned int l_height;
    unsigned int l_mask;
    unsigned int l_rev;
    unsigned int r_width;
    unsigned int r_height;
    unsigned int r_mask;
    unsigned int r_rev;
    GtkWidget *mask[8];
    GuiBineditor *be;
    GtkWidget *width;
    GtkWidget *height;
    GtkWidget *rev;    
} gui_be_bmp_str;

typedef struct {
    GuiBineditor *be;    
    GtkWidget *start;
    GtkWidget *stop;
    GtkWidget *count;
} gui_be_cut_str;

typedef struct {
    GtkWidget *start;    
} gui_be_copy_str;

typedef struct {
    GuiBineditor *be;
    GtkWidget *count;    
    GtkWidget *proc;
    GtkWidget *fsel;
} gui_be_asm_str;
/*
typedef struct {
    GuiBineditor *be;
    GtkWidget *start;    
    GtkWidget *len;
} gui_be_text_str;
*/
typedef struct {
    GuiBineditor *be;
    GtkWidget *size;
} gui_be_aux_str;

typedef struct {
    GuiBineditor *be;
    GtkWidget *algo;
    GtkWidget *start;
    GtkWidget *stop;
    GtkWidget *result;
} gui_be_sum_str;

typedef struct {
    GuiBineditor *be;
    GtkWidget *size;
} gui_be_resize_str;

typedef struct {
    unsigned int l_offs;
    unsigned int l_ins;
    unsigned int l_count;
    unsigned int r_offs;
    unsigned int r_ins;
    unsigned int r_count;
    GtkWidget *foffs;
    GtkWidget *start;
    GtkWidget *count;
    char *fname;
    FILE *fh;
    long fsize;
} gui_be_open_str;

typedef struct {
    unsigned int l_ins;
    unsigned int l_count;
    unsigned int r_ins;
    unsigned int r_count;
    GtkWidget *start;
    GtkWidget *count;
    char *fname;
} gui_be_save_str;

typedef struct
{
    GuiBineditor *be;
    GtkWidget    *view, *brief;
    GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;
    GtkTreeStore *model;   // do wywalenia ?
    GtkTreeIter *top;
    GtkWidget *sw;
    FILE 	*idx_file;
    FILE	*stc_file;
    int x_id;
    char *x_name;
    char *x_path;
    char *x_fname;
    char has_child;
}  gui_bineditor_stencil_str;

enum{
    TREE_COL_ICON = 0,
    TREE_COL_TEXT,
    TREE_COL_FILE,
    TREE_COL_DESC,
    TREE_COL_ID,
    TREE_COL_ALL
};


static inline void gui_bineditor_stencil_build_tree(gui_bineditor_stencil_str *, const char *);
static inline void gui_bineditor_stencil_select_brief(gui_bineditor_stencil_str *, const char *, int);
static inline void gui_bineditor_stencil_selected(gui_bineditor_stencil_str *, const char *, const char *, int);
static inline void gui_bineditor_stencil_tree_popup_menu(gui_bineditor_stencil_str *, GtkMenuShell *, char *, int);
static void gui_bineditor_stencil_tree_operation(gui_bineditor_stencil_str *, int);

/****************************************************************************************************************/

void gui_bineditor_warning(GuiBineditor *be, const char *text)
{
    GtkWidget *dlg;
    dlg = gtk_message_dialog_new(GTK_WINDOW(be->priv->wmain), 
        GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_CLOSE,
        NULL
    );
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dlg), text);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy( dlg );
}

static void gui_bineditor_clear_exec( GuiBineditor *be, GtkWidget *ctx, gui_clear_str *str)
{
    unsigned int from, to;
    const char *pattern;

    from = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->from));
    to   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->to));
    pattern = gtk_entry_get_text(GTK_ENTRY(str->pattern));
    gui_bineditor_buff_clr(be->priv->buff, from, to, pattern);
    if( str->last_pattern == NULL ) 
	store_set(&store, "CLEAR_PATTERN", pattern);
    else {
	if( strcmp(str->last_pattern, pattern) ) 
	    store_set(&store, "CLEAR_PATTERN", pattern);
    }
    gui_bineditor_redraw( be );
}

static inline void gui_bineditor_lookup( GuiBineditor *be, GtkWidget *ctx, unsigned char *fstr, unsigned char *rstr, unsigned int flen, unsigned int rlen, unsigned int from, unsigned int to, char ci, char repl)
{
    GtkWidget *dlg, *hb;
    int resp;
    char ret = 0;
    char skip = 0;
    char fnd = 0, lf = 0;
    unsigned int from_l, to_l;

    dlg = gtk_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(dlg), GTK_WINDOW(be->priv->wmain));
    
    if( repl ){
        gtk_dialog_add_button(GTK_DIALOG(dlg), TXT_BE_FIND_ALL_BT, 1);
        gtk_dialog_add_button(GTK_DIALOG(dlg), TXT_BE_FIND_REPL_BT, 2);
    }

    gtk_dialog_add_button(GTK_DIALOG(dlg), GTK_STOCK_CANCEL, 3);
    gtk_dialog_add_button(GTK_DIALOG(dlg), GTK_STOCK_MEDIA_NEXT, 4);
    ctx = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_add(GTK_CONTAINER(ctx), hb);
    gtk_box_pack_start(GTK_BOX(hb), gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hb), gtk_label_new(TXT_BE_FIND_MATCH), FALSE, FALSE, 0);
    gtk_widget_show_all( ctx );            

    from_l = 0;
    to_l = 0;
    for(;; from++){
	if((ret = gui_bineditor_buff_find(be->priv->buff, (const char *)fstr, flen, &from, to, ci)) == 1){
	    fnd = 1; lf = 1;
	    from_l = from;
	    to_l = from + flen - 1;
	    if(!skip){
		gui_bineditor_marker_set_item(be, GUI_BINEDITOR_MARKER_FOUND, GUI_BINEDITOR_MARKER_HEX | GUI_BINEDITOR_MARKER_ASCII, from, to_l);
		gui_bineditor_show_grid( be, from, from + flen - 1 );
		resp = gtk_dialog_run(GTK_DIALOG(dlg));
		if(resp == 1) skip = 1;
	        if(resp == 3){
	    	    lf = 0;
	    	    break;
	        }
	        if(resp == 4) continue;
	    } else {
		resp = 2;
	    }
	    if(resp == 2){
		gui_bineditor_buff_history_add(be->priv->buff, from, from + rlen - 1);
		memcpy(be->priv->buff->data + from, rstr, rlen);
		gui_bineditor_show_grid( be, from, from + flen - 1 );
	    }
	} else {
	    lf = 0;
	    gtk_widget_destroy( dlg );
	    dlg = gtk_message_dialog_new(GTK_WINDOW(be->priv->wmain), 
    		GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		NULL
	    );
	    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dlg), TXT_BE_FIND_NO_MATCH);    
	    gtk_dialog_run(GTK_DIALOG(dlg));
    	    break;
	}
    }
    gtk_widget_destroy( dlg );
    if( fnd ){
	if( lf ) gui_bineditor_marker_set_item(be, GUI_BINEDITOR_MARKER_FOUND, GUI_BINEDITOR_MARKER_HEX | GUI_BINEDITOR_MARKER_ASCII, from, to);
	gui_bineditor_show_grid( be, from_l, to_l);
    }
}

static void gui_bineditor_find_exec( GuiBineditor *be, GtkWidget *ctx, gui_find_str *str  )
{
    unsigned int from, to;
    char ci;
    const char *find, *replace;

    int error = 0;
    char repl = 0;
    unsigned char *find_data, *replace_data;
    unsigned int find_size = 0, replace_size = 0; 

    find = gtk_entry_get_text(GTK_ENTRY(str->find));    
    replace = gtk_entry_get_text(GTK_ENTRY(str->replace));    

    if( *find == 0) return; // empty 'find' string, so return

    find_data = gui_bineditor_buff_pattern2data(find, &find_size, &error);
    if( error ){
	gui_bineditor_warning(be, TXT_BE_FIND_FIND_PATTERN);
	str->run_again = 1;
	return;
    }

    replace_data = gui_bineditor_buff_pattern2data(replace, &replace_size, &error);
    if( error ){
	gui_bineditor_warning(be, TXT_BE_FIND_REPLACE_PATTERN);    
	str->run_again = 1;
	return;
    }
    repl = replace_size > 0;

    from = 0; to = 0;
    gui_bineditor_marker_get_range(be, GUI_BINEDITOR_MARKER_SELECTED, &from, &to);

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->c0))){
	from = 0;
	to = be->priv->buff->size - 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->c1))){
	from = be->priv->address_mark;
	to = be->priv->buff->size - from - 1;
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->c2))){
	from = be->priv->clpb_start;
	to = be->priv->clpb_end;
    }

    ci = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->ci));

    if(( repl ) && ( find_size != replace_size )){
	gui_bineditor_warning(be, TXT_BE_FIND_ERR_NEQ);
	str->run_again = 1;
	return;
    }

    /* store entries*/
    if( find )
	store_set( &store, "FIND_LAST_PATTERN", find);
    if( replace )
	store_set( &store, "REPLACE_LAST_PATTERN", replace);

    /* do "find and replace" */
    gui_bineditor_lookup(be, ctx, find_data, replace_data, find_size, replace_size, from, to, ci, repl);
    
    if(find_data) free( find_data );
    if(replace_data) free( replace_data );
}

static void gui_bineditor_manipulator_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_bm_str *str )
{
    unsigned int start, count;
    int arg, i;
    char func;
    char rel[8];
    
    for( i = 0; i < 8; i++) rel[i] = i;

    start = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->addr));        
    count = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->count));        
    arg = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->arg));        

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->sub))) func = GUI_BINEDITOR_BM_FUNC_SUB;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->add))) func = GUI_BINEDITOR_BM_FUNC_ADD;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->mul))) func = GUI_BINEDITOR_BM_FUNC_MUL;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->div))) func = GUI_BINEDITOR_BM_FUNC_DIV;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->or ))) func = GUI_BINEDITOR_BM_FUNC_OR;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->and))) func = GUI_BINEDITOR_BM_FUNC_AND;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->xor))) func = GUI_BINEDITOR_BM_FUNC_XOR;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->shl))) func = GUI_BINEDITOR_BM_FUNC_SHL;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->sal))) func = GUI_BINEDITOR_BM_FUNC_SAL;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->shr))) func = GUI_BINEDITOR_BM_FUNC_SHR;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->sar))) func = GUI_BINEDITOR_BM_FUNC_SAR;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->rol))) func = GUI_BINEDITOR_BM_FUNC_ROL;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->ror))) func = GUI_BINEDITOR_BM_FUNC_ROR;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->bx ))){
	func = GUI_BINEDITOR_BM_FUNC_BIT;
	rel[0] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->bx0));
	rel[1] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->bx1));
	rel[2] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->bx2));
	rel[3] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->bx3));
	rel[4] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->bx4));
	rel[5] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->bx5));
	rel[6] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->bx6));
	rel[7] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->bx7));
    }
    if( (arg == 0) && (func == GUI_BINEDITOR_BM_FUNC_DIV)){
	gui_bineditor_warning(be, TXT_MANIPULATOR_DIVBYZERO);    
	return;
    }
    
    gui_bineditor_buff_bman(be->priv->buff, start, count, arg, func, rel);    
    gui_bineditor_redraw( be );
}

static void gui_bineditor_organizer_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_org_str *str )
{
    unsigned int addr, count, i;
    char op = 0;
    char reorg[32];
    
    addr  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->addr));        
    count = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->count));        

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->split))) op = GUI_BINEDITOR_ORG_SPLIT;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->merge))) op = GUI_BINEDITOR_ORG_MERGE;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->xchg ))) op = GUI_BINEDITOR_ORG_XCHG;
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->reorg))) op = GUI_BINEDITOR_ORG_REORG;
    
    for( i =0; i < 32; i++){
	 reorg[i] = -1;
	 if( str->a[i] ) reorg[i] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->a[i]));
    }
    if( count & 1){
	gui_bineditor_warning(be, TXT_ORGANIZER_EVEN);
	return;
    }

    gui_bineditor_buff_reorg(be->priv->buff, addr, count, op, reorg, str->bit_count);
    gui_bineditor_redraw( be );
}

static void gui_be_cut_get_values(gui_be_cut_str *str, int *start, int *count, int *stop)
{
    *start = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->start));
    *count = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->count));
    *stop  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->stop));
}

static void gui_bineditor_cut_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_cut_str *str )
{
    int start, stop, count;
    
    gui_be_cut_get_values( str, &start, &count, &stop );

    gui_bineditor_cut_store( be, start, stop - 1);
}

static void gui_bineditor_copy_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_copy_str *str  )
{
    gui_bineditor_cut_restore(be, gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->start)));
}

static void gui_bineditor_bined_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_bmp_str *str )
{
    unsigned int i;
  
    str->r_width  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->width));
    str->r_height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->height)); 
    str->r_rev = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->rev));

    str->r_mask = 0;
    for(i = 0; i < 8; i++) 
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(str->mask[7 - i]))) str->r_mask |= 1 << i;
        
    gui_bineditor_bitmap( be, str->r_width, str->r_height, str->r_mask, str->r_rev);
}

static void gui_bineditor_asm_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_asm_str *str  )
{
    const char *text;
    
    if(be->priv->core_name != NULL) 
		g_free(be->priv->core_name);

    text = gtk_entry_get_text(GTK_ENTRY(str->proc));
    be->priv->core_name = (char *)malloc( strlen(text) + 1 );
    if(be->priv->core_name == NULL){
	printf("memory error: gui_bineditor_asm_exec()\n");
	return;
    }
    strcpy(be->priv->core_name, text);
    be->priv->core_count = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->count));
    

    gui_bineditor_asm_view(be, be->priv->core_count, be->priv->core_name);
}
/*
static void gui_bineditor_text_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_text_str *str  )
{
    gui_bineditor_text_editor(be, 
	gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->start)),
	gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->len)),
	16 // temporarly
    );
}
*/
static void gui_bineditor_aux_destr( GtkWidget *wg, GuiBineditor *p )
{
    if(p->priv->aux_buffer) free(p->priv->aux_buffer);
    p->priv->aux_buffer = NULL;
    p->priv->aux_size = 0;
    gtk_widget_set_sensitive(p->priv->aux, TRUE);
}

static void gui_bineditor_aux_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_aux_str *str )
{
    unsigned int size;
    GtkWidget *aux;
    
    size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->size));
    be->priv->aux_buffer = (char *)malloc(size);
    if(be->priv->aux_buffer == NULL){
	printf("memory error: gui_bineditor_aux_exec()\n");
	return;
    }
    be->priv->aux_size = size;
    memset(be->priv->aux_buffer, 0, size);

    be->priv->aux_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(be->priv->aux_win), 640,480);
    gtk_window_set_title(GTK_WINDOW(be->priv->aux_win), TXT_BE_WINTIT_AUX);

    aux = gui_bineditor_new(GTK_WINDOW(be->priv->aux_win));
    gui_bineditor_file_tool_insert( GUI_BINEDITOR(aux) );
    gui_bineditor_set_icon( GUI_BINEDITOR(aux), be->priv->icon);
    if(be->priv->icon)
	gtk_window_set_icon(GTK_WINDOW(be->priv->aux_win), gdk_pixbuf_new_from_xpm_data(be->priv->icon));
    
    if(aux == NULL){
	printf("WARN:gui_bineditor_aux_exec() -> Cannot create bineditor.\n");
	return;
    }

    GUI_BINEDITOR(aux)->priv->root = be;
    be->priv->aux_ed = GUI_BINEDITOR(aux);
    gui_bineditor_set_buffer(GUI_BINEDITOR(aux), size, (unsigned char *)be->priv->aux_buffer);
    gtk_container_add(GTK_CONTAINER(be->priv->aux_win), GTK_WIDGET(aux));
    gtk_widget_show_all(be->priv->aux_win);

    gtk_widget_hide(GUI_BINEDITOR(aux)->priv->aux);
    gtk_widget_show(GUI_BINEDITOR(aux)->priv->i_open);
    gtk_widget_show(GUI_BINEDITOR(aux)->priv->i_write);

    if(be->priv->cut_data.data)
	gtk_widget_set_sensitive(GUI_BINEDITOR(aux)->priv->copy, TRUE);	

    gtk_widget_hide(GUI_BINEDITOR(aux)->priv->stenc);
    g_signal_connect(G_OBJECT(be->priv->aux_win), "destroy", G_CALLBACK(gui_bineditor_aux_destr), be );
}

static void gui_bineditor_sum_exec( GtkWidget *wg, gui_be_sum_str *str )
{
    unsigned int start, stop;
    ChecksumAlgo algo;
    int result = 0;
    const char *id;
    char res[16];


    start = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->start));
    stop  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->stop));
    id    = gtk_combo_box_get_active_id(GTK_COMBO_BOX(str->algo));
    
    switch(*id){
	case 'A': algo = CHECKSUM_ALG_LRC; break;
	case 'B': algo = CHECKSUM_ALG_CRC16; break;
	case 'C': algo = CHECKSUM_ALG_CRC32; break;
	default : algo = CHECKSUM_ALG_LRC;
    }

    result = checksum_calculate(algo, str->be->priv->buff->size, str->be->priv->buff->data, start, stop, 0,0, 0,0 );

    if( algo == CHECKSUM_ALG_CRC32) sprintf(res, "0x%X%X%X%X%X%X%X%X",
	(result >> 28 ) & 0xf,
	(result >> 24 ) & 0xf,
	(result >> 20 ) & 0xf,
	(result >> 16 ) & 0xf,
	(result >> 12 ) & 0xf,
	(result >> 8  ) & 0xf,
	(result >> 4  ) & 0xf,
	(result       ) & 0xf
     );

    if( algo == CHECKSUM_ALG_CRC16) sprintf(res, "0x%X%X%X%X",
	(result >> 12 ) & 0xf,
	(result >> 8  ) & 0xf,
	(result >> 4  ) & 0xf,
	(result       ) & 0xf
     );

    if( algo == CHECKSUM_ALG_LRC) sprintf(res, "0x%X%X",
	(result >> 4  ) & 0xf,
	(result       ) & 0xf
     );

    gtk_entry_set_text(GTK_ENTRY(str->result), res);
}

static void gui_bineditor_resize_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_resize_str *str )
{
    unsigned int new_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->size));

    if(new_size <= be->priv->buff->size) return;
    
    be->priv->buff->size = new_size;
    be->priv->buff->data = realloc(be->priv->buff->data, new_size);

    if(be->priv->buff->data == NULL){
	printf("memory error in gui_bineditor_resize_exec()\n");
	exit(-1);
    }
    gui_bineditor_set_buffer(be, new_size, be->priv->buff->data);
}

static void gui_bineditor_open_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_open_str *str )
{
    str->r_offs = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->foffs));
    str->r_ins = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->start));
    str->r_count = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->count));

    gui_bineditor_buff_file_insert(be->priv->buff, str->fh, str->r_offs, str->r_ins, str->r_count);
    gui_bineditor_redraw( be );
}

static void gui_bineditor_save_exec( GuiBineditor *be, GtkWidget *ctx, gui_be_save_str *str )
{
    str->r_ins = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->start)),
    str->r_count = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->count)),
    gui_bineditor_buff_file_save(be->priv->buff, str->r_ins, str->r_count, str->fname );
}

/****************************************************************************************************************************************/

static void gui_bineditor_build_open( GuiBineditor *be, GtkWidget *ctx, gui_be_open_str *str)
{
    GtkWidget *wg;

    str->fh = fopen(str->fname, "r");
    if(str->fh == NULL){
	printf("error open file gui_bineditor_build_open()\n");
	return;
    }
    
    fseek(str->fh, 0L, SEEK_END);
    str->fsize = ftell(str->fh);
    fseek(str->fh, 0L, SEEK_SET);    

    str->foffs = gtk_spin_button_new_with_range( 0, str->fsize - 1, 1);
    str->start = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);
    str->count = gtk_spin_button_new_with_range( 1, be->priv->buff->size, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->foffs), str->l_offs);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->start), str->l_ins);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), str->l_count);    

    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(wg), gtk_label_new( TXT_BE_OPEN_FOFFS));
    gtk_container_add(GTK_CONTAINER(wg), str->foffs);
    gtk_container_add(GTK_CONTAINER(ctx), wg);

    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(wg), gtk_label_new( TXT_BE_OPEN_START));
    gtk_container_add(GTK_CONTAINER(wg), str->start);
    gtk_container_add(GTK_CONTAINER(ctx), wg);

    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(wg), gtk_label_new( TXT_BE_OPEN_COUNT));
    gtk_container_add(GTK_CONTAINER(wg), str->count);
    gtk_container_add(GTK_CONTAINER(ctx), wg);
// add guard    
}

static void gui_bineditor_build_save( GuiBineditor *be, GtkWidget *ctx, gui_be_save_str *str)
{
    GtkWidget *wg;

    str->start = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);
    str->count = gtk_spin_button_new_with_range( 1, be->priv->buff->size, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->start), str->l_ins);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), str->l_count);    

    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(wg), gtk_label_new( TXT_BE_OPEN_START));
    gtk_container_add(GTK_CONTAINER(wg), str->start);
    gtk_container_add(GTK_CONTAINER(ctx), wg);

    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(wg), gtk_label_new( TXT_BE_OPEN_COUNT));
    gtk_container_add(GTK_CONTAINER(wg), str->count);
    gtk_container_add(GTK_CONTAINER(ctx), wg);
// add guard    
}

static void gui_bineditor_build_resize( GuiBineditor *be, GtkWidget *ctx, gui_be_resize_str *str)
{
    GtkWidget *wg;
    
    str->be = be;
    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(ctx), wg);    
    gtk_container_add(GTK_CONTAINER(wg), gtk_label_new(TXT_BE_RESIZE_RES));
    str->size = gtk_spin_button_new_with_range( be->priv->buff->size, be->priv->buff->size * 16, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->size), be->priv->buff->size);
    gtk_container_add(GTK_CONTAINER(wg), str->size);    
}

static void gui_bineditor_build_sum( GuiBineditor *be, GtkWidget *ctx, gui_be_sum_str *str)
{
    GtkWidget *wg, *bt;
    unsigned int from = 0, to = 0;

    if(be->priv->buff->size < 2) return;

    if(!gui_bineditor_marker_get_range(be, GUI_BINEDITOR_MARKER_SELECTED, &from, &to)){
	from = 0;
	to = be->priv->buff->size - 1;
    }

    str->be = be;
    str->algo = gtk_combo_box_text_new();
    gtk_container_add(GTK_CONTAINER(ctx), str->algo);    
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(str->algo), (const char *)"A", TXT_BE_SUM_ALGO_LRC);
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(str->algo), (const char *)"B", TXT_BE_SUM_ALGO_CRC16);
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(str->algo), (const char *)"C", TXT_BE_SUM_ALGO_CRC32);
    gtk_combo_box_set_active(GTK_COMBO_BOX(str->algo), 0);

    str->start = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 2, 1);
    str->stop  = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->start), from);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->stop), to);
    gtk_container_add(GTK_CONTAINER(ctx), str->start);    
    gtk_container_add(GTK_CONTAINER(ctx), str->stop);    

    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(ctx), wg);    
    str->result = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(wg), str->result);    
    bt = gtk_button_new_with_label(TXT_BE_SUM_RUN);
    gtk_container_add(GTK_CONTAINER(wg), bt);    
    gtk_editable_set_editable(GTK_EDITABLE(str->result), FALSE);
    g_signal_connect(G_OBJECT(bt), "pressed", G_CALLBACK(gui_bineditor_sum_exec), str );
}

static void gui_bineditor_build_aux( GuiBineditor *be, GtkWidget *ctx, gui_be_aux_str *str)
{
    GtkWidget *wg;
    unsigned int r;    
    
    str->be = be;
    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(ctx), wg);    
    gtk_container_add(GTK_CONTAINER(wg), gtk_label_new(TXT_BE_AUX_SIZE));
    r = be->priv->buff->size;
    if(r < 16) r = 32;
    str->size = gtk_spin_button_new_with_range( 16, r, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->size), r);
    gtk_container_add(GTK_CONTAINER(wg), str->size);    
}

static void gui_bineditor_build_bined( GuiBineditor *be, GtkWidget *ctx, gui_be_bmp_str *str)
{
    GtkWidget *wg, *lb, *ww, *frm;
    int i;

    str->width  = gtk_spin_button_new_with_range( 0, 256, 1);
    str->height = gtk_spin_button_new_with_range( 0, 256, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->width), str->l_width);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->height), str->l_height);

    wg = gtk_table_new(2, 2, TRUE);
    gtk_container_add(GTK_CONTAINER(ctx), wg);
    lb = gtk_label_new(TXT_BE_BMP_WIDTH);
    gtk_misc_set_alignment(GTK_MISC(lb), 0.0f, 0.5f);
    gtk_table_attach_defaults(GTK_TABLE(wg), lb, 0,1, 0,1);
    lb = gtk_label_new(TXT_BE_BMP_HEIGHT);
    gtk_misc_set_alignment(GTK_MISC(lb), 0.0f, 0.5f);
    gtk_table_attach_defaults(GTK_TABLE(wg), lb, 0,1, 1,2);    
    gtk_table_attach_defaults(GTK_TABLE(wg), str->width,  1,2, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(wg), str->height, 1,2, 1,2);


    frm = gtk_frame_new(TXT_BE_BMP_BS);
    gtk_container_add(GTK_CONTAINER(ctx), frm);
    ww = gtk_table_new(2, 8, TRUE);
    gtk_container_add(GTK_CONTAINER(frm), ww);
    gtk_table_attach_defaults(GTK_TABLE(ww), gtk_label_new("7"), 0,1, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(ww), gtk_label_new("6"), 1,2, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(ww), gtk_label_new("5"), 2,3, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(ww), gtk_label_new("4"), 3,4, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(ww), gtk_label_new("3"), 4,5, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(ww), gtk_label_new("2"), 5,6, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(ww), gtk_label_new("1"), 6,7, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(ww), gtk_label_new("0"), 7,8, 0,1);
//-->
    for(i = 8; i; i--){
            str->mask[i - 1] = gtk_check_button_new();
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(str->mask[i - 1]), (str->l_mask & (0x80 >> (i - 1))) ? TRUE : FALSE);
            gtk_table_attach_defaults(GTK_TABLE(ww), str->mask[i - 1], i - 1, i, 1,2);
    }
    
    str->rev = gtk_check_button_new_with_label(TXT_BE_BMP_REV);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(str->rev), str->l_rev);
    gtk_container_add(GTK_CONTAINER(ctx), str->rev);    

}

/* Find and replace */
static void gui_bineditor_build_find_string( GuiBineditor *be, GtkWidget *ctx, gui_find_str *str )
{
    GtkWidget *hb, *table;
    char *tmp0, *tmp1;

    /* get last entered text into entries */
    store_get( &store, "FIND_LAST_PATTERN", &str->last_fstr);
    store_get( &store, "REPLACE_LAST_PATTERN", &str->last_rstr);

    /* entries */
    gtk_box_pack_start(GTK_BOX(ctx), gtk_label_new(TXT_BE_FIND_ENTRY), FALSE, FALSE, 2);
    str->find = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(ctx), str->find, FALSE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(ctx), gtk_label_new(TXT_BE_REPLACE_ENTRY), FALSE, FALSE, 2);
    hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(ctx), hb, FALSE, TRUE, 2);
    str->replace = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hb), str->replace, TRUE, TRUE, 0);

    /* parameters */
    hb = gtk_frame_new(TXT_BE_FIND_ST_LABEL);
    gtk_frame_set_label_align(GTK_FRAME(hb), 0.5, 0.5);
    gtk_container_add(GTK_CONTAINER(ctx), hb);
    table = gtk_table_new(3, 2, TRUE);
    gtk_container_add(GTK_CONTAINER(hb), table);
    str->ci = gtk_check_button_new_with_label(TXT_BE_FIND_ST_CI);
    str->c0 = gtk_radio_button_new_with_label(NULL, TXT_BE_FIND_ST_BEGIN);
    str->c1 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->c0), TXT_BE_FIND_ST_CURSOR);
    str->c2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->c0), TXT_BE_FIND_ST_MARKED);
    gtk_table_attach_defaults(GTK_TABLE(table), str->c0, 0,1, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(table), str->c1, 0,1, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(table), str->c2, 0,1, 2,3);
    gtk_table_attach(GTK_TABLE(table), str->ci, 1,2, 0,1, GTK_SHRINK, GTK_SHRINK, 10, 2);
    
    tmp1 = "";
    tmp0 = (str->last_fstr == NULL)  ? tmp1 : str->last_fstr;
    gtk_entry_set_text(GTK_ENTRY(str->find), tmp0);
    tmp0 = (str->last_rstr == NULL)  ? tmp1 : str->last_rstr;
    gtk_entry_set_text(GTK_ENTRY(str->replace), tmp0);
}

/**/
void gui_bineditor_bx( GtkWidget *tb, GtkWidget *wg, const char *lbl, int x0, int x1, int y0, int y1)
{
    GtkWidget *hb;

    hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);    
    gtk_table_attach_defaults(GTK_TABLE(tb), hb,  x0,x1, y0,y1);
    gtk_container_add(GTK_CONTAINER(hb), gtk_label_new( lbl ));
    gtk_container_add(GTK_CONTAINER(hb), wg);
}

void gui_bineditor_bmbx_set_sensitive( gui_be_bm_str *str, char x)
{
    gtk_widget_set_sensitive(str->bx0, x);    
    gtk_widget_set_sensitive(str->bx1, x);    
    gtk_widget_set_sensitive(str->bx2, x);    
    gtk_widget_set_sensitive(str->bx3, x);    
    gtk_widget_set_sensitive(str->bx4, x);    
    gtk_widget_set_sensitive(str->bx5, x);    
    gtk_widget_set_sensitive(str->bx6, x);    
    gtk_widget_set_sensitive(str->bx7, x);    
}

void gui_bineditor_bx_sens(GtkToggleButton *tg, gui_be_bm_str *str)
{
    if(gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( tg ))){
	gui_bineditor_bmbx_set_sensitive( str, 1);
	gtk_widget_set_sensitive(str->arg, 0);    
    }else{
        gui_bineditor_bmbx_set_sensitive( str, 0);
	gtk_widget_set_sensitive(str->arg, 1);    
    }
}

static void gui_bineditor_build_manipulator( GuiBineditor *be, GtkWidget *ctx, gui_be_bm_str *str )
{
    GtkWidget *wg, *tb, *vb;
    unsigned int from = 0, to = 0;
// input parameters
    wg = gtk_table_new(3, 2, TRUE);
    gtk_container_add(GTK_CONTAINER(ctx), wg);    
    gtk_table_attach_defaults(GTK_TABLE(wg), gtk_label_new(TXT_BE_BM_START_ADDRESS), 0,1, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(wg), gtk_label_new(TXT_BE_BM_COUNT), 0,1, 1,2);    
    gtk_table_attach_defaults(GTK_TABLE(wg), gtk_label_new(TXT_BE_BM_ARGUMENT), 0,1, 2,3);
    str->addr  = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);//gtk_entry_new();
    str->count = gtk_spin_button_new_with_range( 0, be->priv->buff->size, 1);//gtk_entry_new();
    str->arg   = gtk_spin_button_new_with_range( 0, 255, 1);//gtk_entry_new();

    if(!gui_bineditor_marker_get_range(be, GUI_BINEDITOR_MARKER_SELECTED, &from, &to)){
	from = 0;
	to = be->priv->buff->size - 1;
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->addr), from);    
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), to - from + 1);
    gtk_table_attach_defaults(GTK_TABLE(wg), str->addr,  1,2, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(wg), str->count, 1,2, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(wg), str->arg,   1,2, 2,3);
// functions to select
    wg = gtk_frame_new(TXT_BE_BM_FUNCTIONS);
    gtk_container_add(GTK_CONTAINER(ctx), wg);    
    tb = gtk_table_new( 5, 4, TRUE);
    gtk_container_add(GTK_CONTAINER(wg), tb);    
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(TXT_BE_BM_ARITHM), 0,1, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(TXT_BE_BM_LOGIC),  1,2, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(TXT_BE_BM_SHIFT),  2,3, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(TXT_BE_BM_ROTATE), 3,4, 0,1);

    str->sub = gtk_radio_button_new_with_label(NULL, TXT_BE_BM_SUB);
    str->add = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_ADD);
    str->mul = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_MUL);
    str->div = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_DIV);
    str->or =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_OR);
    str->and = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_AND);
    str->xor = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_XOR);
    str->shl = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_SHL);
    str->sal = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_SAL);
    str->shr = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_SHR);
    str->sar = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_SAR);
    str->rol = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_ROL);
    str->ror = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_ROR);

    gtk_table_attach_defaults(GTK_TABLE(tb), str->sub, 0,1, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->add, 0,1, 2,3);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->mul, 0,1, 3,4);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->div, 0,1, 4,5);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->or,  1,2, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->and, 1,2, 2,3);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->xor, 1,2, 3,4);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->shl, 2,3, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->sal, 2,3, 2,3);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->shr, 2,3, 3,4);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->sar, 2,3, 4,5);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->rol, 3,4, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->ror, 3,4, 2,3);
// bit exchg
    wg = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(ctx), wg);    
    vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(wg), vb);    
    str->bx = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->sub), TXT_BE_BM_BX);    
    gtk_container_add(GTK_CONTAINER(vb), str->bx);    
    tb = gtk_table_new( 4, 2, TRUE);
    gtk_container_add(GTK_CONTAINER(vb), tb);    
    
    str->bx0 = gtk_spin_button_new_with_range( 0, 7, 1);
    str->bx1 = gtk_spin_button_new_with_range( 0, 7, 1);
    str->bx2 = gtk_spin_button_new_with_range( 0, 7, 1);
    str->bx3 = gtk_spin_button_new_with_range( 0, 7, 1);
    str->bx4 = gtk_spin_button_new_with_range( 0, 7, 1);
    str->bx5 = gtk_spin_button_new_with_range( 0, 7, 1);
    str->bx6 = gtk_spin_button_new_with_range( 0, 7, 1);
    str->bx7 = gtk_spin_button_new_with_range( 0, 7, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->bx0), 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->bx1), 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->bx2), 2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->bx3), 3);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->bx4), 4);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->bx5), 5);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->bx6), 6);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->bx7), 7);

    gui_bineditor_bx(tb, str->bx0, "0->", 0,1, 0,1);
    gui_bineditor_bx(tb, str->bx1, "1->", 0,1, 1,2);    
    gui_bineditor_bx(tb, str->bx2, "2->", 0,1, 2,3);    
    gui_bineditor_bx(tb, str->bx3, "3->", 0,1, 3,4);    
    gui_bineditor_bx(tb, str->bx4, "4->", 1,2, 0,1);    
    gui_bineditor_bx(tb, str->bx5, "5->", 1,2, 1,2);    
    gui_bineditor_bx(tb, str->bx6, "6->", 1,2, 2,3);    
    gui_bineditor_bx(tb, str->bx7, "7->", 1,2, 3,4);    

    gui_bineditor_bmbx_set_sensitive( str, 0);
    g_signal_connect(G_OBJECT(str->bx), "toggled", G_CALLBACK(gui_bineditor_bx_sens), str );
}

static void gui_clear_radio_whole(GtkWidget *wg, gui_clear_str *str)
{
    if(!gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(str-> rad0))) return;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->from), 0);    
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->to), str->be->priv->buff->size - 1);
    gtk_widget_set_sensitive(str->from, 0);    
    gtk_widget_set_sensitive(str->to, 0);
}

static void gui_clear_radio_marked(GtkWidget *wg, gui_clear_str *str)
{
    unsigned int from, to;
    if(!gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(str->rad1))) return;
    
    if( !gui_bineditor_marker_get_range(str->be, GUI_BINEDITOR_MARKER_SELECTED, &from, &to) ){
	from = 0;
	to = str->be->priv->buff->size - 1;
    }
    
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->from), from);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->to), to);
    gtk_widget_set_sensitive(str->from, 1);
    gtk_widget_set_sensitive(str->to, 1);
}

static void gui_clear_enter(GtkEntry *entry, gui_clear_str *str)
{
    gtk_dialog_response(GTK_DIALOG(str->dlg), GTK_RESPONSE_OK);
}

/* Clear buffer */
static void gui_bineditor_build_clear( GuiBineditor *be, GtkWidget *ctx,  gui_clear_str *str, GtkWidget *dlg)
{
    GtkWidget *hb, *wg;
// Gui
    /* Radio buttons */
    str->dlg = dlg;
    str->rad0 = gtk_radio_button_new_with_label(NULL, TXT_BE_WHOLE_BUFFER);
    str->rad1 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->rad0), TXT_BE_MARKED_AREA);
    gtk_box_pack_start(GTK_BOX(ctx), str->rad0, FALSE, FALSE, 2);    
    gtk_box_pack_start(GTK_BOX(ctx), str->rad1, FALSE, FALSE, 2);        
    /* Address range */    
    hb = gtk_label_new(TXT_BE_ADDRESS_RANGE);
    gtk_box_pack_start(GTK_BOX(ctx), hb, FALSE, TRUE, 2);
    hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);    
	/* From */
	gtk_box_pack_start(GTK_BOX(ctx), hb, FALSE, FALSE, 2);        
	wg = gtk_label_new(TXT_BE_ADDRESS_FROM);
	gtk_box_pack_start(GTK_BOX(hb), wg, FALSE, FALSE, 4);        
	str->from = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->from), 0);
	gtk_container_add(GTK_CONTAINER(hb), str->from);            
	gtk_widget_set_sensitive(str->from, 0);    
	/* to */
	wg = gtk_label_new(TXT_BE_ADDRESS_TO);
	gtk_box_pack_start(GTK_BOX(hb), wg, FALSE, FALSE, 4);        
	str->to = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->to), be->priv->buff->size - 1);
	gtk_container_add(GTK_CONTAINER(hb), str->to);            
	gtk_widget_set_sensitive(str->to, 0);    
    /* Pattern */
    store_get(&store, "CLEAR_PATTERN", &str->last_pattern); // global !
    hb = gtk_label_new(TXT_BE_PATTERN);
    gtk_box_pack_start(GTK_BOX(ctx), hb, FALSE, FALSE, 2);
    str->pattern = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(ctx), str->pattern, FALSE, FALSE, 8);
    gtk_entry_set_text(GTK_ENTRY(str->pattern), (str->last_pattern != NULL) ? str->last_pattern : "0xff");
// Signals
    g_signal_connect(G_OBJECT(str->rad0), "toggled", G_CALLBACK(gui_clear_radio_whole), str);
    g_signal_connect(G_OBJECT(str->rad1), "toggled", G_CALLBACK(gui_clear_radio_marked), str);    
    g_signal_connect(G_OBJECT(str->pattern), "activate", G_CALLBACK(gui_clear_enter), str);
}

static void gui_bineditor_as_sensitive(gui_be_org_str *str, char sens)
{
    int i;
    for(i = 0; i < 32; i++)
	if(str->a[i])
	    gtk_widget_set_sensitive(str->a[i], sens);

}

static void gui_bineditor_as_show(gui_be_org_str *str, GtkWidget *tb, const char *lbl, int row, int bl)
{
    int i;
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(lbl),  0,1, row, row + 1);
    for(i = 0; i < 8; i++)
	if(str->a[i + bl]){
	    gtk_table_attach_defaults(GTK_TABLE(tb), str->a[i + bl],  i + 1, i + 2, row, row + 1);
	    gtk_widget_set_sensitive(str->a[i + bl], FALSE);
	}
}

static char gui_bineditor_get_bits_size( unsigned int size )
{
    unsigned int i;
    unsigned int mask = 1 << 31;

    if( size > 0) size--; else return 0;
        
    for(i = 32; (i > 0) && !( size & mask); i--, mask >>= 1);
    return i - 1;
}

static GtkWidget *gui_bineditor_set_as(gui_be_org_str *str, unsigned int size)
{    
    GtkWidget *wg, *tb;
    int i, bits;

    bits = gui_bineditor_get_bits_size(size);
    wg = gtk_frame_new(TXT_BE_ORG_REORG_ADDRS);
    if(!bits) return wg;

    tb = gtk_table_new((bits / 8) + 1, 9, TRUE);
    gtk_container_add(GTK_CONTAINER(wg), tb);

    for(i = 0; i < 32; i++){
	if( i <= bits){
	    str->a[i] = gtk_spin_button_new_with_range( 0, bits, 1);
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->a[i]), i);
	}else
	    str->a[i] = NULL;
    }
    str->bit_count = bits;
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new("+0"),  1,2, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new("+1"),  2,3, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new("+2"),  3,4, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new("+3"),  4,5, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new("+4"),  5,6, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new("+5"),  6,7, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new("+6"),  7,8, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new("+7"),  8,9, 0,1);

    gui_bineditor_as_show( str, tb, "00:", 1, 0);
    if(bits > 7)
	gui_bineditor_as_show( str, tb, "08:", 2, 8);
    if(bits > 15)
	gui_bineditor_as_show( str, tb, "16:", 3, 16);
    if(bits > 23)
	gui_bineditor_as_show( str, tb, "24:", 4, 24);
    return wg;
}

static void gui_be_org_bit_rst( gui_be_org_str *str )
{
    gtk_widget_destroy(str->bits);
    str->bits = gui_bineditor_set_as(str, gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->count)));
    gtk_box_pack_start(GTK_BOX(str->vb), str->bits, FALSE, FALSE, 10);
    gui_bineditor_as_sensitive( str, gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(str->reorg)) );
    gtk_widget_show_all(str->vb);
}

static void gui_be_org_bt_sig(GtkWidget *wg, gui_be_org_str *str)
{
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->addr), 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), str->be->priv->buff->size);
    gui_be_org_bit_rst( str );
}

static void gui_be_org_reorg_rad(GtkWidget *wg, gui_be_org_str *str)
{
    gui_bineditor_as_sensitive( str, gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(str->reorg)) );
}

static void gui_be_org_chval(GtkWidget *wg, gui_be_org_str *str)
{
    unsigned int start = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->addr));
    unsigned int count = gtk_spin_button_get_value(GTK_SPIN_BUTTON(str->count));

    if( start + count >= str->be->priv->buff->size - 1){
	count = str->be->priv->buff->size - start;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), count);	
    }
}

static void gui_be_org_unfoc(GtkWidget *wg, GdkEvent *ev, gui_be_org_str *str)
{
    gui_be_org_bit_rst( str );
}

static void gui_bineditor_build_organizer( GuiBineditor *be, GtkWidget *ctx, gui_be_org_str *str )
{
    GtkWidget *wg, *vb, *lb;
    unsigned int start = 0, count = 0, to = 0;
    
    if(!gui_bineditor_marker_get_range(be, GUI_BINEDITOR_MARKER_SELECTED, &start, &to)){
	start = 0;
	to = be->priv->buff->size - 1;
    }

// initial values
    str->be = be;
    count = to - start + 1;
    if(count < 0) count = 0;
    if(start < 0) start = 0;
    if(!(count | start )){
	start = 0;
	count = be->priv->buff->size;
    }
    
// input parameters
    wg = gtk_table_new(3, 2, TRUE);
    gtk_box_pack_start(GTK_BOX(ctx), wg, TRUE, TRUE, 5);
    lb = gtk_label_new(TXT_BE_BM_START_ADDRESS);
    gtk_misc_set_alignment(GTK_MISC(lb), 0.0f, 0.5f);
    gtk_table_attach_defaults(GTK_TABLE(wg), lb, 0,1, 0,1);
    lb = gtk_label_new(TXT_BE_BM_COUNT);
    gtk_misc_set_alignment(GTK_MISC(lb), 0.0f, 0.5f);
    gtk_table_attach_defaults(GTK_TABLE(wg), lb, 0,1, 1,2);    
    str->addr  = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);//gtk_entry_new();
    str->count = gtk_spin_button_new_with_range( 0, be->priv->buff->size, 1);//gtk_entry_new();
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->addr), start);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), count);
    str->all  = gtk_button_new_with_label(TXT_BE_ORG_WHOLE);

    gtk_table_attach_defaults(GTK_TABLE(wg), str->addr,  1,2, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(wg), str->count, 1,2, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(wg), str->all,   1,2, 2,3);
// functions to select
    wg = gtk_frame_new(TXT_BE_ORG_FUNCTIONS);
    gtk_container_add(GTK_CONTAINER(ctx), wg);    
    vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(wg), vb);    

    str->split = gtk_radio_button_new_with_label(NULL, TXT_BE_ORG_SPLIT);
    str->merge = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->split), TXT_BE_ORG_MERGE);
    str->xchg  = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->split), TXT_BE_ORG_XCHG);
    str->reorg = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(str->split), TXT_BE_ORG_REORG);

    gtk_container_add(GTK_CONTAINER(vb), str->split);    
    gtk_container_add(GTK_CONTAINER(vb), str->merge);    
    gtk_container_add(GTK_CONTAINER(vb), str->xchg);    
    gtk_container_add(GTK_CONTAINER(vb), str->reorg);    
// address bits
    str->vb = vb;
    str->bits = gui_bineditor_set_as(str, count);
    gtk_box_pack_start(GTK_BOX(str->vb), str->bits, FALSE, FALSE, 10);
// signal connects
    g_signal_connect(G_OBJECT(str->reorg), "toggled", G_CALLBACK(gui_be_org_reorg_rad), str);
    g_signal_connect(G_OBJECT(str->all), "pressed", G_CALLBACK(gui_be_org_bt_sig), str);
    g_signal_connect(G_OBJECT(str->addr),  "value-changed", G_CALLBACK(gui_be_org_chval), str);    
    g_signal_connect(G_OBJECT(str->count), "value-changed", G_CALLBACK(gui_be_org_chval), str);    
    g_signal_connect(G_OBJECT(str->addr),  "focus-out-event", G_CALLBACK(gui_be_org_unfoc), str);    
    g_signal_connect(G_OBJECT(str->count), "focus-out-event", G_CALLBACK(gui_be_org_unfoc), str);    

}

static void gui_be_cut_start(GtkWidget *wg, gui_be_cut_str *str)
{
    int start, stop, count;
    
    gui_be_cut_get_values( str, &start, &count, &stop );

    if(start > stop){
	stop = start;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->stop), stop);
    }

    if(start - stop != count - 1){
	stop = start + count - 1;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->stop), stop);
    }
    
    if(start + count >= str->be->priv->buff->size){
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), str->be->priv->buff->size - start);
    }
}

static void gui_be_cut_stop(GtkWidget *wg, gui_be_cut_str *str)
{
    int start, stop, count;
    
    gui_be_cut_get_values( str, &start, &count, &stop );

    if(stop < start){
	stop = start;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->stop), stop);
    }

    if( stop >= str->be->priv->buff->size){
	stop = str->be->priv->buff->size - 1;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->stop), stop);
    }

    if(start - stop != count - 1){
	count = stop - start + 1;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), count);
    }
}

static void gui_bineditor_build_cut( GuiBineditor *be, GtkWidget *ctx, gui_be_cut_str *str )
{
    GtkWidget *tb;
    unsigned int start, count, to;

// initial values
    if( !gui_bineditor_marker_get_range(be, GUI_BINEDITOR_MARKER_SELECTED, &start, &to) ){
	start = 0;
	to = 0;
    }

    str->be = be;
    count = to - start;
    if(count <= 0) count = 1;
    if(start < 0) start = 0;

// input parameters
    tb = gtk_table_new(3, 2, TRUE);	
    gtk_box_pack_start(GTK_BOX(ctx), tb, TRUE, TRUE, 5);
    str->start = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);
    str->stop  = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);
    str->count = gtk_spin_button_new_with_range( 1, be->priv->buff->size, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->start), start);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), count);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->stop), start + count - 1);

    gtk_table_attach_defaults(GTK_TABLE(tb), str->start,  1,2, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->stop,   1,2, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->count,  1,2, 2,3);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(TXT_BE_CUT_START), 0,1, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(TXT_BE_CUT_STOP),  0,1, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(TXT_BE_CUT_COUNT), 0,1, 2,3);
// signals
    g_signal_connect(G_OBJECT(str->start), "value-changed", G_CALLBACK(gui_be_cut_start), str);
    g_signal_connect(G_OBJECT(str->stop),  "value-changed", G_CALLBACK(gui_be_cut_stop),  str);
    g_signal_connect(G_OBJECT(str->count), "value-changed", G_CALLBACK(gui_be_cut_start), str);
}

static void gui_bineditor_build_copy( GuiBineditor *be, GtkWidget *ctx, gui_be_copy_str *str )
{
    GtkWidget *wg;
    str->start = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON( str->start ), be->priv->edit_addr_cursor);
    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(ctx), wg, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(wg), gtk_label_new(TXT_BE_COPY_ADDRESS));    
    gtk_container_add(GTK_CONTAINER(wg), str->start);    
}

static void gui_be_asm_fsel(GtkWidget *w, gui_be_asm_str *str )
{
    GtkWidget *dialog;
    GtkFileFilter *filter;

    dialog = gtk_file_chooser_dialog_new(TXT_BE_TITWIN_SELECT_CORE, GTK_WINDOW(str->be->priv->wmain), 
	GTK_FILE_CHOOSER_ACTION_OPEN,
	GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	NULL
    );
    if(str->be->priv->core_name != NULL)
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), str->be->priv->core_name);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, TXT_BE_ASM_CORE);
    gtk_file_filter_add_pattern(filter, "*.brain");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT){
	char *fname;
	fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	gtk_entry_set_text(GTK_ENTRY(str->proc), fname);
	g_free( fname );
    }

    gtk_widget_destroy( dialog );
}

static void gui_bineditor_build_asm( GuiBineditor *be, GtkWidget *ctx, gui_be_asm_str *str )
{
    GtkWidget *wg;    

    str->be = be;
// widgets    
    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(ctx), wg);
    str->fsel = gtk_button_new_with_label(TXT_BE_ASM_FSEL);
    gtk_box_pack_start(GTK_BOX(wg), str->fsel, FALSE, FALSE, 0);
    str->proc = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(wg), str->proc, TRUE, TRUE, 0);
    gtk_editable_set_editable(GTK_EDITABLE(str->proc), FALSE);

    if(be->priv->core_name)
	gtk_entry_set_text(GTK_ENTRY(str->proc), be->priv->core_name);
    
    wg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(ctx), wg);
    gtk_box_pack_start(GTK_BOX(wg), gtk_label_new(TXT_BE_ASM_COUNT), FALSE, FALSE, 0);    
    str->count = gtk_spin_button_new_with_range( 0, 255, 1);
    gtk_box_pack_start(GTK_BOX(wg), str->count, TRUE, TRUE, 0);    
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->count), be->priv->core_count);
// signals
    g_signal_connect(G_OBJECT(str->fsel), "pressed", G_CALLBACK(gui_be_asm_fsel), str);
}
/*
static void gui_bineditor_build_text( GuiBineditor *be, GtkWidget *ctx, gui_be_text_str *str )
{
    GtkWidget *tb;    
    unsigned int from = 0, to = 0;

    if(!gui_bineditor_marker_get_range(be, GUI_BINEDITOR_MARKER_SELECTED, &from, &to)) from = 0;

    tb = gtk_table_new(2, 2, TRUE);	
    gtk_box_pack_start(GTK_BOX(ctx), tb, TRUE, TRUE, 5);
    str->start  = gtk_spin_button_new_with_range( 0, be->priv->buff->size - 1, 1);
    str->len  = gtk_spin_button_new_with_range( 1, be->priv->buff->size, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->start),  from);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(str->len),  16);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->start,  1,2, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), str->len,  1,2, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(TXT_BE_TEXT_START),  0,1, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(tb), gtk_label_new(TXT_BE_TEXT_LEN),  0,1, 1,2);
}
*/
/***************************************************************************************************************************************/
static void gui_bineditor_dialog_tmpl(GuiBineditor *be, void *str, gui_bineditor_tmpl_cb build, gui_bineditor_tmpl_cb exec, const char *title)
{
    int RESPONSE;
    GtkWidget *dlg, *ctx;
    
    if(!be->priv->buff->data) return;
    dlg = gtk_dialog_new_with_buttons(title, GTK_WINDOW(be->priv->wmain), 0, 
	    GTK_STOCK_OK, GTK_RESPONSE_OK,
	    (exec != NULL) ? GTK_STOCK_CANCEL : NULL, GTK_RESPONSE_CANCEL,
	    NULL
	  );
    ctx = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    if(build != NULL){ 
	build( be, ctx, str, dlg);
	gtk_widget_show_all( ctx );            
    }
    RESPONSE = gtk_dialog_run(GTK_DIALOG(dlg));    
    if(( RESPONSE == GTK_RESPONSE_OK) && ( exec != NULL)) exec( be, ctx, str, dlg);
    gtk_widget_destroy( dlg );
}

void gui_bineditor_clear_buffer(GtkWidget *bt, GuiBineditor *be)
{
    gui_clear_str str;
    str.be = be;
    str.last_pattern = NULL;
//    str.run_again = 0;
//    do{
	gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_clear), GUI_BE_CB(gui_bineditor_clear_exec), TEXT(BE_WIN_TIT_CLEAR));
	if(str.last_pattern) free(str.last_pattern);
//    }while( str.run_again );
} 

void gui_bineditor_find_string(GtkWidget *wg, GuiBineditor *be)
{
    gui_find_str str;
    
    str.last_fstr = NULL;
    str.last_rstr = NULL;
    do{
        str.run_again = 0;
	gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_find_string), GUI_BE_CB(gui_bineditor_find_exec), TEXT(BE_WIN_TIT_FIND_AND_REPLACE));
	if( str.last_fstr ) free(str.last_fstr);
	if( str.last_rstr ) free(str.last_rstr);
    }while( str.run_again );
}

void gui_bineditor_manipulator(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_bm_str str;
    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_manipulator), GUI_BE_CB(gui_bineditor_manipulator_exec), TEXT(BE_WIN_TIT_MANIPULATOR));
}

void gui_bineditor_organizer(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_org_str str;
    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_organizer), GUI_BE_CB(gui_bineditor_organizer_exec), TEXT(BE_WIN_TIT_ORGANIZER));
}


void gui_bineditor_cut(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_cut_str str;
    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_cut), GUI_BE_CB(gui_bineditor_cut_exec), TEXT(BE_WIN_TIT_CUT));
}

void gui_bineditor_copy(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_copy_str str;
    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_copy), GUI_BE_CB(gui_bineditor_copy_exec), TEXT(BE_WIN_TIT_COPY));
}

/*
void gui_bineditor_texted(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_text_str str;
    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_text), GUI_BE_CB(gui_bineditor_text_exec), TEXT(BE_WIN_TIT_TEXT));
}
*/
void gui_bineditor_undo(GtkWidget *wg, GuiBineditor *be)
{
    gui_bineditor_buff_history(be->priv->buff, GUI_BE_UNDO);
}

void gui_bineditor_redo(GtkWidget *wg, GuiBineditor *be)
{
    gui_bineditor_buff_history(be->priv->buff, GUI_BE_REDO);
}

void gui_bineditor_aux(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_aux_str str;
    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_aux), GUI_BE_CB(gui_bineditor_aux_exec), TEXT(BE_WIN_TIT_AUX));
}

void gui_bineditor_checksum(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_sum_str str;
    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_sum), NULL, TEXT(BE_WIN_TIT_SUM));
}

void gui_bineditor_resize(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_resize_str str;
    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_resize), GUI_BE_CB(gui_bineditor_resize_exec), TEXT(BE_WIN_TIT_RESIZE));
}

static void gui_bineditor_open_(GuiBineditor *be, gui_be_open_str *str)
{
    gui_bineditor_dialog_tmpl(be, str, GUI_BE_CB(gui_bineditor_build_open), GUI_BE_CB(gui_bineditor_open_exec), TEXT(BE_WIN_TIT_OPEN));
}

static void gui_bineditor_save_(GuiBineditor *be, gui_be_save_str *str)
{
    gui_bineditor_dialog_tmpl(be, str, GUI_BE_CB(gui_bineditor_build_save), GUI_BE_CB(gui_bineditor_save_exec), TEXT(BE_WIN_TIT_SAVE));
}

void gui_bineditor_open(GtkWidget *wg, GuiBineditor *be)
{
    char tmp[256];
    gui_be_open_str str;
    GtkWidget *dialog;
    char *name, *args;

    str.fname = NULL;
    str.fh = NULL;
    dialog = gtk_file_chooser_dialog_new(TXT_BE_OPEN_WINTIT, GTK_WINDOW(be->priv->wmain), 
	GTK_FILE_CHOOSER_ACTION_OPEN,
	GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	NULL
    );

    name = NULL;
    args = NULL;
    str.l_offs = 0;
    str.l_ins = 0;
    str.l_count = 1;
    str.r_offs = 0;
    str.r_ins = 0;
    str.r_count = 1;

    store_get(&store, "BINEDITOR_OPEN_FNAME", &name);
    store_get(&store, "BINEDITOR_OPEN_ARGS", &args);
        
    if( name )
	if( *name ){
	    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), name);
	}

    if( args )
	if( *args ){
	    sscanf(args, "%u %u %u", &str.l_offs, &str.l_ins, &str.l_count);
	}

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	str.fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    gtk_widget_destroy( dialog );

    if( str.fname ){ 
	gui_bineditor_open_(be, &str);
	if( name ){
	    if( strcmp(name, str.fname) )
	        store_set(&store, "BINEDITOR_OPEN_FNAME", str.fname);
	} else
	     store_set(&store, "BINEDITOR_OPEN_FNAME", str.fname);
	if( ( str.l_offs != str.r_offs) || ( str.l_ins != str.r_ins) || ( str.l_count != str.r_count) ){
	     *tmp = 0;
	     sprintf( tmp, "%u %u %u", str.r_offs, str.r_ins, str.r_count);
	     store_set(&store, "BINEDITOR_OPEN_ARGS", tmp);
	}
	g_free( str.fname );
    }
    if(str.fh) fclose(str.fh);
    if( name ) free( name );
    if( args ) free( args );
    gui_bineditor_bitmap_set_address(be, 0);
}

void gui_bineditor_write(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_save_str str;
    GtkWidget *dialog;
    char tmp[256];
    char *name, *args;

    name = NULL;
    str.fname = NULL;
    str.l_ins = 0;
    str.l_count = 1;
    args = NULL;
    dialog = gtk_file_chooser_dialog_new(TXT_BE_SAVE_WINTIT, GTK_WINDOW(be->priv->wmain), 
	GTK_FILE_CHOOSER_ACTION_SAVE,
	GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
	NULL
    );

    store_get(&store, "BINEDITOR_SAVE_FNAME", &name);
    store_get(&store, "BINEDITOR_SAVE_ARGS", &args);
    if( name )
	if( *name ){
	    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), name);
	}

    if( args )
	if( *args ){
	    sscanf(args, "%u %u", &str.l_ins, &str.l_count);
	}
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	str.fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    gtk_widget_destroy( dialog );
    if( str.fname != NULL ){ 
	gui_bineditor_save_(be, &str);
	if( name ){
	    if( strcmp(name, str.fname) )
	        store_set(&store, "BINEDITOR_SAVE_FNAME", str.fname);
	} else
	     store_set(&store, "BINEDITOR_SAVE_FNAME", str.fname);

	if( ( str.l_ins != str.r_ins) || ( str.l_count != str.r_count) ){
	     *tmp = 0;
	     sprintf( tmp, "%u %u", str.r_ins, str.r_count);
	     store_set(&store, "BINEDITOR_SAVE_ARGS", tmp);
	}

	g_free( str.fname );
    }
    if( name ) free( name );
    if( args ) free( args );
}

void gui_bineditor_bined(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_bmp_str str;
    char *args;
    char tmp[256];

    str.l_width = 8;
    str.l_height = 8;
    str.l_mask = 0xff;
    str.l_rev = 0;

    args = NULL;
    store_get(&store, "BINEDITOR_BMP_ARGS", &args);
    if( args )
	if( *args ){
	    sscanf(args, "%u %u %u %u", &str.l_width, &str.l_height, &str.l_mask, &str.l_rev);
	}
    
    str.r_width = str.l_width;
    str.r_height = str.l_height;
    str.r_mask = str.l_mask;
    str.r_rev = str.l_rev;

    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_bined), GUI_BE_CB(gui_bineditor_bined_exec), TEXT(BE_WIN_TIT_BMPEDIT));

    if( ( str.l_width != str.r_width) || ( str.l_height != str.r_height) || ( str.l_mask != str.r_mask) || ( str.l_rev != str.r_rev) ){
	     *tmp = 0;
	     sprintf( tmp, "%u %u %u %u", str.r_width, str.r_height, str.r_mask, str.r_rev);
	     store_set(&store, "BINEDITOR_BMP_ARGS", tmp);
    }

    if( args ) free( args );
}

void gui_bineditor_asmview(GtkWidget *wg, GuiBineditor *be)
{
    gui_be_asm_str str;
    gui_bineditor_dialog_tmpl(be, &str, GUI_BE_CB(gui_bineditor_build_asm), GUI_BE_CB(gui_bineditor_asm_exec), TEXT(BE_WIN_TIT_ASMVIEWER));
}

static void gui_bineditor_stencil_tree_append(gui_bineditor_stencil_str *s, const char *txt, const char *file, const char *desc, GtkTreeIter *ins, GtkTreeIter *top)
{
    gtk_tree_store_append(s->model, ins, top);
    gtk_tree_store_set(s->model, ins, 
	TREE_COL_ICON, (txt[0] == '$') ? GTK_STOCK_INDEX : GTK_STOCK_DIRECTORY, 
	TREE_COL_TEXT, (txt[0] == '$') ? txt + 1 : txt, 
	TREE_COL_FILE, file, //(txt[0] == '$') ? file : "",
	TREE_COL_DESC, (txt[0] == '$') ? desc : "",
	TREE_COL_ID, (txt[0] == '$') ?  1 : 0, 
	-1
    );
}

static gboolean gui_bineditor_stencil_button_ev(GtkWidget *wg, GdkEventButton *ev, gui_bineditor_stencil_str *s)
{
    GtkWidget *menu;
    int id;
    GtkTreeModel *model = GTK_TREE_MODEL(s->model);
    GtkTreeIter  iter;
    GtkTreePath *path;
    char *tmp, *node, *z, *fff;
    int x;
    
    if(ev->type != GDK_BUTTON_PRESS) return FALSE;
    if(ev->button == 3){
	if(s->x_path ) free(s->x_path);
	s->x_path = NULL;
	if(s->x_name) free(s->x_name);
	s->x_name = NULL;
	if(s->x_fname) free(s->x_fname);
	s->x_fname = NULL;
	gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(wg), ev->x, ev->y, &path, NULL, NULL, NULL);
	if( path == NULL ) return FALSE;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 
	    TREE_COL_TEXT, &tmp, 
	    TREE_COL_ID, &id, 
	-1
	);	
	s->x_id = id;
	if( id == 1 ){
	    for( x = gtk_tree_path_get_depth(path) - 1; x > 1; x--){
		gtk_tree_path_up( path );
		gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_model_get(model, &iter,  
		    TREE_COL_TEXT, &node,	
		    TREE_COL_FILE, &fff,	
		    -1 
		);	
		z = (char *)malloc( strlen(node) + (s->x_path ? strlen(s->x_path) : 0) + 2);
		if(s->x_path)
		    sprintf(z,"/%s%s", node, s->x_path);
		else
		    sprintf(z, "/%s", node);
		if(s->x_path) free(s->x_path);
		s->x_path = z;
		free( node );
		s->x_fname = (char *)malloc( strlen(fff) + 1);
		strcpy( s->x_fname, fff);
		free( fff );
	    }
	    s->x_name = tmp;
	}
	gtk_tree_path_free( path );
	s->has_child = 0;
	menu = gtk_menu_new();
	gui_bineditor_stencil_tree_popup_menu(s, GTK_MENU_SHELL(menu), tmp, id);
	gtk_widget_show_all( menu );
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, (ev != NULL) ? ev->button : 0, gdk_event_get_time((GdkEvent *)ev) );	
	return TRUE;
    }
    return FALSE;
}

static void gui_bineditor_stencil_select_ev(GtkTreeSelection *sel, gui_bineditor_stencil_str *s)
{
    GtkTreeModel *model;
    GtkTreeIter  iter;
    char 	 *tmp;
    int 	 id;

    if( !gtk_tree_selection_get_selected( sel, &model, &iter)) return;
    gtk_tree_model_get(model, &iter, 
	TREE_COL_DESC, &tmp, 
	TREE_COL_ID, &id, 
	-1
    );	
    gui_bineditor_stencil_select_brief( s, tmp,  id);
    g_free( tmp );
}

static void gui_bineditor_stencil_row_activated_ev(GtkTreeView *view, GtkTreePath *pth, GtkTreeViewColumn *col, gui_bineditor_stencil_str *s)
{
    GtkTreeModel *model;
    GtkTreeIter  iter;
    char	 *tmp, *ff;
    int		 id;
    
    model = gtk_tree_view_get_model( view );
    if(!gtk_tree_model_get_iter(model, &iter, pth)) return;
    
    gtk_tree_model_get(model, &iter, 
	TREE_COL_TEXT, &tmp, 
	TREE_COL_FILE, &ff, 
	TREE_COL_ID, &id, 
	-1
    );	
    gui_bineditor_stencil_selected( s, tmp, ff, id);
    g_free( tmp );
    g_free( ff );
}

static inline void gui_bineditor_stencil_tree(GuiBineditor *be, gui_bineditor_stencil_str *s)
{
    GtkTreeIter toplevel;
    GtkCellRenderer *renderer;
    GtkTreeSelection *sel;
    
    s->be = be;
    s->x_path = NULL;
    s->x_name = NULL;    
    s->x_fname = NULL;
    s->view = gtk_tree_view_new();
    
    g_signal_connect(G_OBJECT(s->view), "button-press-event", G_CALLBACK(gui_bineditor_stencil_button_ev), s);
    g_signal_connect(G_OBJECT(s->view), "popup-menu", G_CALLBACK(gui_bineditor_stencil_button_ev), s);
    g_signal_connect(G_OBJECT(s->view), "row-activated", G_CALLBACK(gui_bineditor_stencil_row_activated_ev), s);    
    gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(s->view), TRUE);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(s->view), FALSE);

    // column tree setup
    s->col = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_pixbuf_new();    
    gtk_tree_view_column_pack_start(s->col, renderer, FALSE);
    gtk_tree_view_column_add_attribute(s->col, renderer, "stock-id", TREE_COL_ICON);
    s->renderer = gtk_cell_renderer_text_new();    
    gtk_tree_view_column_pack_start(s->col, s->renderer, FALSE);
    gtk_tree_view_column_add_attribute(s->col, s->renderer, "text", TREE_COL_TEXT);
    gtk_tree_view_append_column(GTK_TREE_VIEW(s->view), s->col);

    // content    
    s->model = gtk_tree_store_new(TREE_COL_ALL, G_TYPE_STRING,  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);

    gui_bineditor_stencil_tree_append( s, "Stencils", NULL, NULL, &toplevel, NULL); // !! to change

    gtk_tree_view_set_model(GTK_TREE_VIEW(s->view), GTK_TREE_MODEL(s->model));
    gtk_tree_selection_set_mode( gtk_tree_view_get_selection(GTK_TREE_VIEW(s->view)), GTK_SELECTION_NONE);    
    s->top = &toplevel;
    gui_bineditor_stencil_build_tree( s, "./stencils/stencil.idx" ); // path should be from config !!
    gtk_tree_view_expand_row(GTK_TREE_VIEW(s->view), gtk_tree_path_new_from_indices(0, -1), FALSE);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(s->view));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(gui_bineditor_stencil_select_ev), s);
}

void gui_bineditor_stencil(GtkWidget *wg, GuiBineditor *be)
{
    gint w=0, h=0;
    gui_bineditor_stencil_str str;
    GtkWidget *dlg, *hb, *sw;

    str.x_name = NULL;
    str.x_path = NULL;
    str.x_fname = NULL;

    be->priv->stencil = NULL;
    dlg = gtk_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(dlg), GTK_WINDOW(be->priv->wmain));
    gtk_window_set_title(GTK_WINDOW(dlg), TEXT(WINTITLE_STENCIL));
    gtk_widget_set_size_request(dlg, 320, 200);

    hb = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg))), hb, TRUE, TRUE, 0);
    str.sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_paned_add1(GTK_PANED(hb), str.sw);
    be->priv->stencil = &str;    
    gui_bineditor_stencil_tree(be, &str);
    gtk_container_add( GTK_CONTAINER(str.sw), str.view);
    // set paned divider position
    gtk_widget_get_size_request(dlg, &w,&h);
    gtk_paned_set_position(GTK_PANED(hb), w / 2);

    sw = gtk_frame_new(TEXT(BE_STENCIL_BRIEF));
    gtk_paned_add2(GTK_PANED(hb), sw);
    str.brief = gtk_label_new(NULL);
    gtk_misc_set_alignment(GTK_MISC(str.brief), 0.0, 0.0);
    gtk_container_add( GTK_CONTAINER(sw), str.brief);

    gtk_widget_show_all(dlg);        
    gtk_dialog_run(GTK_DIALOG(dlg));
    g_object_unref(str.model);
    gtk_widget_destroy( dlg );    
    if(str.x_path ) free(str.x_path);
    if(str.x_name) free(str.x_name);
    if(str.x_fname) free(str.x_fname);
}

void gui_bineditor_stencil_update(GuiBineditor *be)
{
    gui_bineditor_stencil_str *str = (gui_bineditor_stencil_str *)be->priv->stencil;
    if( !str ) return;
    if( !str->sw ) return;                                                         
    gtk_widget_destroy( str->view );    

//    if(s->x_path ) free(s->x_path);
//    if(s->x_name) free(s->x_name);
//    s->x_name = NULL;
//    s->x_path = NULL;

    gui_bineditor_stencil_tree(be, str);
    gtk_container_add( GTK_CONTAINER(str->sw), str->view);
    gtk_widget_show_all( str->view);
}

char gui_bineditor_stencil_split(char **s1, char *tmp)
{
    char *s0;
    int i;
    
    s0 = *s1;
    if(tmp == NULL) return 0;
    tmp[0] = 0;
    if((s0 == NULL)||(s1 == NULL)) return 0;
    if( *s0 == 0 ) return 0;    
    if(*s0 == '/' ) s0++; // skip single '/'
    if(*s0 == '/' ) return 0; // protection from empty row '..//..'
    
    *s1 = strchr(s0, '/');
    if(*s1 == NULL) *s1 = strchr(s0, 0);

    if( *s0 == '/') s0++;
    for(i = 0; (i < 256) && (s0 != *s1); i++, s0++) tmp[i] = *s0;
    tmp[i] = 0;

    return (*s1 != NULL);    
}

static void gui_bineditor_stencil_split_rc(gui_bineditor_stencil_str *s, char *file, char *path, char *desc, char *buff, GtkTreeIter *parent)
{
    char *tmp, create = 1;
    GtkTreeIter branch, first, *node;

    if(!gui_bineditor_stencil_split(&path, buff)) return;
    if(gtk_tree_model_iter_has_child( GTK_TREE_MODEL(s->model), parent)){
	if( !gtk_tree_model_iter_children(GTK_TREE_MODEL(s->model), &first, parent) ){
	    printf("ERROR:gui_bineditor_stencil_split_rc() --> iterator not valid\n");
	    return;
	}
	node = &first;
	do{
	    tmp = NULL;
	    gtk_tree_model_get(GTK_TREE_MODEL(s->model), node, TREE_COL_TEXT, &tmp, -1);
	    if( tmp ){
		if(!strcmp( tmp, buff)){
		    g_free( tmp );
		    create = 0;
		    gui_bineditor_stencil_split_rc(s, file, path, desc, buff, node);    
		    return;
		}
		g_free( tmp );
	    }

	} while( gtk_tree_model_iter_next(GTK_TREE_MODEL(s->model), node) );
    } 

    if( create ) gui_bineditor_stencil_tree_append(s, buff, file, desc, &branch, parent);
    gui_bineditor_stencil_split_rc(s, file, path, desc, buff, &branch);
}

static void gui_bineditor_stencil_add_item(gui_bineditor_stencil_str *s, const char *file, const char *path, const char *desc)
{
    char tmp[256];
    GtkTreeIter parent;
    
    memcpy( &parent, s->top, sizeof(GtkTreeIter));    
    gui_bineditor_stencil_split_rc(s, (char *)file, (char *)path, (char *)desc, tmp, &parent);
}
static inline char *gui_bineditor_stencil_index_file_line(gui_bineditor_stencil_str *s, int *length)
{
    int len = 0;
    int pos;
    char *tmp, cmt, a;

    // determine length and first position of valid line
    do{
	pos = ftell(s->idx_file); // store file position
	cmt = 0; len = 0;
	// determine line length
	while(!feof(s->idx_file)){
	    a = fgetc(s->idx_file);
	    if( ( len == 0) && (a == '#' ) ) cmt = 1;
	    len++;
	    if( a == '\n' ) break;
	}
    } while( cmt ); // skip commentary lines
    if( len <= 3 ) return NULL; // if end of string or invalid string
    fseek(s->idx_file, pos, SEEK_SET); // restore file position

    tmp = (char *)malloc( len );
    if( tmp == NULL ){
	printf("ERROR: gui_bineditor_stencil_index_file_line() mem alloc problem\n");
	return NULL;
    }

    if( fread( tmp, 1, len, s->idx_file) != len ){
	printf("ERROR: gui_bineditor_stencil_index_file_line() read file problem\n");
	return NULL;
    }

    if(tmp[ len - 1] == '\n') len--;
    tmp[len] = 0;
    *length = len;
    return tmp;
}

static char gui_bineditor_stencil_get_index_file(gui_bineditor_stencil_str *s, char **file, char **path, char **desc)
{
    int length, i;
    char *line, *pt, *pr;    

    *file = NULL; *path = NULL; *desc = NULL;
    line = gui_bineditor_stencil_index_file_line(s, &length);
    if(line == NULL) return 0;
    
    *file = (char *)malloc( length );
    *path = (char *)malloc( length );    
    *desc = (char *)malloc( length );
    
    (*file)[0] = 0; (*path)[0] = 0; (*desc)[0] = 0;
    pt = strchr(line, ':');
    if(!pt){
	printf("index file syntax error\n");
	return 0;
    }
    for( pr = line, i = 0; pr != pt; pr++, i++ ) 
	(*file)[i] = *pr;
	(*file)[i] = 0;
    if(*pr == ':') pr++;

    pt = strchr(pr, ':');
    if(!pt){
	printf("index file syntax error\n");
	return 0;
    }
    for( i = 0; pr != pt; pr++, i++ ) 
	(*path)[i] = *pr;
	(*path)[i] = 0;
    if(*pr == ':') pr++;

    pr = strchr(pr, '"');
    if( pr != NULL ){
	if(pr[0] == '"') pr++;
	for( i = 0; *pr && (*pr != '"'); pr++, i++ ) 
	    (*desc)[i] = *pr;
	    (*desc)[i] = 0;
    }    
    free(line);    
    return 1;
}

static inline void gui_bineditor_stencil_build_tree(gui_bineditor_stencil_str *s, const char *idx_fpath)
{
    char *file = NULL, *path = NULL, *desc = NULL;

    s->idx_file = NULL;
    if( access(idx_fpath, R_OK) != 0){
	if(!gui_bineditor_stencil_generate_index_file(s->be, idx_fpath )){
	    printf("ERROR:error creating index stencil file\n");
	    return;
	}
    }
    if(!(s->idx_file = fopen(idx_fpath, "r"))){	// try to open index file
	printf("ERROR:error open index stencil file\n");
	return;
    }
    fseek( s->idx_file, 0L, SEEK_SET);
    while( gui_bineditor_stencil_get_index_file(s, &file, &path, &desc) ){
	gui_bineditor_stencil_add_item(s, file, path, desc);
	if( path ) free( path );
	if( file ) free( file );
	if( desc ) free( desc );
    }
    if(!s->idx_file) return;
    fclose(s->idx_file);
}

static inline void gui_bineditor_stencil_select_brief(gui_bineditor_stencil_str *s, const char *name, int id)
{
    gtk_label_set_text(GTK_LABEL(s->brief), (id == 1) ? name : NULL);
}

static inline void gui_bineditor_stencil_selected(gui_bineditor_stencil_str *s, const char *name, const char *file, int id)
{
    if( id == 0 ) return;
    gui_bineditor_stencil_sheet(s->be, name, file );
}

static void gui_bineditor_stencil_tree_popup_menu_new_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
{
    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_NEW);
}

static void gui_bineditor_stencil_tree_popup_menu_add_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
{
    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_ADD);
}
/*
static void gui_bineditor_stencil_tree_popup_menu_copy_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
{
    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_COPY);
}

static void gui_bineditor_stencil_tree_popup_menu_paste_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
{
    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_PASTE);
}
*/
static void gui_bineditor_stencil_tree_popup_menu_edit_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
{
    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_EDIT); 
}
/*
static void gui_bineditor_stencil_tree_popup_menu_rename_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
{
    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_RENAME);
}
*/
static void gui_bineditor_stencil_tree_popup_menu_remove_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
{
    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_REMOVE);
}

//static void gui_bineditor_stencil_tree_popup_menu_delete_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
//{
//    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_DELETE);
//}

static void gui_bineditor_stencil_tree_popup_menu_update_all_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
{
    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_UPDATE_ALL);
}

static void gui_bineditor_stencil_tree_popup_menu_cancel_ev(GtkWidget *m, gui_bineditor_stencil_str *s)
{
printf("RRT\n");
//    gui_bineditor_stencil_tree_operation(s, GUI_BE_OPERATION_DELETE);
}

static inline void gui_bineditor_stencil_tree_popup_menu(gui_bineditor_stencil_str *s, GtkMenuShell *ms, char *name, int id)
{
    GtkWidget *mi;
    mi = gtk_menu_item_new_with_label("New");
    gtk_menu_shell_append(ms, mi);    
    g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_new_ev), s);
    mi = gtk_menu_item_new_with_label("Add");
    gtk_menu_shell_append(ms, mi);    
    g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_add_ev), s);
    if( id == 1){
//	mi = gtk_menu_item_new_with_label("Copy");
//	gtk_menu_shell_append(ms, mi);    
//	g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_copy_ev), s);
//	mi = gtk_menu_item_new_with_label("Paste");
//	gtk_menu_shell_append(ms, mi);    
//	g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_paste_ev), s);
	mi = gtk_menu_item_new_with_label("Edit");
	gtk_menu_shell_append(ms, mi);    
	g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_edit_ev), s);

	mi = gtk_menu_item_new_with_label("Delete");
	gtk_menu_shell_append(ms, mi);    
	g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_remove_ev), s);

    }
//    mi = gtk_menu_item_new_with_label("Rename");
//    gtk_menu_shell_append(ms, mi);    
//    g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_rename_ev), s);

    g_signal_connect(G_OBJECT(ms), "deactivate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_cancel_ev), s);

//    if(id == 1){
//	mi = gtk_menu_item_new_with_label("Delete");
//	gtk_menu_shell_append(ms, mi);    
//	g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_delete_ev), s);
//    }
    mi = gtk_menu_item_new_with_label("Update all");
    gtk_menu_shell_append(ms, mi);    
    g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(gui_bineditor_stencil_tree_popup_menu_update_all_ev), s);

}

static void gui_bineditor_stencil_tree_operation(gui_bineditor_stencil_str *s, int operation)
{
    if(gui_bineditor_stencil_operation(s->be, s->x_fname, s->x_name, s->x_path, operation)){
	gtk_widget_destroy(s->view);
	gui_bineditor_stencil_build_tree( s, "./stencils/stencil.idx" ); // path should be from config !!
    }
}

