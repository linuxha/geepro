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

#include "be_icons_xpm.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <gdk/gdkkeysyms.h>
#include "bineditor.h"
#include "../src/script.h"
#include "../intl/lang.h"
#include "be_panel.h"
#include "be_buffer.h"
#include "be_bitmap.h"

#define GUI_BINEDITOR_BITMAN_ICON	"geepro-bineditor-bitman"
#define GUI_BINEDITOR_BYTMAN_ICON	"geepro-bineditor-bytman"
#define GUI_BINEDITOR_CRC_ICON		"geepro-bineditor-crc"
#define GUI_BINEDITOR_BMP_ICON		"geepro-bineditor-bmp"
#define GUI_BINEDITOR_TEXT_ICON		"geepro-bineditor-text"
#define GUI_BINEDITOR_STENCIL_ICON	"geepro-bineditor-stencil"
#define GUI_BINEDITOR_ASM_ICON		"geepro-bineditor-asm"
#define GUI_BINEDITOR_EXPAND_ICON	"geepro-bineditor-expand"
#define GUI_BINEDITOR_AUX_ICON		"geepro-bineditor-aux"


enum
{
    CHANGED,
    LAST_SIGNAL
};

static guint bineditor_signals[LAST_SIGNAL] = {0};
static void gui_bineditor_class_init(GuiBineditorClass *cl);
static void gui_bineditor_init(GuiBineditor *be);
static void gui_beneditor_destroy(GObject *obj);
static void gui_bineditor_draw( GuiBineditor *be, cairo_t *cr, int vxx, int vyy, char print);

GType gui_bineditor_get_type(void)
{
    static GType bineditor_type = 0;
    if(!bineditor_type){
	static const GTypeInfo bineditor_info =
	{
	    sizeof(GuiBineditorClass),
	    NULL, // base init
	    NULL, // base finalize
	    (GClassInitFunc) gui_bineditor_class_init,
	    NULL, // class finalize
	    NULL, // class data
	    sizeof(GuiBineditor),
	    0,
	    (GInstanceInitFunc)gui_bineditor_init
	};
	bineditor_type = g_type_register_static(GTK_TYPE_BOX, "GuiBineditor", &bineditor_info, (GTypeFlags)0);
    }

    return bineditor_type;
}

static void gui_bineditor_class_init(GuiBineditorClass *cl)
{
    GObjectClass *goc = (GObjectClass*)cl;

    bineditor_signals[CHANGED] = g_signal_new(
	"changed", 
	G_TYPE_FROM_CLASS(cl), 
	G_SIGNAL_ACTION | G_SIGNAL_ACTION,
	G_STRUCT_OFFSET(GuiBineditorClass, bineditor),
	NULL, NULL,
	g_cclosure_marshal_VOID__VOID,
	G_TYPE_NONE, 0
    );
    goc->finalize = gui_beneditor_destroy;

    g_type_class_add_private(goc, sizeof(GuiBineditorPrivate) );
}

static void gui_beneditor_destroy(GObject *obj)
{
    GuiBineditor *be;

    g_return_if_fail(obj != NULL);
    g_return_if_fail(GUI_IS_BINEDITOR(obj));

    be = GUI_BINEDITOR(obj);    
    if(be->priv->vfind) free(be->priv->vfind);
    be->priv->vfind = NULL;
    if(be->priv->vreplace) free(be->priv->vreplace);
    be->priv->vreplace = NULL;
    if(be->priv->core_name) free(be->priv->core_name);
    gui_bineditor_buff_destroy( &be->priv->buff );
    gui_bineditor_marker_free(be);
    if( be->priv->cut_data.data ) free(be->priv->cut_data.data);
}

static unsigned int gui_bineditor_get_grid_start_address( GuiBineditor *be )
{
    return (gtk_adjustment_get_value(be->priv->adj) / be->priv->grid_cols) * be->priv->grid_cols;
}

char gui_bineditor_selection_get_range(GuiBineditor *be, unsigned int *from, unsigned int *to)
{
    unsigned int a, b, c;
    
    a = be->priv->clpb_start;
    b = be->priv->clpb_end;
    if( b < a ) {
        c = a;
        a = b;
        b = c;
    }	

    *from = a;
    *to = b;
    return be->priv->clpb_selected;
}

GtkWidget *gui_bineditor_new(GtkWindow *wmain)
{
    GtkWidget *wg;

    g_return_val_if_fail(wmain != NULL, NULL);
    g_return_val_if_fail(GTK_IS_WINDOW(wmain), NULL);

    wg = GTK_WIDGET(g_object_new(GUI_TYPE_BINEDITOR, NULL));
    ((GuiBineditor*)wg)->priv->wmain = (GtkWidget*)wmain;
    return wg;
}

/******************************************************************************************************************/
static void gui_bineditor_jump_marker(GtkWidget *wg, GuiBineditor *be)
{
    be->priv->address_mark_redo = gtk_adjustment_get_value(be->priv->adj);
    gtk_adjustment_set_value(be->priv->adj, be->priv->address_mark);
    gtk_widget_set_sensitive(be->priv->rjmp, TRUE);
    gtk_adjustment_value_changed(be->priv->adj);    
}

static void gui_bineditor_redo_marker(GtkWidget *wg, GuiBineditor *be)
{
    gtk_adjustment_set_value(be->priv->adj, be->priv->address_mark_redo);
    gtk_widget_set_sensitive(be->priv->rjmp, FALSE);
    gtk_adjustment_value_changed(be->priv->adj);    
}

/**************************************************************************************************************************/

#define SET_COLOR(be, idx, r,g,b)	be->priv->colors[idx * 3] = r; be->priv->colors[idx * 3 + 1] = g; be->priv->colors[idx * 3 + 2] = b
#define GET_COLOR(be, idx)		be->priv->colors[idx * 3], be->priv->colors[idx * 3 + 1], be->priv->colors[idx * 3 + 2]

void gui_bineditor_statusbar(GuiBineditor *be, char *tmp, char *str, ...)
{	
    va_list v;
        
    va_start(v, str);
    vsprintf(tmp, str, v);
    va_end(v);

    if(!be->priv->statusbar) return;
    gtk_statusbar_pop(GTK_STATUSBAR(be->priv->statusbar), be->priv->statusbar_id);
    gtk_statusbar_push(GTK_STATUSBAR(be->priv->statusbar), be->priv->statusbar_id, tmp);
}

static char gui_dig2hex(char i){
    return i + (i < 10 ? '0'  : 'A' - 10 );
}

static int gui_bineditor_get_grid_addr(GuiBineditor *be, int xi, int yi, char *ascii_grid)
{
    int x, y, xa, address, start_addr;

    x  = xi - be->priv->grid_start;  // horizontal position relative to start of hex grid
    xa = xi - be->priv->ascii_start; // horizontal position relative to start of ascii grid  
    y  = yi - be->priv->grid_top;    // vertical position common for hex and ascii grid
    *ascii_grid = 0;
    start_addr =  gui_bineditor_get_grid_start_address( be );

    // Compute grid cell index
    // if cursor is over hex grid or is away grid fields
    if(x < 0 || y < 0 || xi > be->priv->grid_end || !be->priv->cell_width || !be->priv->cell_height){
	if(xa < 0 || y < 0 || xi > be->priv->ascii_end || !be->priv->cell_width || !be->priv->cell_height) return -1;
	x = xa / be->priv->ascii_space;
	*ascii_grid = 1;
    } else {
	x /= be->priv->cell_width;
    }
    y /= be->priv->cell_height + 1; 

    address = y * be->priv->grid_cols + x + start_addr;

    return address;
}

static void gui_bineditor_exit_edit(GuiBineditor *be)
{
    be->priv->edit_hex = 0;
    gtk_widget_set_sensitive(be->priv->tbv, 1);
    gtk_widget_set_sensitive(be->priv->tb, 1);
}

static void gui_bineditor_off_edit(GuiBineditor *be)
{
    gtk_widget_set_sensitive(be->priv->tbv, 0);
    gtk_widget_set_sensitive(be->priv->tb, 0);
}

