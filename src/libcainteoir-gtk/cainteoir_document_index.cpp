/* A GTK+ wrapper around the cainteoir::ref_entry class.
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

#include <gtk/gtk.h>

#include <cainteoir-gtk/cainteoir_document_index.h>
#include <cainteoir-gtk/cainteoir_document_view.h>
#include <cainteoir/document.hpp>

#include "cainteoir_document_private.h"
#include "cainteoir_document_index_private.h"
#include "extensions/glib.h"

namespace rdf = cainteoir::rdf;

enum IndexColumns
{
	INDEX_ENTRY_PTR,
	INDEX_GUTTER,
	INDEX_TITLE,
	INDEX_ANCHOR,
	INDEX_DISPLAY,
	INDEX_COUNT
};

struct CainteoirDocumentIndexPrivate
{
	GtkTreeStore *store;
	GtkTreeSelection *selection;

	std::vector<cainteoir::ref_entry> index;
	const cainteoir::ref_entry *active;
	GtkTreeIter active_iter;

	CainteoirDocumentIndexPrivate()
		: active(nullptr)
	{
		store = gtk_tree_store_new(INDEX_COUNT, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	}
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirDocumentIndex, cainteoir_document_index, GTK_TYPE_TREE_VIEW)

#define CAINTEOIR_DOCUMENT_INDEX_PRIVATE(object) \
	((CainteoirDocumentIndexPrivate *)cainteoir_document_index_get_instance_private(CAINTEOIR_DOCUMENT_INDEX(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirDocumentIndex, cainteoir_document_index, CAINTEOIR_DOCUMENT_INDEX)

static void
cainteoir_document_index_class_init(CainteoirDocumentIndexClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_document_index_finalize;
}

static const rdf::uri &
uri_from_selected_item(GtkTreeModel *model, GList *item, bool advance)
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
		gtk_tree_model_get(model, &iter, INDEX_ENTRY_PTR, &entry, -1);
		return entry->location;
	}
	return empty_uri;
}

static bool
find_ref_entry(GtkTreeModel *model, GtkTreeIter &iter, const cainteoir::ref_entry *value)
{
	if (!gtk_tree_model_get_iter_first(model, &iter))
		return false;

	do
	{
		const cainteoir::ref_entry *entry = nullptr;
		gtk_tree_model_get(model, &iter, INDEX_ENTRY_PTR, &entry, -1);
		if (entry == value) return true;
	} while (gtk_tree_model_iter_next(model, &iter));

	return false;
}

static void
on_cursor_changed(GtkTreeView *view, void *data)
{
	GtkTreeSelection *toc_selection = gtk_tree_view_get_selection(view);
	GtkTreeModel *model = gtk_tree_view_get_model(view);
	GList *selected = gtk_tree_selection_get_selected_rows(toc_selection, nullptr);
	GList *item = g_list_first(selected);

	GtkTreeIter iter;
	if (item && gtk_tree_model_get_iter(model, &iter, (GtkTreePath *)item->data))
	{
		gchar *anchor = nullptr;
		gtk_tree_model_get(model, &iter, INDEX_ANCHOR, &anchor, -1);

		cainteoir_document_view_scroll_to_anchor(CAINTEOIR_DOCUMENT_VIEW(data), anchor);

		g_free(anchor);
	}
}

static void
add_icon_column(GtkTreeView *treeview, int i)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("", renderer, "icon-name", i, nullptr);
	gtk_tree_view_append_column(treeview, column);
}

static void
add_text_column(GtkTreeView *treeview, int i)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("", renderer, "text", i, nullptr);
	gtk_tree_view_append_column(treeview, column);
}

GtkWidget *
cainteoir_document_index_new(CainteoirDocumentView *view)
{
	CainteoirDocumentIndex *self = CAINTEOIR_DOCUMENT_INDEX(g_object_new(CAINTEOIR_TYPE_DOCUMENT_INDEX, nullptr));
	CainteoirDocumentIndexPrivate *priv = CAINTEOIR_DOCUMENT_INDEX_PRIVATE(self);

	gtk_tree_view_set_model(GTK_TREE_VIEW(self), GTK_TREE_MODEL(priv->store));
	add_icon_column(GTK_TREE_VIEW(self), INDEX_GUTTER);
	add_text_column(GTK_TREE_VIEW(self), INDEX_DISPLAY);

	gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(self), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self), FALSE);
	gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(self), INDEX_TITLE);

	priv->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self));
	gtk_tree_selection_set_mode(priv->selection, GTK_SELECTION_MULTIPLE);

	g_signal_connect(GTK_TREE_VIEW(self), "cursor-changed", G_CALLBACK(on_cursor_changed), view);

	return GTK_WIDGET(self);
}

void
cainteoir_document_index_build(CainteoirDocumentIndex *index,
                               CainteoirDocument *doc,
                               const gchar *index_type)
{
	CainteoirDocumentIndexPrivate *priv = CAINTEOIR_DOCUMENT_INDEX_PRIVATE(index);

	gtk_tree_store_clear(priv->store);
	if (!index_type)
		return;

	rdf::graph &metadata = *cainteoir_document_get_rdf_metadata(doc);
	rdf::uri &subject = *cainteoir_document_get_subject(doc);
	priv->index = cainteoir::navigation(metadata, subject, rdf::href(index_type));
	if (priv->index.empty())
		return;

	GtkTreeIter row;
	int initial_depth = priv->index.front().depth;
	for (const auto &entry : priv->index)
	{
		std::ostringstream title;
		for (const auto &i : cainteoir::range<int>(initial_depth, entry.depth))
			title << "... ";
		title << entry.title;

		std::ostringstream tooltip;
		for (auto c : entry.title) switch (c)
		{
		case '&': tooltip << "&amp;"; break;
		case '<': tooltip << "&lt;";  break;
		case '>': tooltip << "&gt;";  break;
		default:  tooltip << c;       break;
		}

		gtk_tree_store_append(priv->store, &row, nullptr);
		gtk_tree_store_set(priv->store, &row,
			INDEX_ENTRY_PTR, &entry,
			INDEX_GUTTER,    "",
			INDEX_TITLE,     tooltip.str().c_str(),
			INDEX_ANCHOR,    entry.location.str().c_str(),
			INDEX_DISPLAY,   title.str().c_str(),
			-1);
	}
}

gboolean
cainteoir_document_index_is_empty(CainteoirDocumentIndex *index)
{
	CainteoirDocumentIndexPrivate *priv = CAINTEOIR_DOCUMENT_INDEX_PRIVATE(index);
	GtkTreeIter iter;
	return gtk_tree_model_get_iter_first(GTK_TREE_MODEL(priv->store), &iter) == FALSE ? TRUE : FALSE;
}

const std::vector<cainteoir::ref_entry> *
cainteoir_document_index_get_listing(CainteoirDocumentIndex *index)
{
	return &CAINTEOIR_DOCUMENT_INDEX_PRIVATE(index)->index;
}

void
cainteoir_document_index_set_playing(CainteoirDocumentIndex *index,
                                     const cainteoir::ref_entry &entry)
{
	CainteoirDocumentIndexPrivate *priv = CAINTEOIR_DOCUMENT_INDEX_PRIVATE(index);

	if (&entry == priv->active) return;

	if (priv->active)
		gtk_tree_store_set(priv->store, &priv->active_iter, INDEX_GUTTER, "", -1);

	if (!find_ref_entry(GTK_TREE_MODEL(priv->store), priv->active_iter, &entry))
		return;

	gtk_tree_store_set(priv->store, &priv->active_iter, INDEX_GUTTER, "media-playback-start", -1);
	priv->active = &entry;
}

cainteoir_document_index_selection
cainteoir_document_index_get_selection_uri(CainteoirDocumentIndex *index)
{
	CainteoirDocumentIndexPrivate *priv = CAINTEOIR_DOCUMENT_INDEX_PRIVATE(index);

	GList *selected = gtk_tree_selection_get_selected_rows(priv->selection, nullptr);
	rdf::uri from;
	rdf::uri to;

	switch (g_list_length(selected))
	{
	case 0: // read all ...
		break;
	case 1: // read from selected item ...
		from = uri_from_selected_item(GTK_TREE_MODEL(priv->store), g_list_first(selected), false);
		break;
	default: // read selected range ...
		from = uri_from_selected_item(GTK_TREE_MODEL(priv->store), g_list_first(selected), false);
		to   = uri_from_selected_item(GTK_TREE_MODEL(priv->store), g_list_last(selected),  true);
		break;
	}

	g_list_foreach(selected, (GFunc)gtk_tree_path_free, nullptr);
	g_list_free(selected);

	return { from, to };
}
