/* Table of Content Side Pane
 *
 * Copyright (C) 2011-2014 Reece H. Dunn
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
#include "compatibility.hpp"

#include "toc.hpp"

enum TocColumns
{
	TOC_ENTRY_PTR,
	TOC_GUTTER,
	TOC_TITLE,
	TOC_ANCHOR,
	TOC_COUNT
};

static const rdf::uri &uri_from_selected_item(GtkTreeModel *model, GList *item, bool advance)
{
	static const rdf::uri empty_uri = {};

	GtkTreeIter iter;
	if (item && gtk_tree_model_get_iter(model, &iter, (GtkTreePath *)item->data))
	{
		if (advance)
		{
			if (!gtk_tree_model_iter_next(model, &iter))
				return empty_uri;
		}

		const cainteoir::ref_entry *entry = nullptr;
		gtk_tree_model_get(model, &iter, TOC_ENTRY_PTR, &entry, -1);
		return entry->location;
	}
	return empty_uri;
}

static bool find_ref_entry(GtkTreeModel *model, GtkTreeIter &iter, const cainteoir::ref_entry *value)
{
	if (!gtk_tree_model_get_iter_first(model, &iter))
		return false;

	do
	{
		const cainteoir::ref_entry *entry = nullptr;
		gtk_tree_model_get(model, &iter, TOC_ENTRY_PTR, &entry, -1);
		if (entry == value) return true;
	} while (gtk_tree_model_iter_next(model, &iter));

	return false;
}

static void on_cursor_changed(GtkTreeView *view, void *data)
{
	GtkTreeSelection *toc_selection = gtk_tree_view_get_selection(view);
	GtkTreeModel *model = gtk_tree_view_get_model(view);
	GList *selected = gtk_tree_selection_get_selected_rows(toc_selection, nullptr);
	GList *item = g_list_first(selected);

	GtkTreeIter iter;
	if (item && gtk_tree_model_get_iter(model, &iter, (GtkTreePath *)item->data))
	{
		gchar *anchor = nullptr;
		gtk_tree_model_get(model, &iter, TOC_ANCHOR, &anchor, -1);

		GtkTextView *text = GTK_TEXT_VIEW((GtkWidget *)data);
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(text);
		GtkTextMark *mark = gtk_text_buffer_get_mark(buffer, anchor);

		GtkTextIter position;
		if (mark)
		{
			gtk_text_buffer_get_iter_at_mark(buffer, &position, mark);
			gtk_text_iter_forward_char(&position);
		}
		else
			gtk_text_buffer_get_start_iter(buffer, &position);

		gtk_text_view_scroll_to_iter(text, &position, 0, TRUE, 0.0, 0.0);

		g_free(anchor);
	}
}

TocPane::TocPane()
	: mActive(nullptr)
{
	store = gtk_tree_store_new(TOC_COUNT, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	for (int i = 0; i < TOC_COUNT; ++i)
	{
		if (i == TOC_ANCHOR || i == TOC_ENTRY_PTR) continue;

		if (i == TOC_GUTTER)
		{
			GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
			GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
				"", renderer, "icon-name", i, nullptr);
			gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
		}
		else
		{
			GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
				"", renderer, "text", i, nullptr);
			gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
		}
	}

	gtk_widget_set_name(view, "toc");
	gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(view), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
	gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(view), TOC_TITLE);

	toc_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(toc_selection, GTK_SELECTION_MULTIPLE);
}

void TocPane::connect(GtkWidget *aTextView)
{
	g_signal_connect(view, "cursor-changed", G_CALLBACK(on_cursor_changed), aTextView);
}

bool TocPane::empty() const
{
	GtkTreeIter iter;
	return gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter) == FALSE;
}

void TocPane::clear()
{
	gtk_tree_store_clear(store);
}

void TocPane::add(const cainteoir::ref_entry &entry)
{
	GtkTreeIter row;
	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
		TOC_ENTRY_PTR, &entry,
		TOC_GUTTER,    "",
		TOC_TITLE,     entry.title.c_str(),
		TOC_ANCHOR,    entry.location.str().c_str(),
		-1);
}

void TocPane::set_playing(const cainteoir::ref_entry &entry)
{
	if (&entry == mActive) return;

	if (mActive)
		gtk_tree_store_set(store, &mActiveIter, TOC_GUTTER, "", -1);

	if (!find_ref_entry(GTK_TREE_MODEL(store), mActiveIter, &entry))
		return;

	gtk_tree_store_set(store, &mActiveIter, TOC_GUTTER, "media-playback-start", -1);
	mActive = &entry;
}

TocSelection TocPane::selection() const
{
	GList *selected = gtk_tree_selection_get_selected_rows(toc_selection, nullptr);
	rdf::uri from;
	rdf::uri to;

	switch (g_list_length(selected))
	{
	case 0: // read all ...
		break;
	case 1: // read from selected item ...
		from = uri_from_selected_item(GTK_TREE_MODEL(store), g_list_first(selected), false);
		break;
	default: // read selected range ...
		from = uri_from_selected_item(GTK_TREE_MODEL(store), g_list_first(selected), false);
		to   = uri_from_selected_item(GTK_TREE_MODEL(store), g_list_last(selected),  true);
		break;
	}

	g_list_foreach(selected, (GFunc)gtk_tree_path_free, nullptr);
	g_list_free(selected);

	return TocSelection(from, to);
}