static void gui_bineditor_mbutton(GtkWidget *wg, GdkEventButton *ev, GuiBineditor *be)
{
    int address;
    char ascii = 0;


    if( gtk_widget_is_focus(wg) == FALSE ){
	gtk_widget_grab_focus( wg );
    }

    if(!be->priv->buff->data || !be->priv->buff->size) return;
    address = gui_bineditor_get_grid_addr(be, ev->x, ev->y, &ascii);
    if(address < 0) return;
    if( !gui_bineditor_bitmap_get_mode(be) ) gui_bineditor_bitmap_set_address( be, address );        
    if( be->priv->bmp ) return; // bitmap mode
    if(ev->button == 2){
	be->priv->address_mark = address;
        gtk_widget_queue_draw(be->priv->drawing_area);
	return;
    }

    if( ev->button == 3){
	if(be->priv->edit_hex == 0){
	    be->priv->edit_hex = ascii ? 2 : 1;
	    be->priv->edit_hex_cursor = 0;
	    be->priv->edit_addr_cursor = address;
	    gui_bineditor_off_edit( be );
	} else {
	    gui_bineditor_exit_edit( be );
	}
	gtk_widget_queue_draw(be->priv->drawing_area);
    }
// copy to clipboard marking
    if(ev->button == 1){
	gui_bineditor_marker_unset_item(be, GUI_BINEDITOR_MARKER_FOUND);
	if(be->priv->edit_hex == 1){ // cancel editing mode
	    gui_bineditor_exit_edit( be );
	}
	if(be->priv->clpb == 0){
	    be->priv->clpb_ascii = ( ev->x > be->priv->ascii_start );
	    be->priv->clpb = 1;
	    be->priv->clpb_start = address;
	    be->priv->clpb_end = address;
	    be->priv->clpb_selected = TRUE;
	    gui_bineditor_marker_set_item(be, GUI_BINEDITOR_MARKER_SELECTED,  
		be->priv->clpb_ascii ? GUI_BINEDITOR_MARKER_ASCII : GUI_BINEDITOR_MARKER_HEX, address, address);
	} else {
	    gui_bineditor_marker_unset_item(be, GUI_BINEDITOR_MARKER_SELECTED);
	    be->priv->clpb = 0;
	    be->priv->clpb_start = 0;
	    be->priv->clpb_end = 0;
	    be->priv->clpb_selected = FALSE;
	}
	gtk_widget_queue_draw(be->priv->drawing_area);
    }
}

static void gui_bineditor_mbutton_free(GtkWidget *wg, GdkEventButton *ev, GuiBineditor *be)
{
    int address;
    char ascii = 0;

    if(!be->priv->buff->data || !be->priv->buff->size) return;
    address = gui_bineditor_get_grid_addr(be, ev->x, ev->y, &ascii);
    if(address < 0) return;

// copy to clipboard marking
    if(ev->button == 1){
	if(be->priv->clpb == 1){
	    be->priv->clpb = 2;
	    be->priv->clpb_end = address;
	    gui_bineditor_marker_set_value(be, GUI_BINEDITOR_MARKER_SELECTED, be->priv->clpb_start, be->priv->clpb_end);
	}
	gtk_widget_queue_draw(be->priv->drawing_area);
    }
}

static void gui_bineditor_hint(GtkWidget *wg, GdkEventMotion *ev, GuiBineditor *be)
{
    char tmp[256];
    int address;
    int data, offs;
    char ascii = 0;

    if(!be->priv->buff->data || !be->priv->buff->size) return;
    address = gui_bineditor_get_grid_addr(be, ev->x, ev->y, &ascii);

    if(address < 0){
	gui_bineditor_statusbar(be, tmp, "");
        return;
    }

    if(be->priv->clpb == 1){
	be->priv->clpb_end = address;
	gui_bineditor_marker_set_value(be, GUI_BINEDITOR_MARKER_SELECTED, be->priv->clpb_start, be->priv->clpb_end);
	gtk_widget_queue_draw(be->priv->drawing_area);
    }

    data = be->priv->buff->data[address];
    if(be->priv->address_old_hint == address) return;
    if(address >= be->priv->buff->size){
	gui_bineditor_statusbar(be, tmp, "");
	return;
    }
    be->priv->address_old_hint = address;
    offs = address - be->priv->address_mark;

    sprintf(tmp, " %x:%x(%i) ", address, data & 0xff, data & 0xff);
    gtk_label_set_text(GTK_LABEL(be->priv->info_addr), tmp);

    sprintf(tmp, " %x:%c%x(%i) ", be->priv->address_mark, offs < 0 ? '-':' ',abs(offs), offs );
    gtk_label_set_text(GTK_LABEL(be->priv->info_mark), tmp);

    if( gui_bineditor_bitmap_get_mode(be) ) 
	    gui_bineditor_bitmap_set_address( be, address );        
}

static void gui_bineditor_leave(GtkWidget *wg, GdkEventCrossing *ev, GuiBineditor *be)
{
    char tmp[2];
    /* oczyszczenie pola statusu */
    gui_bineditor_statusbar(be, tmp, "");
    gdk_window_set_cursor(gtk_widget_get_parent_window(wg), NULL);
}

static void gui_bineditor_enter(GtkWidget *wg, GdkEventCrossing *ev, GuiBineditor *be)
{
    GdkCursor *cursor;
    cursor = gdk_cursor_new(GDK_HAND2);
    gdk_window_set_cursor(gtk_widget_get_parent_window(wg), cursor);
    g_object_unref(G_OBJECT(cursor));
}

static void gui_bineditor_slider(GtkAdjustment *wig, GuiBineditor *wg, GuiBineditor *be)
{
    gtk_widget_queue_draw(wg->priv->drawing_area);
}

static void gui_bineditor_scroll(GtkWidget *wg, GdkEventScroll *ev, GuiBineditor *be)
{
    gdouble inc = gtk_adjustment_get_step_increment( GTK_ADJUSTMENT(be->priv->adj) );
    gdouble val = gtk_adjustment_get_value( GTK_ADJUSTMENT(be->priv->adj) );

    switch( ev->direction ){
	case GDK_SCROLL_UP:   val -= inc; break; 
	case GDK_SCROLL_DOWN: val += inc; break;
	default: return;
    }
    gtk_adjustment_set_value( GTK_ADJUSTMENT(be->priv->adj), val );
}

static  void gui_cairo_line(cairo_t *cr, int x0, int y0, int x1, int y1)
{
    cairo_move_to(cr, x0, y0);
    cairo_line_to(cr, x1, y1);
}

static void gui_cairo_outtext(cairo_t *cr, char *tmp, int x, int y, char *str, ...)
{
    va_list varg;

    va_start(varg, str);
    vsprintf(tmp, str, varg);
    va_end(varg);
    
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, tmp);
}

static gboolean gui_bineditor_draw_sig(GtkWidget *wg, cairo_t *cr, GuiBineditor *be)
{
    gui_bineditor_draw(be, cr, gtk_widget_get_allocated_width( wg ), gtk_widget_get_allocated_height( wg ), 0);  
    return FALSE;
}

static void gui_bineditor_cursor_decrement(GuiBineditor *be, int v )
{
    if( be->priv->edit_addr_cursor < v){
	be->priv->edit_hex_cursor = 0;	
        return;
    }
    if(((be->priv->edit_hex_cursor -= v) < 0) || (be->priv->edit_hex == 2)){
	if(v == 1) 
	    be->priv->edit_hex_cursor = 1;
	else
	    be->priv->edit_hex_cursor &= 1;
	if( (be->priv->edit_hex == 2) || ( be->priv->edit_hex_cursor < 0) ) be->priv->edit_hex_cursor = 0;
        be->priv->edit_addr_cursor -= v;
	if(be->priv->edit_addr_cursor < 0) be->priv->edit_addr_cursor = 0;
    } 
}

static void gui_bineditor_cursor_increment(GuiBineditor *be, int v )
{
    if(((be->priv->edit_hex_cursor += v) > 1) || (be->priv->edit_hex == 2)){
	if((v == 1) || (be->priv->edit_hex == 2)) 
	    be->priv->edit_hex_cursor = 0;
	else
	    be->priv->edit_hex_cursor &= 1;
        be->priv->edit_addr_cursor += v;
	if(be->priv->edit_addr_cursor >= be->priv->buff->size) be->priv->edit_addr_cursor -= v;
    }
}

