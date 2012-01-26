/* Gtkmm 3 and 3 compatibility helpers.
 *
 * Copyright (C) 2011 Reece H. Dunn
 *
 * This file is part of cainteoir-gtk.
 *
 * cainteoir-gtk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cainteoir-gtk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cainteoir-gtk.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <stdint.h>
#include "gtk-compatibility.hpp"

#if !GTK_CHECK_VERSION(3, 0, 0)
void gtk_widget_set_hexpand(GtkWidget *widget, gboolean expand)
{
	g_object_set_data(G_OBJECT(widget), "hexpand", (gpointer)(uintptr_t)expand);
}

gboolean gtk_widget_get_hexpand(GtkWidget *widget)
{
	return g_object_get_data(G_OBJECT(widget), "hexpand") != NULL;
}

void gtk_widget_set_valign(GtkWidget *widget, GtkAlign align)
{
}

void gtk_progress_bar_set_show_text(GtkProgressBar *, gboolean)
{
}

GtkWidget *gtk_box_new(GtkOrientation orientation, gint spacing)
{
	GtkWidget *widget = (orientation == GTK_ORIENTATION_HORIZONTAL) ? gtk_hbox_new(FALSE, spacing) : gtk_vbox_new(FALSE, spacing);
	return widget;
}

GtkWidget *gtk_button_box_new(GtkOrientation orientation)
{
	GtkWidget *widget = (orientation == GTK_ORIENTATION_HORIZONTAL) ? gtk_hbutton_box_new() : gtk_vbutton_box_new();
	return widget;
}

GtkWidget *gtk_scale_new(GtkOrientation orientation, GtkAdjustment *adjustment)
{
	GtkWidget *widget = (orientation == GTK_ORIENTATION_HORIZONTAL) ? gtk_hscale_new(adjustment) : gtk_vscale_new(adjustment);
	return widget;
}

GtkWidget *gtk_grid_new()
{
	return gtk_table_new(0, 0, FALSE);
}

void gtk_grid_attach(GtkGrid *grid, GtkWidget *child, gint left, gint top, gint width, gint height)
{
	guint w;
	guint h;

	gtk_table_get_size(grid, &w, &h);
	if (w < left+width || h < top+height)
		gtk_table_resize(grid, left+width, top+height);

	GtkAttachOptions hattach = gtk_widget_get_hexpand(child) ? GtkAttachOptions(GTK_FILL|GTK_EXPAND) : GTK_FILL;

	gtk_table_attach(grid, child, left, left+width, top, top+height, hattach, GTK_FILL, 4, 4);
}

void gtk_grid_set_row_spacing(GtkGrid *grid, guint spacing)
{
}

void gtk_grid_set_column_spacing(GtkGrid *grid, guint spacing)
{
}
#endif

#if !GTK_CHECK_VERSION(3, 4, 0)
void gtk_window_set_hide_titlebar_when_maximized(GtkWindow *, gboolean)
{
}
#endif
