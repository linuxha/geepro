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

#include <stdio.h>
#include <stdlib.h>
#include "../intl/lang.h"
#include "be_bitmap_int.h"

/***********************************************
*            Public functions                  *
************************************************/
void gui_bineditor_bitmap(GuiBineditor *be, unsigned int width, unsigned int height, unsigned char mask, unsigned char br)
{
    gui_bitmap_bmp_str *s;
    GtkWidget *fr, *wg;

    s = (gui_bitmap_bmp_str *)malloc(sizeof(gui_bitmap_bmp_str));
    if( !s ){
	printf("ERR: gui_bineditor_bitmap() -> memory allocation problem.\n");
	return;    
    }

    s->off_x0 = 0;
    s->off_y0 = 0;
    s->off_x1 = 0;
    s->off_y1 = 0;

    s->first_run = 1;
    s->inverse = 0;
    s->mirror = br;
    s->grid = 1;
    s->live = 0;
    s->edit = 0;
    s->addr_shift = 0;
    s->addr = 0;
    s->val_multip = 0;
    s->val_offset = 0;
    s->be = be;
    s->width = width;
    s->height = height;
    s->bit_mask = mask;
    be->priv->bmp = s;
    gui_bineditor_bmp_mask_generator( s );

    gtk_widget_set_sensitive( be->priv->bined, FALSE );
    s->wmain = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title(GTK_WINDOW(s->wmain),TEXT(WINTITLE_BINEDITOR));
    g_signal_connect(G_OBJECT(s->wmain), "destroy", G_CALLBACK(gui_bineditor_bmp_destroy), be);
    if( be->priv->icon )
	gtk_window_set_icon( GTK_WINDOW( s->wmain ), gdk_pixbuf_new_from_xpm_data( be->priv->icon ));
    gtk_window_set_keep_above(GTK_WINDOW(s->wmain), TRUE);
    gtk_widget_set_size_request( s->wmain, 320, 320); //
    s->ctx = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); 
    gtk_container_add(GTK_CONTAINER(s->wmain), s->ctx);
    s->tb_line = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3); 
    gtk_box_pack_start(GTK_BOX(s->ctx), s->tb_line, FALSE, FALSE, 0 );
    s->param_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(s->ctx), s->param_hbox, FALSE, FALSE, 0);
    s->info_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(s->ctx), s->info_hbox, FALSE, FALSE, 0);
    fr = gtk_frame_new( NULL );    
    gtk_box_pack_start(GTK_BOX(s->ctx), fr, TRUE, TRUE, 0);    
    wg = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(fr), wg);
    fr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(wg), fr, TRUE, TRUE, 0);
    s->draw_area = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(fr), s->draw_area, TRUE, TRUE, 0);
    s->vadj = gtk_adjustment_new(0, 0, 1, 1, 0, 0);
    s->vslid = gtk_scrollbar_new( GTK_ORIENTATION_VERTICAL, s->vadj );
    gtk_box_pack_end(GTK_BOX(fr), s->vslid, FALSE, FALSE, 0);
    s->hadj = gtk_adjustment_new(0, 0, 1, 1, 0, 0);
    s->hslid = gtk_scrollbar_new( GTK_ORIENTATION_HORIZONTAL, s->hadj );
    gtk_box_pack_end(GTK_BOX(wg), s->hslid, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(s->vadj), "value_changed", G_CALLBACK(gui_bineditor_bmp_vch), s);
    g_signal_connect(G_OBJECT(s->hadj), "value_changed", G_CALLBACK(gui_bineditor_bmp_hch), s);

    gui_bineditor_bmp_toolbar_setup( s );
    gui_bineditor_bmp_parameter_setup( s );
    gui_bineditor_bmp_drawing_setup( s );
    gtk_widget_show_all( s->wmain );
}

void gui_bineditor_bitmap_set_address(GuiBineditor *be, unsigned int address)
{
    if( be->priv->bmp == NULL) return;
    ((gui_bitmap_bmp_str *)be->priv->bmp)->addr = address;
    gui_bineditor_bmp_view_addr( ((gui_bitmap_bmp_str *)be->priv->bmp) );
    gui_bineditor_bmp_redraw(  (gui_bitmap_bmp_str *)(be->priv->bmp) );
}

char gui_bineditor_bitmap_get_mode(GuiBineditor *be)
{
    if( be->priv->bmp == NULL) return FALSE;
    return ((gui_bitmap_bmp_str *)be->priv->bmp)->live;
}