static void gui_bineditor_edit(GuiBineditor *be, int key)
{
    int val;
    unsigned char data;

    if(be->priv->edit_hex == 1){
	if((key >= 'A') && (key <= 'F')) key = key - 'A' + 'a';
	if(!(((key >= '0') && (key <= '9')) || ((key >= 'a') && (key <= 'f')))) return;
	val = key - '0';
	if((key >= 'a') && (key <= 'f')) val = 10 + key - 'a';
	val &= 0x0f;
	if(be->priv->edit_hex_cursor == 0)
	    data = (be->priv->buff->data[ be->priv->edit_addr_cursor] & 0x0f) | (val << 4);
	else
	    data = (be->priv->buff->data[ be->priv->edit_addr_cursor] & 0xf0) | val;
    } else {
	data = key;
    }
    gui_bineditor_buff_history_add(be->priv->buff, be->priv->edit_addr_cursor, be->priv->edit_addr_cursor);
    be->priv->buff->data[ be->priv->edit_addr_cursor] = data;
    gui_bineditor_cursor_increment( be, 1 );
}

static char *gui_bineditor_get_text_from_clpb( GuiBineditor *be )
{
    int length = (be->priv->clpb_end - be->priv->clpb_start) + 1;
    int slength, i, x;
    char *text = NULL;
    
    if(length < 0) return NULL;
    slength = length;
    if( !be->priv->clpb_ascii ) slength *= 5; // 0x00, <= 5 characters
    slength++; // for '\0'
    if(!(text = malloc( slength ))) return NULL;
    for( i = 0; i < length; i++){
        x = i + be->priv->clpb_start;
	if( x < be->priv->buff->size ){
	    x = be->priv->buff->data[ x ];
	    if( be->priv->clpb_ascii )
    		text[i] = x;
    	    else {
		sprintf(text + i * 5, "0x%x%x,", (x >> 4) & 0x0f, x & 0x0f);
    	    }
	}
    }
    text[slength] = 0;
    return text;
}

static void gui_bineditor_clipboard_insert(GuiBineditor *be, gchar *txt)
{
    long len, i;
        
    if(be->priv->edit_hex == 0) return; // must be in edit mode to insert
    if( (be->priv->clpb_start < 0) || (be->priv->buff->size <= 0 )) return;
    if( (be->priv->clpb_start >= be->priv->buff->size)) return;
    len = strlen(txt); // bytes count to insert
    if( (be->priv->clpb_start + len) >= be->priv->buff->size ) len = be->priv->buff->size - be->priv->clpb_start;

//    if(be->priv->edit_hex & 1) gui_bineditor_hex2ascii(be, txt);

    // ascii field insert
    if(be->priv->edit_hex & 2){
        gui_bineditor_buff_history_add(be->priv->buff, be->priv->clpb_start, be->priv->clpb_start + len - 1);
	for(i = 0; i < len; i++)
    	    be->priv->buff->data[be->priv->clpb_start + i] = txt[ i ];
	gtk_widget_queue_draw(be->priv->drawing_area);
	gui_bineditor_exit_edit( be );
    }
}

static void gui_bineditor_clipboard(GuiBineditor *be, gint c)
{
    gchar *text = NULL;
    
    switch( c ){
	case 'C' :
        case 'c' : text = gui_bineditor_get_text_from_clpb( be );
		   gtk_clipboard_set_text( be->priv->clipb, text, -1 );
    		   break;
	case 'V' :
	case 'v' : if( gtk_clipboard_wait_is_text_available( be->priv->clipb ) ){
	             text = gtk_clipboard_wait_for_text( be->priv->clipb );
	             gui_bineditor_clipboard_insert( be, text );
		   };
		   break;
	case 'X' :
	case 'x' :;
    };

    if( text ) g_free( text );
}


static void gui_bineditor_keystroke(GtkWidget *wg, GdkEventKey *ev, GuiBineditor *be)
{
    int key = ev->keyval;
    int adj;

    if( key == GDK_KEY_Control_L ){
	be->priv->key_ctrl = ev->type == GDK_KEY_PRESS;
	return;	
    }

    if( be->priv->key_ctrl ){
	gui_bineditor_clipboard(be, key);
	return;
    }

    if( ev->type == GDK_KEY_RELEASE ) return;
    if( be->priv->edit_hex == 0 ) return; // ignore if edit mode not active
    if(key == be->priv->key_left) gui_bineditor_cursor_decrement( be, 1 ); 
    if(key == be->priv->key_right) gui_bineditor_cursor_increment( be, 1 ); 
    if(key == be->priv->key_up) gui_bineditor_cursor_decrement( be, be->priv->grid_cols ); 
    if(key == be->priv->key_down) gui_bineditor_cursor_increment( be, be->priv->grid_cols ); 
    if(key == be->priv->key_home){
//	be->priv->edit_hex_cursor = 0;
//	be->priv->edit_addr_cursor = (be->priv->edit_addr_cursor / be->priv->grid_cols) * be->priv->grid_cols;
    }
    if(key == be->priv->key_end){
//        be->priv->edit_hex_cursor = 0;
//        be->priv->edit_addr_cursor = ((be->priv->edit_addr_cursor / be->priv->grid_cols) + 1) * be->priv->grid_cols - 1;
//        if(be->priv->edit_addr_cursor >= be->priv->buff->size) be->priv->edit_addr_cursor = be->priv->buff->size - 1;
//        be->priv->edit_addr_cursor = be->priv->address_hl_start;
    }
    if(key == be->priv->key_pgup){	
//	be->priv->edit_hex_cursor = 0;
//	be->priv->edit_addr_cursor -= be->priv->grid_cols * be->priv->grid_rows;
//	if(be->priv->edit_addr_cursor < 0) be->priv->edit_addr_cursor = 0;
//	be->priv->edit_addr_cursor = be->priv->edit_addr_cursor = (be->priv->edit_addr_cursor / be->priv->grid_cols) * be->priv->grid_cols;
    }
    if(key == be->priv->key_pgdn){	
//	be->priv->edit_hex_cursor = 0;
//      be->priv->edit_addr_cursor += be->priv->grid_cols * be->priv->grid_rows - 1;
//	be->priv->edit_addr_cursor = ((be->priv->address_hl_start / be->priv->grid_cols) + 1) * be->priv->grid_cols - 1;
//	if(be->priv->edit_addr_cursor >= be->priv->buffer_size) be->priv->address_hl_start = be->priv->buffer_size - 1;
//      be->priv->edit_addr_cursor = be->priv->address_hl_start;
    }

    if(key == be->priv->key_tab){	
        be->priv->edit_hex ^= 0x03; 
        if(be->priv->edit_hex == 2) be->priv->edit_hex_cursor = 0; 
    }
    if((key >= 32) && (key < 128)) gui_bineditor_edit( be, key); 

    //scroll if outside grid window
    adj = gtk_adjustment_get_value(be->priv->adj);
    if( be->priv->edit_addr_cursor < adj){
	adj -= be->priv->grid_cols;
	if(adj < 0) adj = 0;
	gtk_adjustment_set_value(be->priv->adj, adj);
    }
    if( be->priv->edit_addr_cursor >= adj + be->priv->grid_cols * be->priv->grid_rows){
	adj += be->priv->grid_cols;
	if(adj >= be->priv->buff->size) adj = be->priv->buff->size - be->priv->grid_cols;
	gtk_adjustment_set_value(be->priv->adj, adj);
    }
    gtk_widget_queue_draw(be->priv->wmain);
}

static void gui_bineditor_focus_out(GtkWidget *wg, GdkEventKey *ev, GuiBineditor *be)
{
    be->priv->edit_hex = 0; 
    be->priv->clpb_end = be->priv->clpb_start = 0;
    gui_bineditor_exit_edit( be );
}

static void gui_bineditor_focus_in(GtkWidget *wg, GdkEventKey *ev, GuiBineditor *be)
{
    be->priv->edit_hex = 0; 
    be->priv->clpb_end = be->priv->clpb_start = 0;

    gtk_widget_set_sensitive(be->priv->clear, TRUE);
    gtk_widget_set_sensitive(be->priv->mjmp,  TRUE);
    gtk_widget_set_sensitive(be->priv->find,  TRUE);
    gtk_widget_set_sensitive(be->priv->rjmp,  FALSE);
}

