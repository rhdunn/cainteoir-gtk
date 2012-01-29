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
	NavigationBar *parent;
	GtkNotebook *view;
	int page;
};

static void on_navbutton_toggled(GtkWidget *button, void *data)
{
	NavButtonData *navbutton = (NavButtonData *)data;
	if (navbutton->parent->set_active_button(button))
		gtk_notebook_set_current_page(navbutton->view, navbutton->page);
}

NavigationBar::NavigationBar()
	: active_button(NULL)
	, setting_active_button(false)
{
	layout = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_name(layout, "navbar");
}

GtkWidget *NavigationBar::add_paged_button(const char *label, GtkNotebook *view, int page)
{
	NavButtonData *data = g_slice_new(NavButtonData);
	data->parent = this;
	data->view = view;
	data->page = page;

	GtkWidget *button = gtk_toggle_button_new_with_label(label);
	g_signal_connect(button, "toggled", G_CALLBACK(on_navbutton_toggled), data);

	gtk_box_pack_start(GTK_BOX(layout), button, FALSE, FALSE, 0);
	return button;
}

GtkWidget *NavigationBar::get_active_button() const
{
	return active_button;
}

bool NavigationBar::set_active_button(GtkWidget *button)
{
	//g_return_if_fail(button != NULL);
	//g_return_if_fail(GTK_IS_TOGGLE_BUTTON(button));

	if (setting_active_button)
		return false;

	setting_active_button = true;

	if (active_button && button != active_button)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(active_button), FALSE);

	active_button = button;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(active_button)) == FALSE)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(active_button), TRUE);

	setting_active_button = false;
	return true;
}
