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
#include "be_bitmap.h"

#define GET_COLOR( idx ) s->be->priv->colors[(idx)*3 + 0],s->be->priv->colors[(idx)*3 + 1],s->be->priv->colors[(idx)*3 + 2]

typedef struct
{
    GuiBineditor *be;	// parent
    GtkWidget *wmain;
    GtkToolItem *w_zoom_out, *w_zoom_in, *w_zoom_fit;
    GtkToolItem *w_inverse, *w_mirror, *w_grid;
    GtkToolItem *w_live, *w_edit;
    GtkWidget *ctx, *tb, *draw_area, *param_hbox, *info_hbox, *tb_line, *lbl_addr;
    GtkWidget *vslid, *hslid, *w_width, *w_height, *w_offset, *w_multip; // scrollbars
    GtkAdjustment *vadj, *hadj; // scrollbars adjustments
    unsigned int width, height, addr, addr_shift;
    cairo_t *cr;
    unsigned int da_width, da_height; // drawing area size in pixels
    unsigned int wx0,wy0, wxx, wyy;   // position and size of 'window' in pixels 
    unsigned int pixel;		      // 'pixel' size in pixels
    unsigned char bit_mask;
    unsigned char masks[8];
    unsigned char bits;
    unsigned int cell_xx, cell_yy;   // cols and rows of displayed grid
    unsigned int off_x0, off_y0;
    unsigned int off_x1, off_y1;
    char first_run:1;
    char inverse:1; 	// view '1' as white, and '0' as black
    char mirror:1;  	// reverse bit order
    char grid:1;	// display grid
    char live:1;	// address actualisation on fly
    char edit:1;	// permision to edit
    unsigned int val_multip;
    unsigned int val_offset;
    unsigned int mx, my;
} gui_bitmap_bmp_str;

static inline void gui_bineditor_bmp_draw_ambient( gui_bitmap_bmp_str *);
static inline void gui_bineditor_bmp_toolbar_setup(gui_bitmap_bmp_str *);
static void gui_bineditor_bmp_destroy(GtkWidget *, GuiBineditor *);
static void gui_bineditor_bmp_parameter_setup(gui_bitmap_bmp_str *);
static void gui_bineditor_bmp_view_addr( gui_bitmap_bmp_str *);
static gboolean gui_bineditor_bmp_draw_event(GtkWidget *, cairo_t *, gui_bitmap_bmp_str *);
static inline void gui_bineditor_bmp_drawing_setup( gui_bitmap_bmp_str *);
static void gui_bineditor_bmp_draw(gui_bitmap_bmp_str *);
static inline void gui_bineditor_bmp_draw_background( gui_bitmap_bmp_str * );
static void gui_bineditor_bmp_draw_compute(gui_bitmap_bmp_str *);
void gui_bineditor_bmp_zoom_fit( gui_bitmap_bmp_str * );
static inline void gui_bineditor_bmp_draw_grid( gui_bitmap_bmp_str *);
static void gui_bineditor_bmp_tb_zoom_fit(GtkWidget *, gui_bitmap_bmp_str *);
static void gui_bineditor_bmp_tb_zoom_in(GtkWidget *, gui_bitmap_bmp_str * );
static void gui_bineditor_bmp_tb_zoom_out(GtkWidget *, gui_bitmap_bmp_str * );
static void gui_bineditor_bmp_scroll(GtkWidget *, GdkEventScroll *, gui_bitmap_bmp_str *);
static inline void gui_bineditor_bmp_redraw( gui_bitmap_bmp_str * );
static void gui_bineditor_bmp_mask_generator(gui_bitmap_bmp_str * );
static inline void gui_bineditor_bmp_draw_ctx( gui_bitmap_bmp_str *);
static void gui_bineditor_bmp_vch(GtkAdjustment *, gui_bitmap_bmp_str *);
static void gui_bineditor_bmp_hch(GtkAdjustment *, gui_bitmap_bmp_str *);
static void gui_bineditor_bmp_mouse_button( gui_bitmap_bmp_str *, int, char, unsigned int, unsigned int );