static inline void gui_bineditor_vert_tool(GuiBineditor *be)
{
    be->priv->tbv = gtk_toolbar_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(be->priv->tbv), GTK_ORIENTATION_VERTICAL);
    gtk_toolbar_set_style(GTK_TOOLBAR(be->priv->tbv), GTK_TOOLBAR_ICONS);    

    /* Clear buffer */
    be->priv->clear = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_CLEAR ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->clear), TIP_BE_CLEAR_BUFFER);
    g_signal_connect(G_OBJECT(be->priv->clear), "clicked", G_CALLBACK(gui_bineditor_clear_buffer), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->clear), -1);

    /* Cut to buffer */
    be->priv->cut = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_CUT ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->cut), TIP_BE_CUT);
    g_signal_connect(G_OBJECT(be->priv->cut), "clicked", G_CALLBACK(gui_bineditor_cut), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->cut), -1);

    /* Copy to buffer */
    be->priv->copy = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_COPY ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->copy), TIP_BE_COPY);
    g_signal_connect(G_OBJECT(be->priv->copy), "clicked", G_CALLBACK(gui_bineditor_copy), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->copy), -1);
    gtk_widget_set_sensitive(be->priv->copy, FALSE );

    /* Find string */
    be->priv->find = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_FIND_AND_REPLACE ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->find), TIP_BE_FIND_STRING);
    g_signal_connect(G_OBJECT(be->priv->find), "clicked", G_CALLBACK(gui_bineditor_find_string), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->find), -1);

    /* Bit Manipulator */
    be->priv->manipulator = GTK_WIDGET(gtk_tool_button_new_from_stock( GUI_BINEDITOR_BITMAN_ICON ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->manipulator), TIP_BE_MANIPULATOR);
    g_signal_connect(G_OBJECT(be->priv->manipulator), "clicked", G_CALLBACK(gui_bineditor_manipulator), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->manipulator), -1);

    /* Byte organizer */
    be->priv->organizer = GTK_WIDGET(gtk_tool_button_new_from_stock( GUI_BINEDITOR_BYTMAN_ICON ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->organizer), TIP_BE_ORGANIZER);
    g_signal_connect(G_OBJECT(be->priv->organizer), "clicked", G_CALLBACK(gui_bineditor_organizer), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->organizer), -1);

    // Separator
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), gtk_separator_tool_item_new() , -1);

    /* Checksum calculator */
    be->priv->checksum = GTK_WIDGET(gtk_tool_button_new_from_stock( GUI_BINEDITOR_CRC_ICON ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->checksum), TIP_BE_CHECKSUM);
    g_signal_connect(G_OBJECT(be->priv->checksum), "clicked", G_CALLBACK(gui_bineditor_checksum), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->checksum), -1);

    /* Binary editor */
    be->priv->bined = GTK_WIDGET(gtk_tool_button_new_from_stock( GUI_BINEDITOR_BMP_ICON ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->bined), TIP_BE_BINED);
    g_signal_connect(G_OBJECT(be->priv->bined), "clicked", G_CALLBACK(gui_bineditor_bined), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->bined), -1);

    /* Text editor */
//    be->priv->texted = GTK_WIDGET(gtk_tool_button_new_from_stock( GUI_BINEDITOR_TEXT_ICON ));
//    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->texted), TIP_BE_TEXTED);
//    g_signal_connect(G_OBJECT(be->priv->texted), "clicked", G_CALLBACK(gui_bineditor_texted), be);
//    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->texted), -1);
//gtk_widget_set_sensitive( be->priv->texted, FALSE );
    /* Stencil */
    be->priv->stenc = GTK_WIDGET(gtk_tool_button_new_from_stock( GUI_BINEDITOR_STENCIL_ICON));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->stenc), TIP_BE_stencil);
    g_signal_connect(G_OBJECT(be->priv->stenc), "clicked", G_CALLBACK(gui_bineditor_stencil), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->stenc), -1);
    /* Asm viewer */
//    be->priv->asmview = GTK_WIDGET(gtk_tool_button_new_from_stock( GUI_BINEDITOR_ASM_ICON ));
//    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->asmview), TIP_BE_ASMVIEW);
//    g_signal_connect(G_OBJECT(be->priv->asmview), "clicked", G_CALLBACK(gui_bineditor_asmview), be);
//    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tbv), GTK_TOOL_ITEM(be->priv->asmview), -1);
//gtk_widget_set_sensitive( be->priv->asmview, FALSE );
}

void gui_bineditor_file_tool_insert(GuiBineditor *be)
{
    // Separator
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), gtk_separator_tool_item_new() , 0);

    // File write
    be->priv->i_write = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_SAVE ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->i_write), TIP_BE_WRITE);
    g_signal_connect(G_OBJECT(be->priv->i_write), "clicked", G_CALLBACK(gui_bineditor_write), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), GTK_TOOL_ITEM(be->priv->i_write), 0);
    
    // File open
    be->priv->i_open = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_OPEN ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->i_open), TIP_BE_OPEN);
    g_signal_connect(G_OBJECT(be->priv->i_open), "clicked", G_CALLBACK(gui_bineditor_open), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), GTK_TOOL_ITEM(be->priv->i_open), 0);
}

static inline void gui_bineditor_hor_tool(GuiBineditor *be)
{
    be->priv->tb = gtk_toolbar_new();

    be->priv->i_open  = NULL;
    be->priv->i_write = NULL;
        
    // Undo
    be->priv->i_undo = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_UNDO ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->i_undo), TIP_BE_UNDO);
    g_signal_connect(G_OBJECT(be->priv->i_undo), "clicked", G_CALLBACK(gui_bineditor_undo), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), GTK_TOOL_ITEM(be->priv->i_undo), -1);
gtk_widget_set_sensitive( be->priv->i_undo, FALSE );
    // Redo
    be->priv->i_redo = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_REDO ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->i_redo), TIP_BE_REDO);
    g_signal_connect(G_OBJECT(be->priv->i_redo), "clicked", G_CALLBACK(gui_bineditor_redo), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), GTK_TOOL_ITEM(be->priv->i_redo), -1);
gtk_widget_set_sensitive( be->priv->i_redo, FALSE );
    // Separator
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), gtk_separator_tool_item_new() , -1);

    /* Jump to marker */
    be->priv->mjmp = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_JUMP_TO ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->mjmp), TIP_BE_JUMP_TO_MARKER);
    g_signal_connect(G_OBJECT(be->priv->mjmp), "clicked", G_CALLBACK(gui_bineditor_jump_marker), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), GTK_TOOL_ITEM(be->priv->mjmp), -1);

    /* Return from marker */
    be->priv->rjmp = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_GO_BACK ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->rjmp), TIP_BE_REDO_JUMP);
    g_signal_connect(G_OBJECT(be->priv->rjmp), "clicked", G_CALLBACK(gui_bineditor_redo_marker), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), GTK_TOOL_ITEM(be->priv->rjmp), -1);

    // Separator
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), gtk_separator_tool_item_new() , -1);

    /* Run Script */
//    be->priv->exec = GTK_WIDGET(gtk_tool_button_new_from_stock( GTK_STOCK_EXECUTE ));
//    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->exec), TIP_BE_EXEC);
//    g_signal_connect(G_OBJECT(be->priv->exec), "clicked", G_CALLBACK(gui_bineditor_execute), be);
//    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), GTK_TOOL_ITEM(be->priv->exec), -1);

    /* Resize buffer */
    be->priv->resize = GTK_WIDGET(gtk_tool_button_new_from_stock( GUI_BINEDITOR_EXPAND_ICON ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->resize), TIP_BE_RESIZE);
    g_signal_connect(G_OBJECT(be->priv->resize), "clicked", G_CALLBACK(gui_bineditor_resize), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), GTK_TOOL_ITEM(be->priv->resize), -1);

    /* Auxiliary buffer */
    be->priv->aux = GTK_WIDGET(gtk_tool_button_new_from_stock( GUI_BINEDITOR_AUX_ICON ));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(be->priv->aux), TIP_BE_AUX);
    g_signal_connect(G_OBJECT(be->priv->aux), "clicked", G_CALLBACK(gui_bineditor_aux), be);
    gtk_toolbar_insert( GTK_TOOLBAR(be->priv->tb), GTK_TOOL_ITEM(be->priv->aux), -1);

/* shadows buttons */
    gtk_widget_set_sensitive(be->priv->mjmp, FALSE);
    gtk_widget_set_sensitive(be->priv->rjmp, FALSE);
}