/***********************************************
*            Private functions                 *
************************************************/
/*
    (============================================================================================)
    (===================================> INTERFACE <============================================)
    (============================================================================================)
*/

static void gui_bineditor_bmp_mask_generator(gui_bitmap_bmp_str *s )
{
    int i;

    // count '1' in mask
    for(s->bits  = 0, i = 1; i & 0xff; i <<= 1) 
	if( s->bit_mask & i )
	    s->masks[s->bits++] = i;
}

static void gui_bineditor_bmp_destroy(GtkWidget *wg, GuiBineditor *be)
{
    gtk_widget_set_sensitive( be->priv->bined, TRUE );
    free(be->priv->bmp);
    be->priv->bmp = NULL;
}

static void gui_bitmap_bmp_offs( gui_bitmap_bmp_str *s )
{

    if( s->bits )
	s->addr_shift = s->addr + s->val_multip * ((s->width / s->bits) * s->height + s->val_offset);

    gui_bineditor_bmp_view_addr( s );
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_tb_inverse(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    s->inverse = gtk_toggle_tool_button_get_active( GTK_TOGGLE_TOOL_BUTTON( wg ));
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_tb_mirror(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    s->mirror = gtk_toggle_tool_button_get_active( GTK_TOGGLE_TOOL_BUTTON( wg ));
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_tb_grid(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    s->grid = gtk_toggle_tool_button_get_active( GTK_TOGGLE_TOOL_BUTTON( wg ));
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_tb_live(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    s->live = gtk_toggle_tool_button_get_active( GTK_TOGGLE_TOOL_BUTTON( wg ));
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_tb_edit(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    s->edit = gtk_toggle_tool_button_get_active( GTK_TOGGLE_TOOL_BUTTON( wg ));
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_sb_multip(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    s->val_multip = gtk_spin_button_get_value(GTK_SPIN_BUTTON( wg ));
    gui_bitmap_bmp_offs( s );
}

static void gui_bineditor_bmp_sb_width(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    s->width = gtk_spin_button_get_value(GTK_SPIN_BUTTON( wg ));
    gui_bitmap_bmp_offs( s );
}

static void gui_bineditor_bmp_sb_height(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    s->height = gtk_spin_button_get_value(GTK_SPIN_BUTTON( wg ));
    gui_bitmap_bmp_offs( s );
}

static void gui_bineditor_bmp_sb_offset(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    s->val_offset = gtk_spin_button_get_value(GTK_SPIN_BUTTON( wg ));
    gui_bitmap_bmp_offs( s );
}

static void gui_bineditor_bmp_vch(GtkAdjustment *adj, gui_bitmap_bmp_str *s)
{
    unsigned int value = gtk_adjustment_get_value( adj );

    if( s->pixel == 0) return;
    s->off_y0 = value / s->pixel;
    s->off_y1 = value % s->pixel;
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_hch(GtkAdjustment *adj, gui_bitmap_bmp_str *s)
{
    unsigned int value = gtk_adjustment_get_value( adj );

    if( s->pixel == 0) return;
    s->off_x0 = value / s->pixel;    
    s->off_x1 = value % s->pixel;
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_button_press(GtkWidget *wg, GdkEventButton *ev, gui_bitmap_bmp_str *s )
{
    gui_bineditor_bmp_mouse_button( s, ev->button, 1, ev->x, ev->y);
}

static void gui_bineditor_bmp_button_release(GtkWidget *wg, GdkEventButton *ev, gui_bitmap_bmp_str *s )
{
    gui_bineditor_bmp_mouse_button( s, ev->button, 0, ev->x, ev->y);
}

static inline void gui_bineditor_bmp_toolbar_setup( gui_bitmap_bmp_str *s )
{
    GtkWidget *wg;

    // ===> Toolbar <===
    s->tb = gtk_toolbar_new();
    gtk_orientable_set_orientation( GTK_ORIENTABLE(s->tb), GTK_ORIENTATION_HORIZONTAL);
    gtk_toolbar_set_style(GTK_TOOLBAR(s->tb), GTK_TOOLBAR_ICONS);
    gtk_box_pack_start(GTK_BOX(s->tb_line), s->tb, TRUE, TRUE, 0);
    // Zoom out 
    s->w_zoom_out = gtk_tool_button_new_from_stock( GTK_STOCK_ZOOM_OUT );
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM( s->w_zoom_out ), TEXT(TIP_TB_ZOOM_OUT));
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), GTK_TOOL_ITEM(s->w_zoom_out), -1);
    // Zoom in
    s->w_zoom_in = gtk_tool_button_new_from_stock( GTK_STOCK_ZOOM_IN );
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM( s->w_zoom_in ), TEXT(TIP_TB_ZOOM_IN));
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), GTK_TOOL_ITEM(s->w_zoom_in), -1);
    // Zoom fit
    s->w_zoom_fit = gtk_tool_button_new_from_stock( GTK_STOCK_ZOOM_FIT );
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM( s->w_zoom_fit ), TEXT(TIP_TB_ZOOM_FIT));
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), GTK_TOOL_ITEM(s->w_zoom_fit), -1);
    // Separator
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), gtk_separator_tool_item_new(), -1);
    // Inverse on/off
    s->w_inverse = gtk_toggle_tool_button_new_from_stock( GUI_BINEDITOR_INVERSE_ICON );
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM( s->w_inverse ), TEXT(TIP_TB_INVERSE));
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), GTK_TOOL_ITEM(s->w_inverse), -1);
    // Mirror  on/off
    s->w_mirror = gtk_toggle_tool_button_new_from_stock( GUI_BINEDITOR_MIRROR_ICON );
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM( s->w_mirror ), TEXT(TIP_TB_MIRROR));
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), GTK_TOOL_ITEM(s->w_mirror), -1);
    gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( s->w_mirror ), s->mirror);
    // Grid    on/off
    s->w_grid = gtk_toggle_tool_button_new_from_stock( GUI_BINEDITOR_GRID_ICON );
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM( s->w_grid ), TEXT(TIP_TB_GRID));
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), GTK_TOOL_ITEM(s->w_grid), -1);
    gtk_toggle_tool_button_set_active( GTK_TOGGLE_TOOL_BUTTON( s->w_grid ), s->grid);
    // Separator
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), gtk_separator_tool_item_new(), -1);
    // Live    on/off
    s->w_live = gtk_toggle_tool_button_new_from_stock( GUI_BINEDITOR_LIVE_ICON );
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM( s->w_live ), TEXT(TIP_TB_LIVE));
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), GTK_TOOL_ITEM(s->w_live), -1);
    // edit    on/off
    s->w_edit = gtk_toggle_tool_button_new_from_stock( GUI_BINEDITOR_EDIT_ICON );
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM( s->w_edit ), TEXT(TIP_TB_EDIT));
    gtk_toolbar_insert(GTK_TOOLBAR( s->tb ), GTK_TOOL_ITEM(s->w_edit), -1);

    g_signal_connect(G_OBJECT(s->w_zoom_out), "clicked", G_CALLBACK( gui_bineditor_bmp_tb_zoom_out), s);
    g_signal_connect(G_OBJECT(s->w_zoom_in),  "clicked", G_CALLBACK( gui_bineditor_bmp_tb_zoom_in), s);
    g_signal_connect(G_OBJECT(s->w_zoom_fit), "clicked", G_CALLBACK( gui_bineditor_bmp_tb_zoom_fit ), s);
    g_signal_connect(G_OBJECT(s->w_inverse),  "toggled", G_CALLBACK( gui_bineditor_bmp_tb_inverse ), s);
    g_signal_connect(G_OBJECT(s->w_mirror),   "toggled", G_CALLBACK( gui_bineditor_bmp_tb_mirror ), s);
    g_signal_connect(G_OBJECT(s->w_grid),     "toggled", G_CALLBACK( gui_bineditor_bmp_tb_grid ), s);
    g_signal_connect(G_OBJECT(s->w_live),     "toggled", G_CALLBACK( gui_bineditor_bmp_tb_live ), s);
    g_signal_connect(G_OBJECT(s->w_edit),     "toggled", G_CALLBACK( gui_bineditor_bmp_tb_edit ), s);

    // Address info
    wg = gtk_frame_new(NULL);
    gtk_widget_set_size_request(wg, 230, 20);
    s->lbl_addr = gtk_label_new("--");
    gtk_container_add(GTK_CONTAINER( wg ), s->lbl_addr);
    gtk_box_pack_start(GTK_BOX(s->tb_line), wg, FALSE, FALSE, 3);
    gui_bineditor_bmp_view_addr( s );

}

