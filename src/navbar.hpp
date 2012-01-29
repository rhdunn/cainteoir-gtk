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

#ifndef CAINTEOIRGTK_SRC_NAVBAR_HPP
#define CAINTEOIRGTK_SRC_NAVBAR_HPP

class NavigationBar
{
public:
	NavigationBar();

	operator GtkWidget *() { return layout; }

	GtkWidget *add_paged_button(const char *label, GtkNotebook *view, int page);

	GtkWidget *get_active_button() const;

	bool set_active_button(GtkWidget *button);
private:
	GtkWidget *layout;
	GtkWidget *active_button;
	bool setting_active_button;
};

#endif