static inline GtkWidget *gui_bineditor_coord(GuiBineditor *be)
{
    GtkWidget *wg1, *wg2;

    /* info fields */
    wg1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    be->priv->info_addr = gtk_label_new("0000:00");

    wg2 = gtk_frame_new(NULL);
    gtk_widget_set_size_request( wg2, 120, 20);
    gtk_frame_set_shadow_type(GTK_FRAME(wg2), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(wg2), be->priv->info_addr);
    gtk_box_pack_start(GTK_BOX(wg1), wg2, FALSE, FALSE, 0);
    be->priv->info_mark = gtk_label_new("0000:00");
    wg2 = gtk_frame_new(NULL);
    gtk_widget_set_size_request( wg2, 120, 20);
    gtk_container_add(GTK_CONTAINER(wg2), be->priv->info_mark);
    gtk_frame_set_shadow_type(GTK_FRAME(wg2), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(wg1), wg2, FALSE, FALSE, 0);
    return wg1;
}

static inline GtkWidget *gui_bineditor_hex_viewer(GuiBineditor *be)
{
    GtkWidget *wg0, *wg1, *wg2;
// shadowed in drawing area for CAIRO and slider 
    wg0 = gtk_frame_new(NULL);    
    gtk_frame_set_shadow_type(GTK_FRAME(wg0), GTK_SHADOW_IN);

    wg1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(wg0), wg1);

    /* add vertical slider */
    be->priv->adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1, 1, 0, 0));
    wg2 = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, be->priv->adj);
    gtk_box_pack_end(GTK_BOX(wg1), wg2, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(be->priv->adj), "value_changed", G_CALLBACK(gui_bineditor_slider), be);

    /* create drawing area and connect signals */
    be->priv->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_can_focus(be->priv->drawing_area, TRUE);
    gtk_widget_set_events(be->priv->drawing_area, 
	GDK_SCROLL_MASK | GDK_BUTTON_MOTION_MASK | GDK_POINTER_MOTION_MASK
	| GDK_LEAVE_NOTIFY_MASK | GDK_ENTER_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK 
	| GDK_KEY_PRESS_MASK | GDK_FOCUS_CHANGE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_RELEASE_MASK
    );
    
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"draw",G_CALLBACK(gui_bineditor_draw_sig), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"scroll_event",G_CALLBACK(gui_bineditor_scroll), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"button_press_event",G_CALLBACK(gui_bineditor_mbutton), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"button_release_event",G_CALLBACK(gui_bineditor_mbutton_free), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"motion_notify_event",G_CALLBACK(gui_bineditor_hint), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"leave_notify_event",G_CALLBACK(gui_bineditor_leave), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"enter_notify_event",G_CALLBACK(gui_bineditor_enter), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"key_press_event",G_CALLBACK(gui_bineditor_keystroke), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"key_release_event",G_CALLBACK(gui_bineditor_keystroke), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"focus_out_event",G_CALLBACK(gui_bineditor_focus_out), be);
    g_signal_connect(G_OBJECT(be->priv->drawing_area),"focus_in_event",G_CALLBACK(gui_bineditor_focus_in), be);
    gtk_box_pack_end(GTK_BOX(wg1), be->priv->drawing_area, TRUE, TRUE, 1);
    return wg0;
}

static void gui_bineditor_add_icons( GuiBineditor *be)
{
    static char first_run = 1;
    GtkIconFactory *ifc;

    if( first_run == 0) return;
    first_run = 0;
    ifc = gtk_icon_factory_new();

    gtk_icon_factory_add(ifc, GUI_BINEDITOR_INVERSE_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_BMP_INVERSE_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_MIRROR_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_BMP_MIRROR_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_GRID_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_BMP_GRID_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_LIVE_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_BMP_LIVE_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_EDIT_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_BMP_EDIT_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_BITMAN_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_BITMAN_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_BYTMAN_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_BYTMAN_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_CRC_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_CRC_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_BMP_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_BMP_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_TEXT_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_TEXT_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_STENCIL_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_STENCIL_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_ASM_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_ASM_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_EXPAND_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_EXPAND_ICON)));
    gtk_icon_factory_add(ifc, GUI_BINEDITOR_AUX_ICON, gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data( BE_AUX_ICON)));

    gtk_icon_factory_add_default( ifc );    
}

static inline void gui_bineditor_init(GuiBineditor *be)
{
    GtkWidget *wg0, *wg1, *whb;

    be->priv = G_TYPE_INSTANCE_GET_PRIVATE (be, GUI_TYPE_BINEDITOR, GuiBineditorPrivate);
    
    g_return_if_fail(be != NULL);
    g_return_if_fail(GUI_IS_BINEDITOR(be));

    gui_bineditor_buff_constr( &be->priv->buff );

    be->priv->aux_ed = NULL;
    be->priv->icon = NULL;
    be->priv->root = NULL;
    be->priv->cut_data.size = 0;
    be->priv->cut_data.data = NULL;
    be->priv->markers.count = 0;    
    be->priv->aux_buffer = NULL;
    be->priv->aux_size = 0;
    be->priv->core_count = 10;
    be->priv->core_name = NULL;
    be->priv->atom = gdk_atom_intern("PRIMARY", FALSE);
    be->priv->clipb = gtk_clipboard_get( be->priv->atom );
    be->priv->edit_hex = 0;
    be->priv->address_mark = 0;
    be->priv->address_mark_redo = 0;
    be->priv->address_old_hint = -1;
    be->priv->clpb_start = -1;
    be->priv->clpb_end = -1;
    be->priv->adj = NULL;
    be->priv->drawing_area = NULL;
    be->priv->tb = NULL;
    be->priv->buff->size = 0;
    be->priv->buff->data = NULL;    
    be->priv->statusbar = NULL;
    be->priv->statusbar_id = 0;
    be->priv->properties = ~0;
    be->priv->vfind = NULL;
    be->priv->vreplace = NULL;
    be->priv->rfsh = 0; /* force redrawing */
    be->priv->texted_str = NULL;

    be->priv->key_left = gdk_keyval_from_name("Left");
    be->priv->key_right = gdk_keyval_from_name("Right");
    be->priv->key_up = gdk_keyval_from_name("Up");
    be->priv->key_down = gdk_keyval_from_name("Down");
    be->priv->key_home = gdk_keyval_from_name("Home");
    be->priv->key_end = gdk_keyval_from_name("End");
    be->priv->key_pgup = gdk_keyval_from_name("Page_Up");
    be->priv->key_pgdn = gdk_keyval_from_name("Page_Down");
    be->priv->key_tab = gdk_keyval_from_name("Tab");

    SET_COLOR(be, GUI_BINEDITOR_COLOR_UD, 1, 0.5, 0.5);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_HL, 0.8, 0.8, 0.8);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_HL_MRK, 0.6, 0.6, 0.6);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_MARKED_BG, 0.3,0.3,0.3);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_MARKED, 0, 1, 0);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_NORMAL, 0, 0, 0);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_DESC, 0, 1, 1);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_GRID, 0.8, 0.8, 0.8);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_GRID_BG, 1, 1, 0.9);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_DESC_BG, 0.3, 0.3, 1);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_MARKER_FG_FOUND, 0.0, 0.0, 0.0);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_MARKER_BG_FOUND, 0.9, 0.9, 0.3);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_BMP_BG, 0.9, 0.9, 1.0);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_BMP_GRID, 0.8, 0.8, 0.9);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_BMP_PIXEL, 0.0, 0.5, 0.0);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_BMP_AMBIENT, 0.0, 0.0, 0.2);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_TXT_AMBIENT, 0.0, 0.0, 0.5);
    SET_COLOR(be, GUI_BINEDITOR_COLOR_TXT_FONT, 0.5, 1.0, 0.5);

    gui_bineditor_add_icons( be );
    gui_bineditor_marker_new(be, GUI_BINEDITOR_MARKER_ALL);
    gui_bineditor_marker_set_color(be, GUI_BINEDITOR_MARKER_SELECTED, GUI_BINEDITOR_COLOR_TEXT_MARKED, GUI_BINEDITOR_COLOR_HL_MRK );
    gui_bineditor_marker_set_color(be, GUI_BINEDITOR_MARKER_FOUND, GUI_BINEDITOR_COLOR_MARKER_FG_FOUND, GUI_BINEDITOR_COLOR_MARKER_BG_FOUND );
    be->priv->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_box_pack_start(GTK_BOX(be), be->priv->vbox, TRUE, TRUE, 0);        