static void gui_bineditor_bmp_parameter_setup(gui_bitmap_bmp_str *s)
{
    s->w_multip = gtk_spin_button_new_with_range(0, s->be->priv->buff->size, 1);
    s->w_width  = gtk_spin_button_new_with_range(0, s->be->priv->buff->size, 1);
    s->w_height = gtk_spin_button_new_with_range(0, s->be->priv->buff->size, 1);
    s->w_offset = gtk_spin_button_new_with_range(0, s->be->priv->buff->size, 1);
    gtk_box_pack_start( GTK_BOX(s->param_hbox), gtk_label_new(TEXT(LBL_SB_MULT)), FALSE, FALSE, 10);
    gtk_box_pack_start( GTK_BOX(s->param_hbox), s->w_multip, FALSE, FALSE, 0);
    gtk_box_pack_start( GTK_BOX(s->param_hbox), gtk_label_new(TEXT(LBL_SB_WIDTH)), FALSE, FALSE, 10);
    gtk_box_pack_start( GTK_BOX(s->param_hbox), s->w_width,  FALSE, FALSE, 0);
    gtk_box_pack_start( GTK_BOX(s->param_hbox), gtk_label_new(TEXT(LBL_SB_HEIGHT)), FALSE, FALSE, 10);
    gtk_box_pack_start( GTK_BOX(s->param_hbox), s->w_height, FALSE, FALSE, 0);
    gtk_box_pack_start( GTK_BOX(s->param_hbox), gtk_label_new(TEXT(LBL_SB_OFFSET)), FALSE, FALSE, 10);    
    gtk_box_pack_start( GTK_BOX(s->param_hbox), s->w_offset, FALSE, FALSE, 0);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->w_multip), 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->w_width),  s->width);    
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->w_height), s->height);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->w_offset), 0);

    g_signal_connect(G_OBJECT(s->w_multip), "value_changed", G_CALLBACK( gui_bineditor_bmp_sb_multip ), s);    
    g_signal_connect(G_OBJECT(s->w_width),  "value_changed", G_CALLBACK( gui_bineditor_bmp_sb_width ),  s);    
    g_signal_connect(G_OBJECT(s->w_height), "value_changed", G_CALLBACK( gui_bineditor_bmp_sb_height ), s);    
    g_signal_connect(G_OBJECT(s->w_offset), "value_changed", G_CALLBACK( gui_bineditor_bmp_sb_offset ), s);    
}

