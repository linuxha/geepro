/* $Revision: 1.5 $ */
/* hex, binary viewer, editor, kontrolka GTK
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

/*
    Kontrolka emituje sygnal "changed" jesli zawartosc bufora ulegnie zmianie
*/

#ifndef __GUI_BINEDITOR_H__
#define __GUI_BINEDITOR_H__
#include <gtk/gtk.h>

#include "be_buffer.h"


G_BEGIN_DECLS

#define GUI_TYPE_BINEDITOR		( gui_bineditor_get_type() )
#define GUI_BINEDITOR(obj)		( G_TYPE_CHECK_INSTANCE_CAST((obj), GUI_TYPE_BINEDITOR, GuiBineditor) )
#define GUI_BINEDITOR_CLASS(klass)	( G_TYPE_CHECK_CLASS_CAST((klass), GUI_TYPE_BINEDITOR, GuiBineditorClass) )
#define GUI_IS_BINEDITOR(obj)		( G_TYPE_CHECK_INSTANCE_TYPE((obj), GUI_TYPE_BINEDITOR) )
#define GUI_IS_BINEDITOR_CLASS(klass)	( G_TYPE_CHECK_CLASS_TYPE((klass), GUI_BINEDITOR) )
#define GUI_BINEDITOR_GET_CLASS(obj)	( G_TYPE_INSTANCE_GET_CLASS((obj), GUI_BINEDITOR, GuiBineditorClass) )

#define GUI_BINEDITOR_COLOR_R( var, id ) ( (var)->priv->colors[(id) * 3 + 0])
#define GUI_BINEDITOR_COLOR_G( var, id ) ( (var)->priv->colors[(id) * 3 + 1])
#define GUI_BINEDITOR_COLOR_B( var, id ) ( (var)->priv->colors[(id) * 3 + 2])

typedef struct _GuiBineditor 		GuiBineditor;
typedef struct _GuiBineditorPrivate	GuiBineditorPrivate;
typedef struct _GuiBineditorClass 	GuiBineditorClass;

typedef enum
{
    GUI_BINEDITOR_MARKER_CURSOR = 0,
    GUI_BINEDITOR_MARKER_SELECTED,
    GUI_BINEDITOR_MARKER_FOUND,
    GUI_BINEDITOR_MARKER_ALL
} GuiBineditorMarkerType;

typedef enum
{
    GUI_BINEDITOR_PROP_TOOLBAR = 1 << 0,
    GUI_BINEDITOR_PROP_CLEAR   = 1 << 1,
    GUI_BINEDITOR_PROP_FIND    = 1 << 2,
    GUI_BINEDITOR_PROP_MJMP    = 1 << 3,
    GUI_BINEDITOR_PROP_RJMP    = 1 << 4, 
    GUI_BINEDITOR_PROP_CHKSUM  = 1 << 5,
    GUI_BINEDITOR_PROP_CALC    = 1 << 6,
    GUI_BINEDITOR_PROP_PRINT   = 1 << 7,
    GUI_BINEDITOR_PROP_EDITABLE = 1 << 8,
    GUI_BINEDITOR_PROP_CHILD   = 1 << 9,
} GuiBineditorProperties;

