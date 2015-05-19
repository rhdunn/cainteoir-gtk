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

enum IndexTypeColumns
{
	INDEX_TYPE_LABEL,
	INDEX_TYPE_ID,
	INDEX_TYPE_COUNT,
};

struct _ReaderWindowPrivate
{
	GtkWidget *doc_pane;
	GtkWidget *index_type;
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

	cainteoir_settings_set_string(reader->priv->settings, "document", "index-type",
	                              gtk_combo_box_get_active_id(GTK_COMBO_BOX(reader->priv->index_type)));

	cainteoir_settings_set_integer(reader->priv->settings, "document", "splitter.pos",
	                               gtk_paned_get_position(GTK_PANED(reader->priv->doc_pane)));

	cainteoir_settings_save(reader->priv->settings);
	return FALSE;
}

static GtkWidget *
create_index_type_combo(void)
{
	GtkTreeStore *store = gtk_tree_store_new(INDEX_TYPE_COUNT, G_TYPE_STRING, G_TYPE_STRING);
	GtkTreeIter row;

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Index"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_TOC,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Index (Abridged)"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_TOC_BRIEF,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Landmarks"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_LANDMARKS,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Pages"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_PAGE_LIST,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Illustrations"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_ILLUSTRATIONS,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Tables"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_TABLES,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Audio"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_AUDIO,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Video"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_VIDEO,
	                   -1);

	GtkWidget *combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_combo_box_set_id_column(GTK_COMBO_BOX(combo), INDEX_TYPE_ID);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", INDEX_TYPE_LABEL, nullptr);

	g_object_unref(G_OBJECT(store));
	return combo;
}

static void
on_index_type_changed(GtkWidget *window, gpointer data)
{
	ReaderWindow *reader = (ReaderWindow *)data;

	CainteoirDocument *doc = cainteoir_document_view_get_document(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view));
	if (doc)
	{
		cainteoir_document_index_build(CAINTEOIR_DOCUMENT_INDEX(reader->priv->index), doc,
		                               gtk_combo_box_get_active_id(GTK_COMBO_BOX(reader->priv->index_type)));
		g_object_unref(G_OBJECT(doc));
	}
}

GtkWidget *
reader_window_new(const gchar *filename)
{
	static constexpr int INDEX_PANE_WIDTH = 265;
	static constexpr int DOCUMENT_PANE_WIDTH = 300;

	ReaderWindow *reader = READER_WINDOW(g_object_new(READER_TYPE_WINDOW, nullptr));
	gtk_window_set_default_size(GTK_WINDOW(reader), INDEX_PANE_WIDTH + DOCUMENT_PANE_WIDTH + 5, 300);
	gtk_window_set_title(GTK_WINDOW(reader), i18n("Cainteoir Text-to-Speech"));

#ifdef HAVE_GTK3_HEADER_BAR
	GtkWidget *header = gtk_header_bar_new();
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), i18n("Cainteoir Text-to-Speech"));
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_window_set_titlebar(GTK_WINDOW(reader), header);
#endif

	reader->priv->doc_pane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_container_add(GTK_CONTAINER(reader), reader->priv->doc_pane);

	GtkWidget *view_scroll = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_widget_set_size_request(view_scroll, DOCUMENT_PANE_WIDTH, -1);
	gtk_paned_pack2(GTK_PANED(reader->priv->doc_pane), view_scroll, TRUE, FALSE);

	reader->priv->view  = cainteoir_document_view_new();
	gtk_container_add(GTK_CONTAINER(view_scroll), reader->priv->view);

	GtkWidget *index_pane = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_widget_set_size_request(index_pane, INDEX_PANE_WIDTH, -1);
	gtk_paned_pack1(GTK_PANED(reader->priv->doc_pane), index_pane, TRUE, FALSE);

	reader->priv->index_type = create_index_type_combo();
	gtk_box_pack_start(GTK_BOX(index_pane), reader->priv->index_type, FALSE, FALSE, 0);

	GtkWidget *index_scroll = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_box_pack_start(GTK_BOX(index_pane), index_scroll, TRUE, TRUE, 0);

	reader->priv->index = cainteoir_document_index_new(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view));
	gtk_container_add(GTK_CONTAINER(index_scroll), reader->priv->index);

	g_signal_connect(reader, "window-state-event", G_CALLBACK(on_window_state_changed), reader->priv->settings);
	g_signal_connect(reader, "delete_event", G_CALLBACK(on_window_delete), reader);
	g_signal_connect(reader->priv->index_type, "changed", G_CALLBACK(on_index_type_changed), reader);

	gtk_window_resize(GTK_WINDOW(reader),
	                  cainteoir_settings_get_integer(reader->priv->settings, "window", "width",  700),
	                  cainteoir_settings_get_integer(reader->priv->settings, "window", "height", 445));
	gtk_window_move(GTK_WINDOW(reader),
	                cainteoir_settings_get_integer(reader->priv->settings, "window", "left", 0),
	                cainteoir_settings_get_integer(reader->priv->settings, "window", "top",  0));
	if (cainteoir_settings_get_boolean(reader->priv->settings, "window", "maximized", FALSE))
		gtk_window_maximize(GTK_WINDOW(reader));


	gchar *active_index = cainteoir_settings_get_string(reader->priv->settings, "document", "index-type", CAINTEOIR_INDEXTYPE_TOC);
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(reader->priv->index_type), active_index);
	g_free(active_index);

	gtk_paned_set_position(GTK_PANED(reader->priv->doc_pane),
	                       cainteoir_settings_get_integer(reader->priv->settings, "document", "splitter.pos", INDEX_PANE_WIDTH));

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

		cainteoir_document_index_build(CAINTEOIR_DOCUMENT_INDEX(reader->priv->index), doc,
		                               gtk_combo_box_get_active_id(GTK_COMBO_BOX(reader->priv->index_type)));
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