static inline void gui_bineditor_bmp_drawing_setup( gui_bitmap_bmp_str *s )
{
    gtk_widget_set_events(s->draw_area, GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK );
    gtk_widget_set_can_focus(s->draw_area, TRUE);
    g_signal_connect(G_OBJECT(s->draw_area), "draw", G_CALLBACK(gui_bineditor_bmp_draw_event), s);
    g_signal_connect(G_OBJECT(s->draw_area), "scroll_event", G_CALLBACK(gui_bineditor_bmp_scroll), s);
    g_signal_connect(G_OBJECT(s->draw_area), "button_press_event", G_CALLBACK(gui_bineditor_bmp_button_press), s);
    g_signal_connect(G_OBJECT(s->draw_area), "button_release_event", G_CALLBACK(gui_bineditor_bmp_button_release), s);
}

static void gui_bineditor_bmp_view_addr( gui_bitmap_bmp_str *s )
{
    char tmp[256];
    unsigned int x,y;
    x = s->addr_shift + s->addr;
    y = s->addr;
    sprintf( tmp, "ADDR: 0x%X%X%X%X%X%X%X%X -> 0x%X%X%X%X%X%X%X%X", 
	(y >> (4*7)) & 0xf,
	(y >> (4*6)) & 0xf,    
	(y >> (4*5)) & 0xf,
	(y >> (4*4)) & 0xf,
	(y >> (4*3)) & 0xf,
	(y >> (4*2)) & 0xf,
	(y >> (4*1)) & 0xf,
	(y >> (4*0)) & 0xf,
	(x >> (4*7)) & 0xf,
	(x >> (4*6)) & 0xf,    
	(x >> (4*5)) & 0xf,
	(x >> (4*4)) & 0xf,
	(x >> (4*3)) & 0xf,
	(x >> (4*2)) & 0xf,
	(x >> (4*1)) & 0xf,
	(x >> (4*0)) & 0xf
    );
    gtk_label_set_text(GTK_LABEL( s->lbl_addr), tmp);    
}