// horizontal toolbar 
    gui_bineditor_hor_tool( be );
    gtk_box_pack_start(GTK_BOX(be->priv->vbox), be->priv->tb, FALSE, FALSE, 0);    

// coordinates
    wg1 = gui_bineditor_coord( be );
    gtk_box_pack_start(GTK_BOX(be->priv->vbox), wg1, FALSE, FALSE, 0);

//---> Hex viewer and left toolbar
    whb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(be->priv->vbox), whb, TRUE, TRUE, 0);

// vertical toolbar 
    gui_bineditor_vert_tool( be );    
    gtk_box_pack_start(GTK_BOX(whb), be->priv->tbv, FALSE, FALSE, 0);    

// hex viewer
    wg0 = gui_bineditor_hex_viewer( be );
    gtk_box_pack_start(GTK_BOX(whb), wg0, TRUE, TRUE, 0);    

}

void gui_bineditor_set_buffer(GuiBineditor *be, int bfsize, unsigned char *buffer)
{
    g_return_if_fail(be != NULL);
    g_return_if_fail(GUI_IS_BINEDITOR(be));
    g_return_if_fail(buffer != NULL);

    be->priv->buff->size = bfsize;
    be->priv->buff->data = bfsize ? buffer: NULL;

//    if(be->priv->bmp)
//	gtk_widget_destroy( be->priv->bmp );


    gtk_adjustment_set_lower(be->priv->adj, 0);
    gtk_adjustment_set_upper(be->priv->adj,bfsize);
    gtk_adjustment_set_page_size(be->priv->adj, 0);
    gtk_adjustment_set_value(be->priv->adj, 0);
    gtk_adjustment_value_changed(be->priv->adj);
    gtk_widget_set_sensitive(be->priv->clear, bfsize);
    gtk_widget_set_sensitive(be->priv->mjmp, bfsize);
    gtk_widget_set_sensitive(be->priv->find, bfsize);
/* now everything to redraw */
    be->priv->rfsh = 0; 
    gtk_widget_queue_draw(be->priv->drawing_area);
}

void gui_bineditor_set_colors(GuiBineditor *be, GuiBineditorColors color, float r, float g, float b)
{
    int tmp = color;
    g_return_if_fail(be != NULL);
    g_return_if_fail(GUI_IS_BINEDITOR(be));
    g_return_if_fail(tmp < GUI_BINEDITOR_COLOR_LAST);
    
    tmp *= 3;
    be->priv->colors[color + 0] = r;
    be->priv->colors[color + 1] = g;
    be->priv->colors[color + 2] = b;
}

static void gui_widget_show(GtkWidget *wg, gboolean flag)
{
    if(flag)
	gtk_widget_show(wg);
    else
	gtk_widget_hide(wg);    
}

void gui_bineditor_set_properties(GuiBineditor *be, GuiBineditorProperties pr)
{
    g_return_if_fail(be != NULL);
    g_return_if_fail(GUI_IS_BINEDITOR(be));
    g_return_if_fail(be->priv->tb != NULL);
    g_return_if_fail(GTK_IS_TOOLBAR(be->priv->tb));

    be->priv->properties = pr;

    gui_widget_show(be->priv->tb,     pr & GUI_BINEDITOR_PROP_TOOLBAR);
    gui_widget_show(be->priv->find,   pr & GUI_BINEDITOR_PROP_FIND);
    gui_widget_show(be->priv->mjmp,   pr & GUI_BINEDITOR_PROP_MJMP);
    gui_widget_show(be->priv->rjmp,   pr & GUI_BINEDITOR_PROP_RJMP);
    gui_widget_show(be->priv->calc,   pr & GUI_BINEDITOR_PROP_CALC);
    gui_widget_show(be->priv->clear,  (pr & GUI_BINEDITOR_PROP_CLEAR) && (pr & GUI_BINEDITOR_PROP_EDITABLE));
//    gui_widget_show(be->priv->aux,   pr & GUI_BINEDITOR_PROP_CHILD);
}

void gui_bineditor_connect_statusbar(GuiBineditor *be, GtkWidget *sb)
{
    g_return_if_fail(be != NULL);
    g_return_if_fail(GUI_IS_BINEDITOR(be));
    g_return_if_fail(sb != NULL);
    g_return_if_fail(GTK_IS_STATUSBAR(sb));
    g_return_if_fail(be->priv->statusbar == NULL);

    be->priv->statusbar = sb;
    be->priv->statusbar_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(sb), "bineditor_statusbar");
}

void gui_bineditor_redraw(GuiBineditor *be)
{
    gtk_widget_queue_draw(be->priv->wmain);
    gui_bineditor_bitmap_redraw( be );
//    gui_bineditor_text_rfsh( be );
}

/**********************************************************************************/

void gui_bineditor_marker_new(GuiBineditor *be, unsigned int count)
{
    int i;
    if(be->priv->markers.count != 0){
	printf("Warning: gui_bineditor_marker_new() -> markers was already created.\n");
	return;
    }
    if((be->priv->markers.items = (GuiBineditorMarkerItem *)malloc( count * sizeof(GuiBineditorMarkerItem))) == NULL){
	printf("Warning: gui_bineditor_marker_new() -> markers cannot be created. Memory problem.\n");
	return;
    }
    be->priv->markers.count = count;
    
    for(i = 0; i < count; i++){
	be->priv->markers.items[i].active = 0;
	be->priv->markers.items[i].from = 0;
	be->priv->markers.items[i].to = 0;
	be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_BR] = 0;
	be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_BG] = 0;
	be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_BB] = 0;
	be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_FR] = 0;
	be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_FG] = 0;
	be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_FB] = 0;
	be->priv->markers.items[i].mode = 0;
    }
}

void gui_bineditor_marker_free(GuiBineditor *be)
{
    if(be->priv->markers.items) free(be->priv->markers.items);
    be->priv->markers.count = 0;
}

void gui_bineditor_marker_set_color(GuiBineditor *be, GuiBineditorMarkerType id, GuiBineditorColors fg, GuiBineditorColors bg)
{
    if(be->priv->markers.count == 0){
	printf("[WARN] gui_bineditor_marker_set_color() -> Marker list is empty.\n");
	return;
    }
    if((id >= be->priv->markers.count)||(id < 0)){
	printf("[WARN] gui_bineditor_marker_set_color() -> Marker index is out of range.\n");
	return;
    }
    if((bg >= GUI_BINEDITOR_COLOR_LAST ) || (fg >= GUI_BINEDITOR_COLOR_LAST ) || (bg < 0) || (fg < 0)){
	printf("[WARN] gui_bineditor_marker_set_color() -> Color index is out of range.\n");
	return;
    }
    be->priv->markers.items[id].colors[GUI_BINEDITOR_COLOR_BG_BR] = GUI_BINEDITOR_COLOR_R(be, bg);
    be->priv->markers.items[id].colors[GUI_BINEDITOR_COLOR_BG_BG] = GUI_BINEDITOR_COLOR_G(be, bg);
    be->priv->markers.items[id].colors[GUI_BINEDITOR_COLOR_BG_BB] = GUI_BINEDITOR_COLOR_B(be, bg);
    be->priv->markers.items[id].colors[GUI_BINEDITOR_COLOR_BG_FR] = GUI_BINEDITOR_COLOR_R(be, fg);
    be->priv->markers.items[id].colors[GUI_BINEDITOR_COLOR_BG_FG] = GUI_BINEDITOR_COLOR_G(be, fg);
    be->priv->markers.items[id].colors[GUI_BINEDITOR_COLOR_BG_FB] = GUI_BINEDITOR_COLOR_B(be, fg);
    be->priv->markers.items[id].mode = 0;
}

void gui_bineditor_marker_set_item(GuiBineditor *be, GuiBineditorMarkerType id, GuiBineditorMarkerMode mode, unsigned int from, unsigned int to )
{
    if(id < 0) return;
    if(be->priv->markers.count == 0){
	printf("[WARN] try to set marker in empty marker list.\n");
	return;
    }
    if(id >= be->priv->markers.count){
	printf("[WARN] try to set marker over marker list.\n");
	return;    
    }
    be->priv->markers.items[id].active = 1;
    be->priv->markers.items[id].from = from;
    be->priv->markers.items[id].to = to;    
    be->priv->markers.items[id].mode = mode;
}