typedef enum
{
    GUI_BINEDITOR_COLOR_TEXT_NORMAL = 0,/* text */
    GUI_BINEDITOR_COLOR_TEXT_MARKED,	/* text on marker position */
    GUI_BINEDITOR_COLOR_MARKED_BG,	/* text background on marker position */
    GUI_BINEDITOR_COLOR_TEXT_DESC,	/* grid description */
    GUI_BINEDITOR_COLOR_GRID,		/* grid */
    GUI_BINEDITOR_COLOR_GRID_BG,	/* grid background */
    GUI_BINEDITOR_COLOR_DESC_BG,	/* description background */
    GUI_BINEDITOR_COLOR_UD,		/* color of the solid block drawed on character over range */
    GUI_BINEDITOR_COLOR_HL,		/* highlight color */
    GUI_BINEDITOR_COLOR_HL_MRK,		/* highlight color on marker */
    GUI_BINEDITOR_COLOR_MARKER_FG_FOUND,
    GUI_BINEDITOR_COLOR_MARKER_BG_FOUND,
    GUI_BINEDITOR_COLOR_BMP_BG,		// background color in bitmap editor
    GUI_BINEDITOR_COLOR_BMP_GRID,	// 
    GUI_BINEDITOR_COLOR_BMP_PIXEL,	// 
    GUI_BINEDITOR_COLOR_BMP_AMBIENT,	// 
    GUI_BINEDITOR_COLOR_TXT_AMBIENT,	//     
    GUI_BINEDITOR_COLOR_TXT_FONT,	//     
    GUI_BINEDITOR_COLOR_LAST
} GuiBineditorColors;

enum
{
    GUI_BINEDITOR_COLOR_BG_BR = 0,	// background colors
    GUI_BINEDITOR_COLOR_BG_BG,
    GUI_BINEDITOR_COLOR_BG_BB,
    GUI_BINEDITOR_COLOR_BG_FR,		// foreground colors
    GUI_BINEDITOR_COLOR_BG_FG,
    GUI_BINEDITOR_COLOR_BG_FB,
    GUI_BINEDITOR_COLOR_SIZE
} GuiBineditorColorsSet;

typedef enum
{
    GUI_BINEDITOR_MARKER_OFF	= 0,
    GUI_BINEDITOR_MARKER_HEX	= 1 << 0,    
    GUI_BINEDITOR_MARKER_ASCII	= 1 << 1,
} GuiBineditorMarkerMode;

typedef struct 
{
    unsigned char	active;
    GuiBineditorMarkerMode mode;	// 0 - off, 1- ascii, 2 - hex
    unsigned int 	from;
    unsigned int 	to;
    float		colors[ GUI_BINEDITOR_COLOR_SIZE ];
} GuiBineditorMarkerItem;

typedef struct 
{
    unsigned int count;
    float br, bg, bb, fr, fg, fb;
    char mode, hit;
    GuiBineditorMarkerItem *items;
} GuiBineditorMarker;

typedef struct
{
    unsigned char *data;
    unsigned int size;    
} GuiBineditorCut;

struct _GuiBineditorPrivate
{
    const char	 **icon;
    GuiBineditor *root, *aux_ed;
    GuiBineditorCut cut_data;
    GuiBineditorMarker markers;
    gui_be_buffer_str *buff;
    void *texted_str;
    GdkAtom atom;
    GtkClipboard *clipb;
    /* key values */
    int key_left;
    int key_right;
    int key_up;
    int key_down;
    int key_home;
    int key_end;
    int key_pgup;
    int key_pgdn;
    int key_tab;
    int key_ctrl;
    /* */
    float colors[GUI_BINEDITOR_COLOR_LAST * 3];
    GtkWidget *wmain;
    GtkWidget *drawing_area;
    GtkWidget *tb;
    GtkWidget *tbv;
    char *core_name;
    unsigned int core_count;
    char *vfind;
    char *vreplace;
    unsigned int sum;
    char rfsh;     /* jesli 0 to przerysowanie calosci */
    int cell_width;
    int cell_height;
    int grid_start;
    int grid_end;
    int grid_cols;
    int grid_rows;
    int grid_top;
    int ascii_start;
    int ascii_end;
    int ascii_space;
    int address_mark;
    int address_mark_redo;
    int address_old_hint;    
//    int address_hl_start;	// to delete
//    int address_hl_end;		// to delete
    int properties;
    unsigned int aux_size;
    int pagesize;
    void  *stencil;
    GtkWidget *clear;
    GtkWidget *texted;
    GtkWidget *cut;
    GtkWidget *copy;
    GtkWidget *find;
    GtkWidget *mjmp;
    GtkWidget *rjmp;
    GtkWidget *bined;
    GtkWidget *calc;
    GtkWidget *statusbar;
    GtkWidget *vbox;
    GtkWidget *manipulator;
    GtkWidget *stenc;
//    GtkWidget *exec;
    GtkWidget *aux;
    GtkWidget *aux_win;
    GtkWidget *resize;    
    GtkWidget *organizer;    
    GtkWidget *checksum;    
    GtkWidget *asmview;    
    GtkWidget *i_open;
    GtkWidget *i_write;
    GtkWidget *i_undo;
    GtkWidget *i_redo;
    char *aux_buffer;
    int statusbar_id;
    /* hex editor */
    char edit_hex;		  // flag: bit0 -> hex grid, bit 1 -> ascii grid
    int edit_hex_cursor; // cursor position in hex grid
    int edit_addr_cursor;
    GtkWidget *info_addr; 
    GtkWidget *info_mark;
// clipboard marking
    gint     clpb;
    gboolean clpb_ascii;
    gboolean clpb_selected;
    unsigned long clpb_start;	// selected begin
    unsigned long clpb_end;	// selected end
    GtkAdjustment  *adj;
    void *bmp;
    void *user_ptr1;
    void *user_ptr2;    
};

