/* Cainteoir Reader Application.
 *
 * Copyright (C) 2015 Reece H. Dunn
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

#include "config.h"
#include "i18n.h"

#include <gtk/gtk.h>
#include "reader_window.h"
#include <cainteoir-gtk/cainteoir_document_view.h>

static void
on_window_destroy(GtkWidget *object, gpointer data)
{
	gtk_main_quit();
}

int
main(int argc, char ** argv)
{
	gtk_init(&argc, &argv);

	GtkWidget *window = reader_window_new((argc == 2) ? argv[1] : nullptr);
	g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), nullptr);

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}