void gui_bineditor_marker_unset_item(GuiBineditor *be, GuiBineditorMarkerType id)
{
    if(id < 0) return;
    if(be->priv->markers.count == 0) return;
    if(id >= be->priv->markers.count) return;    
    be->priv->markers.items[id].active = 0;
    be->priv->markers.items[id].from = 0;
    be->priv->markers.items[id].to = 0;    
}

void gui_bineditor_marker_show_item(GuiBineditor *be, GuiBineditorMarkerType id, char show)
{
    if(id < 0) return;
    if(be->priv->markers.count == 0) return;
    if(id >= be->priv->markers.count) return;    
    be->priv->markers.items[id].active = show;
}

void gui_bineditor_marker_set_value(GuiBineditor *be, GuiBineditorMarkerType id, unsigned int from, unsigned int to)
{
    if(id < 0) return;
    if(be->priv->markers.count == 0) return;
    if(id >= be->priv->markers.count) return;    
    be->priv->markers.items[id].from = from;
    be->priv->markers.items[id].to = to;
}

char gui_bineditor_marker_get_range(GuiBineditor *be, GuiBineditorMarkerType id, unsigned int *from, unsigned int *to)
{
    unsigned int tmp;
    
    *from = be->priv->markers.items[id].from;
    *to   = be->priv->markers.items[id].to;

    if(*to < *from){
	tmp = *from;
	*from = *to;
	*to = tmp;
    }

    return be->priv->markers.items[id].active;
}

/****************************************************************************************************************************/

#define nx	2	
#define border	4	
#define fx	6 	
#define fy	10	
#define ac	8 	
#define ry	26	
#define underln_pos 3   

static inline void gui_bineditor_draw_compute( GuiBineditor *be, int vxx, int vyy )
{
    be->priv->grid_top    = ry;
    be->priv->ascii_space = fx + border;
    be->priv->cell_height = fy + border;
    be->priv->grid_start = (ac + 2) * be->priv->ascii_space;
    be->priv->cell_width = nx * be->priv->ascii_space;
    be->priv->grid_cols = (((vxx - be->priv->grid_start - 2*be->priv->ascii_space) / be->priv->ascii_space) / 3);
    be->priv->grid_rows = (vyy - be->priv->grid_top) / (be->priv->cell_height + 1);
    be->priv->ascii_start = (vxx + be->priv->grid_start + be->priv->ascii_space + be->priv->grid_cols * (be->priv->cell_width - be->priv->ascii_space)) / 2;
    be->priv->pagesize  = be->priv->grid_cols * be->priv->grid_rows;
    be->priv->grid_end  = be->priv->ascii_start - be->priv->cell_width;
    be->priv->ascii_end = be->priv->ascii_start + be->priv->ascii_space * be->priv->grid_cols;
}	

static inline void gui_bineditor_set_slider_adj( GuiBineditor *be)
{
    gtk_adjustment_set_page_size(be->priv->adj, be->priv->pagesize);
    gtk_adjustment_set_page_increment(be->priv->adj, be->priv->grid_cols);
    gtk_adjustment_set_step_increment(be->priv->adj, be->priv->grid_cols);
}

static inline void gui_bineditor_set_font(cairo_t *cr)
{
    cairo_select_font_face(cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
}

static inline void gui_bineditor_draw_background(GuiBineditor *be, cairo_t *cr, int vxx, int vyy )
{
    cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_GRID_BG));
    cairo_rectangle(cr, 0, 0, vxx, vyy);
    cairo_fill(cr);
    cairo_stroke(cr);    
}

static inline void gui_bineditor_draw_description_background(GuiBineditor *be, cairo_t *cr, int vxx, int vyy )
{
    cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_DESC_BG));
    cairo_rectangle(cr, 0, 0, vxx, be->priv->cell_height + 5);
    cairo_fill(cr);
    cairo_stroke(cr);    
}

static inline void gui_bineditor_draw_grid(GuiBineditor *be, cairo_t *cr, int vxx, int vyy )    
{
    int xx, yy, i;
    
    cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_GRID));    
    for(i = 0, yy = be->priv->grid_top + be->priv->cell_height; i < be->priv->grid_rows; i++, yy += be->priv->cell_height + 1) gui_cairo_line(cr, 0, yy, vxx, yy);
    for(i = 0, xx = be->priv->grid_start; i < be->priv->grid_cols + 1; i++, xx += be->priv->cell_width ) gui_cairo_line(cr, xx, 0, xx, vyy);
    cairo_stroke(cr);
}

static inline void gui_bineditor_draw_description_grid_label(GuiBineditor *be, cairo_t *cr, char *tmp)
{
    int xx, i;

    cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_DESC));    
    for(i = 0, xx = be->priv->grid_start; i < be->priv->grid_cols; i++, xx += be->priv->cell_width)
	gui_cairo_outtext(cr, tmp, xx + 1 , 4 + fy, "%c%c", gui_dig2hex(i / 16), gui_dig2hex(i % 16));
    gui_cairo_outtext(cr, tmp, border , 4 + fy, TXT_BE_GRID_LBL_ADDRESS);
    gui_cairo_outtext(cr, tmp, be->priv->ascii_start , 4 + fy, TXT_BE_GRID_LBL_ASCII);
    cairo_stroke(cr);
}

static inline void gui_bineditor_draw_address(GuiBineditor *be, cairo_t *cr, int y, int addr, char *tmp)
{
    int i;
    
    for(i = 0; i < 8; i++)
        gui_cairo_outtext( cr, tmp, be->priv->ascii_space * (8 - i), y, "%c", gui_dig2hex( (addr >> ( i * 4 ) ) & 0xf) );
}

static inline void gui_bineditor_draw_hex(GuiBineditor *be, cairo_t *cr, char *tmp, int xx, int yy, int n)
{
    char hl;
    
    hl = be->priv->markers.hit && (be->priv->markers.mode & GUI_BINEDITOR_MARKER_HEX);
    
    if( hl )
	cairo_set_source_rgb(cr, be->priv->markers.fr, be->priv->markers.fg, be->priv->markers.fb);
    
    gui_cairo_outtext(cr, tmp, xx + 1, yy , "%c%c", gui_dig2hex(n / 16), gui_dig2hex(n % 16));

    if( hl )
	cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_NORMAL));        
}

static inline void gui_bineditor_draw_block(GuiBineditor *be, cairo_t *cr, int xa, int yy)
{
    cairo_rectangle(cr, xa, yy - fy, be->priv->ascii_space - 1, be->priv->cell_height - 1);
    cairo_fill(cr);
}

static inline void gui_bineditor_draw_underline(GuiBineditor *be, cairo_t *cr, int addr, int xx, int xa, int yy)
{
    int x, bl_pos;

    if( !be->priv->edit_hex) return;
    if(be->priv->edit_addr_cursor != addr) return;
//    cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_NORMAL));    
    x = (be->priv->edit_hex == 2) ? xa : xx;
    bl_pos = (be->priv->edit_hex_cursor & 1) * (fx + 2);
    gui_cairo_line(cr, x + 1 + bl_pos, yy + underln_pos, x + 1 + fx + 1 + bl_pos, yy + underln_pos);
    cairo_stroke(cr);
}

static char gui_bineditor_filter(GuiBineditor *be, int chr)
{
    return !(((chr > 0x1f) && (chr < 0x80)));
}

static inline void gui_bineditor_draw_ascii(GuiBineditor *be, cairo_t *cr, int xa, int yy, int n, char *tmp)
{
    char filter, hl;
    float rr, rg, rb;
    
    rr = GUI_BINEDITOR_COLOR_R(be, GUI_BINEDITOR_COLOR_UD);
    rg = GUI_BINEDITOR_COLOR_G(be, GUI_BINEDITOR_COLOR_UD);
    rb = GUI_BINEDITOR_COLOR_B(be, GUI_BINEDITOR_COLOR_UD);
    filter = gui_bineditor_filter(be, n);
    hl = be->priv->markers.hit && (be->priv->markers.mode & GUI_BINEDITOR_MARKER_ASCII);

    if( hl ){
	rr = be->priv->markers.fr;
	rg = be->priv->markers.fg;
	rb = be->priv->markers.fb;
    }

    if( filter || hl )
	    cairo_set_source_rgb(cr, rr, rg, rb);

    if( filter )
	gui_bineditor_draw_block(be, cr, xa, yy);
    else
	gui_cairo_outtext(cr, tmp, xa, yy, "%c", n);

    if( filter || hl  )
	cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_NORMAL));    

    cairo_stroke(cr);
}	    