struct _GuiBineditor
{
    /*<private>*/
    GtkBox parent;
    GuiBineditorPrivate *priv;
};

struct _GuiBineditorClass
{
    GtkBoxClass parent_class;

    /* Padding for future expansion */
    void (*bineditor) (GuiBineditor *be);
};

GType hui_bineditor_get_type	(void) G_GNUC_CONST;
GtkWidget *gui_bineditor_new(GtkWindow *parent);
void gui_bineditor_set_icon(GuiBineditor *be, const char **xpm_data);
void gui_bineditor_set_buffer(GuiBineditor *be, int bfsize, unsigned char *buffer);
void gui_bineditor_set_properties(GuiBineditor *be, GuiBineditorProperties prop);
void gui_bineditor_set_colors(GuiBineditor *be, GuiBineditorColors color, float r, float g, float b);
void gui_bineditor_redraw(GuiBineditor *be);
void gui_bineditor_connect_statusbar(GuiBineditor *be, GtkWidget *sb);
void gui_bineditor_show_grid(GuiBineditor *be, unsigned int from, unsigned int to);
GType gui_bineditor_get_type(void);
/* marker */
void gui_bineditor_marker_set_color(GuiBineditor *be, GuiBineditorMarkerType id, GuiBineditorColors foreground, GuiBineditorColors background);
void gui_bineditor_marker_set_item(GuiBineditor *be, GuiBineditorMarkerType id, GuiBineditorMarkerMode mode, unsigned int from, unsigned int to);
void gui_bineditor_marker_set_value(GuiBineditor *be, GuiBineditorMarkerType id, unsigned int from, unsigned int to);
void gui_bineditor_marker_show_item(GuiBineditor *be, GuiBineditorMarkerType id, char show);
void gui_bineditor_marker_unset_item(GuiBineditor *be, GuiBineditorMarkerType id);
void gui_bineditor_marker_new(GuiBineditor *be, unsigned int count);
char gui_bineditor_marker_get_range( GuiBineditor *be, GuiBineditorMarkerType id, unsigned int *from, unsigned int *to); // return true if visible
void gui_bineditor_marker_free(GuiBineditor *be);
/*  Gets mouse selected region range. Return True if region is selected */
char gui_bineditor_selection_get_range( GuiBineditor *be, unsigned int *from, unsigned int *to);
unsigned int gui_bineditor_grid_get_page_size( GuiBineditor *be );
/* Cut & paste*/
GuiBineditor *gui_bineditor_get_root( GuiBineditor *be); // return initial bineditor object
void gui_bineditor_cut_store(GuiBineditor *be, unsigned int from, unsigned int to);
void gui_bineditor_cut_restore(GuiBineditor *be, unsigned int from);
void gui_bineditor_file_tool_insert(GuiBineditor *be);
G_END_DECLS

#endif 