static gboolean gui_bineditor_bmp_draw_event(GtkWidget *wg, cairo_t *cr, gui_bitmap_bmp_str *s)
{
    s->cr = cr;
    s->da_width  = gtk_widget_get_allocated_width( wg );
    s->da_height = gtk_widget_get_allocated_height( wg );
    gui_bineditor_bmp_draw( s );
    return FALSE;
}

static void gui_bineditor_bmp_scrollers_actualization( gui_bitmap_bmp_str *s)
{
    gdouble tmp;
    int range_y, range_x;
    unsigned int val_y, val_x;

    gui_bineditor_bmp_draw_compute( s );
    range_y = s->wyy - s->da_height;
    if( range_y < 0) range_y = 0;

    range_x = s->wxx - s->da_width;
    if( range_x < 0) range_x = 0;

    tmp = gtk_adjustment_get_upper(s->vadj);
    if( tmp != 0)
	tmp = gtk_adjustment_get_value(s->vadj) / tmp;
    else
	tmp = 0;
    val_y = tmp * range_y;    

    tmp = gtk_adjustment_get_upper(s->hadj);
    if( tmp != 0)
	tmp = gtk_adjustment_get_value(s->hadj) / tmp;
    else
	tmp = 0;
    val_x = tmp * range_x;    
    
    if( s->pixel != 0){
	s->off_y0 = val_y / s->pixel;
	s->off_y1 = val_y % s->pixel;
	s->off_x0 = val_x / s->pixel;
	s->off_x1 = val_x % s->pixel;
    }
    if( range_y == 0)
	gtk_adjustment_configure(s->vadj, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0 );
    else
	gtk_adjustment_configure(s->vadj, val_y, 0.0, range_y, 1.0, 1.0, 1.0); // page size should be calculated --> to do

    if( range_x == 0)
	gtk_adjustment_configure(s->hadj, 0.0, 0.0, 1.0, 1.0, 1.0, 1 );
    else
	gtk_adjustment_configure(s->hadj, val_x, 0.0, range_x, 1.0, 1.0, 1.0);
}

void gui_bineditor_bmp_zoom_fit( gui_bitmap_bmp_str *s )
{
    unsigned int a, b;

    if((s->width == 0) || (s->height == 0)) return;
    a = s->width  ? s->da_width / s->width : 1;
    b = s->height ? s->da_height / s->height : 1;
    s->pixel = (a > b) ? b : a;
    s->cell_xx = s->width;
    s->cell_yy = s->height;
    s->wxx = s->pixel * s->cell_xx;
    s->wyy = s->pixel * s->cell_yy;
    // Center 
    s->wx0 = (s->da_width - s->wxx) / 2;
    s->wy0 = (s->da_height - s->wyy) / 2;
    
    gui_bineditor_bmp_scrollers_actualization( s );
    s->first_run = 0;
}

