/* Gtkmm 2 and 3 compatibility helpers.
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

#ifndef CAINTEOIRGTK_SRC_GTK_COMPATIBILITY_HPP
#define CAINTEOIRGTK_SRC_GTK_COMPATIBILITY_HPP

#if !GTK_CHECK_VERSION(3, 0, 0)
typedef enum
{
	GTK_ALIGN_FILL,
	GTK_ALIGN_START,
	GTK_ALIGN_END,
	GTK_ALIGN_CENTER
} GtkAlign;

void gtk_widget_set_hexpand(GtkWidget *widget, gboolean expand);

gboolean gtk_widget_get_hexpand(GtkWidget *widget);

void gtk_widget_set_valign(GtkWidget *widget, GtkAlign align);

void gtk_progress_bar_set_show_text(GtkProgressBar *, gboolean);

GtkWidget *gtk_box_new(GtkOrientation orientation, gint spacing);

GtkWidget *gtk_button_box_new(GtkOrientation orientation);

GtkWidget *gtk_scale_new(GtkOrientation orientation, GtkAdjustment *adjustment);

typedef GtkTable GtkGrid;
#define GTK_GRID(widget) GTK_TABLE(widget)

GtkWidget *gtk_grid_new();

void gtk_grid_attach(GtkGrid *grid, GtkWidget *child, gint left, gint top, gint width, gint height);

void gtk_grid_set_row_spacing(GtkGrid *grid, guint spacing);

void gtk_grid_set_column_spacing(GtkGrid *grid, guint spacing);
#endif

#if !GTK_CHECK_VERSION(3, 4, 0)
void gtk_window_set_hide_titlebar_when_maximized(GtkWindow *, gboolean);
#endif

#endif
