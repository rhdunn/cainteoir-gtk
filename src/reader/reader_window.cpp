/* The Cainteoir Reader main window.
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
#include <cainteoir-gtk/cainteoir_document_index.h>
#include <cainteoir-gtk/cainteoir_settings.h>

struct _ReaderWindowPrivate
{
	GtkWidget *index;
	GtkWidget *view;

	CainteoirSettings *settings;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderWindow, reader_window, GTK_TYPE_WINDOW)

static void
reader_window_finalize(GObject *object)
{
	ReaderWindow *reader = READER_WINDOW(object);

	G_OBJECT_CLASS(reader_window_parent_class)->finalize(object);
}

static void
reader_window_class_init(ReaderWindowClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = reader_window_finalize;
}

static void
reader_window_init(ReaderWindow *reader)
{
	reader->priv = (ReaderWindowPrivate *)reader_window_get_instance_private(reader);
	reader->priv->settings = cainteoir_settings_new("settings.dat");
}

static gboolean
on_window_state_changed(GtkWidget *widget, GdkEvent *event, void *data)
{
	CainteoirSettings *settings = (CainteoirSettings *)data;
	gboolean maximized = (((GdkEventWindowState *)event)->new_window_state & GDK_WINDOW_STATE_MAXIMIZED);
	cainteoir_settings_set_boolean(settings, "window", "maximized", maximized);
	cainteoir_settings_save(settings);
	return TRUE;
}

static gboolean
on_window_delete(GtkWidget *window, GdkEvent *event, gpointer data)
{
	ReaderWindow *reader = (ReaderWindow *)data;
	if (!cainteoir_settings_get_boolean(reader->priv->settings, "window", "maximized", FALSE))
	{
		gint width = 0;
		gint height = 0;
		gint top = 0;
		gint left = 0;

		gtk_window_get_position(GTK_WINDOW(reader), &left, &top);
		gtk_window_get_size(GTK_WINDOW(reader), &width, &height);

		cainteoir_settings_set_integer(reader->priv->settings, "window", "width",  width);
		cainteoir_settings_set_integer(reader->priv->settings, "window", "height", height);
		cainteoir_settings_set_integer(reader->priv->settings, "window", "top",    top);
		cainteoir_settings_set_integer(reader->priv->settings, "window", "left",   left);
	}
	cainteoir_settings_save(reader->priv->settings);
	return FALSE;
}

GtkWidget *
reader_window_new(const gchar *filename)
{
	ReaderWindow *reader = READER_WINDOW(g_object_new(READER_TYPE_WINDOW, nullptr));
	gtk_window_set_default_size(GTK_WINDOW(reader), 400, 300);

#ifdef HAVE_GTK3_HEADER_BAR
	GtkWidget *header = gtk_header_bar_new();
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), i18n("Cainteoir Text-to-Speech"));
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_window_set_titlebar(GTK_WINDOW(reader), header);
#else
	gtk_window_set_title(GTK_WINDOW(reader), i18n("Cainteoir Text-to-Speech"));
#endif

	GtkWidget *document_layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_container_add(GTK_CONTAINER(reader), document_layout);

	GtkWidget *view_scroll = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_box_pack_end(GTK_BOX(document_layout), view_scroll, TRUE, TRUE, 0);

	reader->priv->view  = cainteoir_document_view_new();
	gtk_container_add(GTK_CONTAINER(view_scroll), reader->priv->view);

	GtkWidget *index_scroll = gtk_scrolled_window_new(nullptr, nullptr);
	g_object_set(G_OBJECT(index_scroll), "width-request", 265, nullptr);
	gtk_box_pack_end(GTK_BOX(document_layout), index_scroll, FALSE, TRUE, 0);

	reader->priv->index = cainteoir_document_index_new(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view));
	gtk_container_add(GTK_CONTAINER(index_scroll), reader->priv->index);

	g_signal_connect(reader, "window-state-event", G_CALLBACK(on_window_state_changed), reader->priv->settings);
	g_signal_connect(reader, "delete_event", G_CALLBACK(on_window_delete), reader);

	gtk_window_resize(GTK_WINDOW(reader),
	                  cainteoir_settings_get_integer(reader->priv->settings, "window", "width",  700),
	                  cainteoir_settings_get_integer(reader->priv->settings, "window", "height", 445));
	gtk_window_move(GTK_WINDOW(reader),
	                cainteoir_settings_get_integer(reader->priv->settings, "window", "left", 0),
	                cainteoir_settings_get_integer(reader->priv->settings, "window", "top",  0));
	if (cainteoir_settings_get_boolean(reader->priv->settings, "window", "maximized", FALSE))
		gtk_window_maximize(GTK_WINDOW(reader));

	if (filename)
		reader_window_load_document(reader, filename);
	else
	{
		gchar *prev_filename = cainteoir_settings_get_string(reader->priv->settings, "document", "filename", nullptr);
		if (prev_filename)
		{
			reader_window_load_document(reader, prev_filename);
			g_free(prev_filename);
		}
	}

	return GTK_WIDGET(reader);
}

gboolean
reader_window_load_document(ReaderWindow *reader,
                            const gchar *filename)
{
	CainteoirDocument *doc = cainteoir_document_new(filename);
	if (doc)
	{
		cainteoir_document_view_set_document(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view), doc);

		CainteoirMetadata *metadata = cainteoir_document_get_metadata(doc);
		gchar *mimetype = cainteoir_metadata_get_string(metadata, CAINTEOIR_METADATA_MIMETYPE);

		cainteoir_settings_set_string(reader->priv->settings, "document", "filename", filename);
		cainteoir_settings_set_string(reader->priv->settings, "document", "mimetype", mimetype);
		cainteoir_settings_save(reader->priv->settings);

		cainteoir_document_index_build(CAINTEOIR_DOCUMENT_INDEX(reader->priv->index), doc, CAINTEOIR_INDEXTYPE_TOC);
		if (cainteoir_document_index_is_empty(CAINTEOIR_DOCUMENT_INDEX(reader->priv->index)))
			gtk_widget_hide(reader->priv->index);
		else
			gtk_widget_show(reader->priv->index);

		if (mimetype) g_free(mimetype);
		g_object_unref(metadata);
		g_object_unref(doc);
		return TRUE;
	}
	return FALSE;
}