static void gui_bineditor_bmp_tb_zoom_fit(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    gui_bineditor_bmp_zoom_fit( s );
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_zoom_in( gui_bitmap_bmp_str *s )
{
    s->pixel++;
    if( s->pixel >= s->da_width)  s->pixel = s->da_width;
    if( s->pixel >= s->da_height) s->pixel = s->da_height;
    if( s->pixel >= s->da_width)  s->pixel = s->da_width;
    gui_bineditor_bmp_scrollers_actualization( s );
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_zoom_out( gui_bitmap_bmp_str *s )
{
    s->pixel--;
    if( s->pixel < 1) s->pixel = 1;
    gui_bineditor_bmp_scrollers_actualization( s );
    gui_bineditor_bmp_redraw( s );
}

static void gui_bineditor_bmp_tb_zoom_in(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    gui_bineditor_bmp_zoom_in( s );
}

static void gui_bineditor_bmp_tb_zoom_out(GtkWidget *wg, gui_bitmap_bmp_str *s )
{
    gui_bineditor_bmp_zoom_out( s );
}

static void gui_bineditor_bmp_scroll(GtkWidget *wg, GdkEventScroll *ev, gui_bitmap_bmp_str *s)
{
    switch( ev->direction ){
	case GDK_SCROLL_UP: gui_bineditor_bmp_zoom_in(s); break;
	case GDK_SCROLL_DOWN: gui_bineditor_bmp_zoom_out(s); break;
	default:;
    }
    gui_bineditor_bmp_scrollers_actualization( s );
}

/*  
    ==================================================================================================
    ..--------================================> DRAW <===============================------------..
    ==================================================================================================
*/

static inline void gui_bineditor_bmp_draw_pixel_lo( gui_bitmap_bmp_str *s, int x, int y)
{
    unsigned int pixelx, pixely;
    
    pixelx  = s->pixel;
    pixelx -= s->grid ? 2 : 0;
    pixely  = pixelx;

    x += s->grid ? 1 : 0;
    y += s->grid ? 1 : 0;

    cairo_set_source_rgb( s->cr, GET_COLOR(GUI_BINEDITOR_COLOR_BMP_PIXEL) ); // background color
    cairo_rectangle( s->cr, x, y, pixelx, pixely);
    cairo_fill( s->cr );
}

static inline unsigned char gui_bineditor_bmp_get_data( gui_bitmap_bmp_str *s, unsigned int index)
{
    if( index >= s->be->priv->buff->size) return 0;
    return s->be->priv->buff->data[index];
}

static inline char gui_bineditor_bmp_test_bit(gui_bitmap_bmp_str *s, int x, int y )
{
    unsigned char data;
    unsigned int kx = 0, idx = 0;

    y = y + s->off_y0;
    x = x + s->off_x0;

    if( s->bits == 0) return 0; // protect from div by 0
    
    kx = x + y * s->width;    // bit adress
    
    idx = kx % s->bits; // bit address
    kx  = kx / s->bits; // byte address

    data = gui_bineditor_bmp_get_data(s, s->addr + kx + s->addr_shift);
    if( s->mirror) idx = s->bits - idx - 1;
    if( idx < 0) return 0;
    data ^= s->inverse ? 0xff : 00;
    data &= s->masks[ idx ];
    return data;
}

static inline char gui_bineditor_bmp_draw_pixel( gui_bitmap_bmp_str *s, int x, int y)
{
    gui_bineditor_bmp_draw_pixel_lo(s, x * s->pixel + s->wx0 - s->off_x1, y * s->pixel + s->wy0 - s->off_y1);
    return 0;
}

static inline void gui_bineditor_bmp_draw_ctx( gui_bitmap_bmp_str *s)
{
    unsigned int x, y;
    
    for( y = 0; y < s->height; y++){
	for( x = 0; x < s->width; x++){    
	    if(gui_bineditor_bmp_test_bit(s, x, y))
		if(gui_bineditor_bmp_draw_pixel(s, x, y)) break;
	}
    }
}

static inline void gui_bineditor_bmp_draw_hline( cairo_t *cr, int x, int y, int len)
{
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x + len, y);
}

static inline void gui_bineditor_bmp_draw_vline( cairo_t *cr, int x, int y, int len)
{
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x, y + len);
}

static inline void gui_bineditor_bmp_draw_grid( gui_bitmap_bmp_str *s)
{
    int i, n;

    if( !s->grid ) return;
    cairo_set_source_rgb( s->cr, GET_COLOR(GUI_BINEDITOR_COLOR_BMP_GRID) ); 
    for(i = 0, n = s->wx0; i < s->width; i++, n += s->pixel) 
	gui_bineditor_bmp_draw_vline( s->cr, n - s->off_x1, s->wy0 - s->off_y1, s->wyy);
    
    for(i = 0, n = s->wy0; i < s->height; i++, n += s->pixel)
	gui_bineditor_bmp_draw_hline( s->cr, s->wx0 - s->off_x1, n - s->off_y1, s->wxx);

    cairo_stroke( s->cr );        
}

static inline void gui_bineditor_bmp_redraw( gui_bitmap_bmp_str *s )
{
    gtk_widget_queue_draw( s->draw_area );
}

static void gui_bineditor_bmp_draw(gui_bitmap_bmp_str *s)
{
    if(s->first_run) gui_bineditor_bmp_zoom_fit( s );
    gui_bineditor_bmp_draw_compute( s );
    gui_bineditor_bmp_draw_ambient( s );
    gui_bineditor_bmp_draw_background( s );

    gui_bineditor_bmp_draw_grid( s );
    gui_bineditor_bmp_draw_ctx( s );
}

static inline void gui_bineditor_bmp_draw_ambient( gui_bitmap_bmp_str *s)
{
    cairo_set_source_rgb( s->cr, GET_COLOR(GUI_BINEDITOR_COLOR_BMP_AMBIENT) ); // background color
    cairo_rectangle( s->cr, 0, 0, s->da_width, s->da_height );
    cairo_fill( s->cr );
}

