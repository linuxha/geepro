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

#ifndef __BE_SCRIPT_H__
#define __BE_SCRIPT_H__

enum{
    BE_MODE_STRING,
    BE_MODE_REGEX,
    BE_MODE_HEX,
    BE_MODE_BINARY,
    BE_MODE_BEGIN,
    BE_MODE_CURSOR,
    BE_MODE_MARKED,
};

void gui_bineditor_find_string(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_manipulator(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_stencil(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_execute(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_aux(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_bined(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_clear_buffer(GtkWidget *bt, GuiBineditor *be);
void gui_bineditor_hex2ascii(GuiBineditor *be, gchar *txt);
void gui_bineditor_organizer(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_resize(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_checksum(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_asmview(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_cut(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_copy(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_texted(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_open(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_write(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_undo(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_redo(GtkWidget *wg, GuiBineditor *be);
void gui_bineditor_stencil_update(GuiBineditor *be);
#endif
