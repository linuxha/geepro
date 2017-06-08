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

#ifndef __BE_BITMAP_H__
#define __BE_BITMAP_H__

#include "bineditor.h"

#define GUI_BINEDITOR_INVERSE_ICON	"geepro-bineditor-inverse"
#define GUI_BINEDITOR_MIRROR_ICON	"geepro-bineditor-mirror"
#define GUI_BINEDITOR_GRID_ICON		"geepro-bineditor-grid"
#define GUI_BINEDITOR_LIVE_ICON		"geepro-bineditor-live"
#define GUI_BINEDITOR_EDIT_ICON		"geepro-bineditor-edit"

extern void gui_bineditor_bitmap(GuiBineditor *be, unsigned int width, unsigned int height, unsigned char mask, unsigned char bit_rev);
extern void gui_bineditor_bitmap_set_address(GuiBineditor *be, unsigned int addr);
extern char gui_bineditor_bitmap_get_mode(GuiBineditor *be);
extern void gui_bineditor_bitmap_redraw(GuiBineditor *be);
#endif // __BE_BITMAP_H__