static inline void gui_bineditor_bmp_draw_background( gui_bitmap_bmp_str *s )
{
    cairo_set_source_rgb( s->cr, GET_COLOR(GUI_BINEDITOR_COLOR_BMP_BG) ); // background color
    cairo_rectangle( s->cr, s->wx0, s->wy0, s->wxx, s->wyy );
    cairo_fill( s->cr );
}

static void gui_bineditor_bmp_draw_compute(gui_bitmap_bmp_str *s)
{
    if(!s->pixel) return;
    s->first_run = 0;

    s->cell_xx = s->width;
    s->cell_yy = s->height;

    // width and high of the displayed pole in pixels
    s->wxx = s->pixel * s->width;
    s->wyy = s->pixel * s->height;

    // Center 
    s->wx0 = (s->da_width - s->wxx) / 2;
    s->wy0 = (s->da_height - s->wyy) / 2;

    if( s->pixel != 0){
	if( s->wxx >= s->da_width){
	    s->cell_xx = s->da_width / s->pixel;
	    s->cell_xx += (s->da_width % s->pixel) ? 1:0;
	}
	if( s->wyy >= s->da_height){
	    s->cell_yy = s->da_height / s->pixel;
	    s->cell_yy += (s->da_height % s->pixel) ? 1:0;
	}

	if(s->wyy > s->da_height) s->wy0 = 0;
	if(s->wxx > s->da_width) s->wx0 = 0;
    }
}

static char gui_bineditor_bmp_grid_coordinates(gui_bitmap_bmp_str *s, unsigned int x, unsigned int y)
{
    int rx, ry;

    s->mx = 0;
    s->my = 0;
    rx = x - s->wx0;
    ry = y - s->wy0;
    if(( rx < 0) || ( ry < 0) || (rx >= s->wxx) || (ry >= s->wyy)) return 1;
    if(s->pixel != 0){
	s->mx = (rx + s->off_x1) / s->pixel;
	s->my = (ry + s->off_y1) / s->pixel;
    }
    return 0;
}

static inline void gui_bineditor_bmp_set_data( gui_bitmap_bmp_str *s, unsigned int index, unsigned char data)
{
    if( index >= s->be->priv->buff->size) return;
    s->be->priv->buff->data[index] = data;
}

static inline void gui_bineditor_bmp_set_grid_bit(gui_bitmap_bmp_str *s, int x, int y, char op )
{
    unsigned char data, tmp;
    unsigned int kx = 0, idx = 0, ad;

    y += s->off_y0;
    x += s->off_x0;

    if( s->bits == 0) return; // protect from div by 0
    
    kx = x + y * s->width;    // bit adress
    
    idx = kx % s->bits; // bit address
    kx  = kx / s->bits; // byte address

    ad = s->addr + kx + s->addr_shift;
    data = gui_bineditor_bmp_get_data(s, ad);
    
    if( s->mirror) idx = s->bits - idx - 1;
    if( idx < 0) return;

    tmp = s->masks[ idx ];

    switch( op ){
	case 0: data |= tmp;  break;	// set pixel
	case 1: data &= ~tmp; break;	// clear bit
	case 2: data ^= tmp;  break;	// change bit to oposite
    }
    gui_bineditor_buff_history_add(s->be->priv->buff, ad, ad);
    gui_bineditor_bmp_set_data(s, ad, data);
}

static void gui_bineditor_bmp_mouse_button( gui_bitmap_bmp_str *s, int button, char state, unsigned int x, unsigned int y )
{
    if( !s->edit ) return;
    if( gui_bineditor_bmp_grid_coordinates(s, x, y) ) return;

    switch( button ){
	case 1 : gui_bineditor_bmp_set_grid_bit(s, s->mx, s->my, 0 ); break;
	case 3 : gui_bineditor_bmp_set_grid_bit(s, s->mx, s->my, 1 ); break;
    }
    gui_bineditor_bmp_redraw( s );
    gui_bineditor_redraw( s->be );
}

void gui_bineditor_bitmap_redraw(GuiBineditor *be)
{
    if(be->priv->bmp == NULL) return;
    gui_bineditor_bmp_redraw(  (gui_bitmap_bmp_str *)(be->priv->bmp) );
}

