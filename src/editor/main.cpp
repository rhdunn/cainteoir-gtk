/* Cainteoir Editor Application.
 *
 * Copyright (C) 2014 Reece H. Dunn
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
#include <cainteoir-gtk/cainteoir_audio_data.h>

static void
on_window_destroy(GtkWidget *object, gpointer data)
{
	gtk_main_quit();
}

int
main(int argc, char ** argv)
{
	gtk_init(&argc, &argv);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

#ifdef HAVE_GTK3_HEADER_BAR
	GtkWidget *header = gtk_header_bar_new();
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), i18n("Cainteoir Text-to-Speech Editor"));
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_window_set_titlebar(GTK_WINDOW(window), header);
#else
	gtk_window_set_title(GTK_WINDOW(window), i18n("Cainteoir Text-to-Speech Editor"));
#endif

	g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), nullptr);

	if (argc == 2)
	{
		CainteoirAudioData *audio = cainteoir_audio_data_new(argv[1]);
		if (audio)
		{
			g_object_unref(audio);
		}
	}

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}
