/* Navigation Bar
 *
 * Copyright (C) 2012 Reece H. Dunn
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

#include <config.h>
#include <gtk/gtk.h>
#include <cainteoir/platform.hpp>

#include "navbar.hpp"

struct NavButtonData
{
	GtkNotebook *view;
	int page;
};

static void on_navbutton_clicked(GtkTreeViewColumn *column, void *data)
{
	NavButtonData *navbutton = (NavButtonData *)data;
	gtk_notebook_set_current_page(navbutton->view, navbutton->page);
}

NavigationBar::NavigationBar()
{
	layout = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_name(layout, "navbar");
}

GtkWidget *NavigationBar::add_paged_button(const char *label, GtkNotebook *view, int page)
{
	NavButtonData *data = g_slice_new(NavButtonData);
	data->view = view;
	data->page = page;

	GtkWidget *button = gtk_button_new_with_label(label);
	g_signal_connect(button, "clicked", G_CALLBACK(on_navbutton_clicked), data);

	gtk_box_pack_start(GTK_BOX(layout), button, FALSE, FALSE, 0);
	return button;
}