static inline void gui_bineditor_draw_pointer(GuiBineditor *be, cairo_t *cr, int addr, int xx, int xa, int yy)
{
    if( be->priv->address_mark != addr ) return;
    cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_MARKED_BG));
    cairo_rectangle(cr, xa, yy - fy, be->priv->ascii_space - 1, be->priv->cell_height - 1);
    cairo_rectangle(cr, xx, yy - fy, be->priv->cell_width - 1,  be->priv->cell_height - 1);
    cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_NORMAL));    
    cairo_stroke(cr);
}

static inline void gui_bineditor_test_marked(GuiBineditor *be, unsigned int addr)
{
    unsigned int i, a, b, c;
    float scale;
    
    be->priv->markers.hit = 0;    
    be->priv->markers.br = 0.0; be->priv->markers.bg = 0.0; be->priv->markers.bb = 0.0;
    be->priv->markers.fr = 0.0; be->priv->markers.fg = 0.0; be->priv->markers.fb = 0.0;
    be->priv->markers.mode = 0;
    for( i = 0; i < be->priv->markers.count; i++ ){    
	if( !be->priv->markers.items[i].active ) continue;
	a = be->priv->markers.items[i].from;
	b = be->priv->markers.items[i].to;
	if( b < a ) {
	    c = a;
	    a = b;
	    b = c;
	}	
	if( (addr < a ) || (addr > b) ) continue;
	scale = be->priv->markers.hit + 1;
	be->priv->markers.br += be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_BR] / scale;
	be->priv->markers.bg += be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_BG] / scale;
	be->priv->markers.bb += be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_BB] / scale;
	be->priv->markers.fr += be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_FR] / scale;
	be->priv->markers.fg += be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_FG] / scale;
	be->priv->markers.fb += be->priv->markers.items[i].colors[GUI_BINEDITOR_COLOR_BG_FB] / scale;
	be->priv->markers.mode |= be->priv->markers.items[i].mode;
	be->priv->markers.hit++;
    }
    if( !be->priv->markers.hit ) return; // there are no blocks to draw
    be->priv->markers.hit = 1;
}

static inline void gui_bineditor_draw_marked(GuiBineditor *be, cairo_t *cr, int addr, int xx, int xa, int yy)
{                                                                                                             
    if( !be->priv->markers.hit ) return;

    cairo_set_source_rgb(cr, be->priv->markers.br, be->priv->markers.bg, be->priv->markers.bb);

    if( be->priv->markers.mode & GUI_BINEDITOR_MARKER_ASCII )
	cairo_rectangle(cr, xa, yy - fy, be->priv->ascii_space - 1, be->priv->cell_height - 1);

    if( be->priv->markers.mode & GUI_BINEDITOR_MARKER_HEX )
	cairo_rectangle(cr, xx, yy - fy, be->priv->cell_width - 1,  be->priv->cell_height - 1);

    cairo_fill(cr);
    cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_NORMAL));
    cairo_stroke(cr);

}

static inline void gui_bineditor_draw_line_context(GuiBineditor *be, cairo_t *cr, int yy, int addr, int max_addr, char *tmp)
{    
    int i, xa, xx, data;

    for(i = 0, xx = be->priv->grid_start, xa = be->priv->ascii_start; (i < be->priv->grid_cols) && (addr < max_addr); i++, xx += be->priv->cell_width, xa += be->priv->ascii_space, addr++ ){
        data = (be->priv->buff->data) ? be->priv->buff->data[addr] : 0;	
	gui_bineditor_test_marked(be, addr);
	gui_bineditor_draw_marked(be, cr, addr, xx, xa, yy);
	gui_bineditor_draw_pointer(be, cr, addr, xx, xa, yy);
        gui_bineditor_draw_hex(be, cr, tmp, xx, yy, data);
	gui_bineditor_draw_ascii(be, cr, xa, yy, data, tmp);
	gui_bineditor_draw_underline(be, cr, addr, xx, xa, yy);
    }
}

static void gui_bineditor_draw( GuiBineditor *be, cairo_t *cr, int vxx, int vyy, char print)
{
    char tmp[16];
    unsigned int i, addr,max_addr, yy;
    
    gui_bineditor_draw_compute( be, vxx, vyy );
    gui_bineditor_set_slider_adj( be );
    gui_bineditor_set_font( cr );
    gui_bineditor_draw_background( be, cr, vxx, vyy );
    gui_bineditor_draw_description_background( be, cr, vxx, vyy );
    gui_bineditor_draw_grid(be, cr, vxx, vyy);
    gui_bineditor_draw_description_grid_label(be, cr, tmp);

    be->priv->markers.hit = 0;
    max_addr = be->priv->buff->size;
    addr =  gui_bineditor_get_grid_start_address( be );
    cairo_set_source_rgb(cr, GET_COLOR(be, GUI_BINEDITOR_COLOR_TEXT_NORMAL));
    for(i = 0, yy = be->priv->grid_top + fy; (i < be->priv->grid_rows) && (addr < max_addr); i++, yy += be->priv->cell_height + 1, addr += be->priv->grid_cols){
	gui_bineditor_draw_address(be, cr, yy, addr, tmp);
	gui_bineditor_draw_line_context(be, cr, yy, addr, max_addr, tmp );
    }
    cairo_stroke(cr);
}

unsigned int gui_bineditor_grid_get_page_size( GuiBineditor *be )
{
    return be->priv->grid_rows * be->priv->grid_cols;
}

void gui_bineditor_show_grid(GuiBineditor *be, unsigned int from, unsigned int to)
{
    unsigned int saddr, eaddr;
    char visible = 1;

// get visible address range    
    saddr = gui_bineditor_get_grid_start_address( be );
    eaddr = saddr + gui_bineditor_grid_get_page_size( be );
// test 
    if(( from < saddr) || ( from > eaddr)) visible = 0;
    if(( to < saddr) || ( to > eaddr)) visible = 0;
// move to visible region
    if(!visible){
	gtk_adjustment_set_value(be->priv->adj, from);
	gtk_adjustment_value_changed(be->priv->adj);    
    }
// refresh
    gtk_widget_queue_draw(be->priv->drawing_area);
}

/****************************************************/
GuiBineditor *gui_bineditor_get_root(GuiBineditor *be)
{
    return be->priv->root == NULL ? be : be->priv->root;
}

void gui_bineditor_cut_store(GuiBineditor *be, unsigned int from, unsigned int to)
{

    unsigned char *data;
    GuiBineditor *bt;

    bt = gui_bineditor_get_root(be);
    bt->priv->cut_data.size = (from > to ? from - to : to - from) + 1;	
    if(( from >= be->priv->buff->size) || ( to >= be->priv->buff->size)){
	printf("WARN: gui_bineditor_cut_save() -> index exceed buffer\n");	
	return;
    }

    data = (unsigned char *)realloc(bt->priv->cut_data.data, bt->priv->cut_data.size );
    if( data == NULL ){
	printf("WARN: gui_bineditor_cut_store() -> memory allocation problem\n");
	return;
    }
    bt->priv->cut_data.data = data;
    memcpy(bt->priv->cut_data.data, be->priv->buff->data, bt->priv->cut_data.size);
    // 
    gtk_widget_set_sensitive(be->priv->copy, TRUE);
    if(be->priv->aux_ed && be->priv->aux)
	gtk_widget_set_sensitive(GUI_BINEDITOR(be->priv->aux_ed)->priv->copy, TRUE);    
}

void gui_bineditor_cut_restore(GuiBineditor *be, unsigned int from)
{
    unsigned int size;
    GuiBineditor *bt;
    
    bt = gui_bineditor_get_root(be);
    size = bt->priv->cut_data.size;
    if( from >= be->priv->buff->size){
	printf("WARN: gui_bineditor_cut_restore() -> index exceed buffer\n");	
	return;
    }
    
    if( from + size >= be->priv->buff->size) size = be->priv->buff->size - from - 1;
    gui_bineditor_buff_history_add(be->priv->buff, from, from + size - 1);
    memcpy(be->priv->buff->data + from, bt->priv->cut_data.data, size);
    gui_bineditor_redraw( be );
}

void gui_bineditor_set_icon(GuiBineditor *be, const char **xpm_data)
{
    be->priv->icon = xpm_data;
}
