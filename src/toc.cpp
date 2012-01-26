/* Table of Content Side Pane
 *
 * Copyright (C) 2011-2012 Reece H. Dunn
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

#include "toc.hpp"

enum TocColumns
{
	TOC_TITLE,
	TOC_ANCHOR, // rdf::uri
	TOC_COUNT // number of columns
};

static rdf::uri uri_from_selected_item(GtkTreeModel *model, GList *item)
{
	GtkTreeIter iter;
	if (item && gtk_tree_model_get_iter(model, &iter, (GtkTreePath *)item->data))
	{
		gchar *anchor = NULL;
		gtk_tree_model_get(model, &iter, TOC_ANCHOR, &anchor, -1);

		rdf::uri ref(anchor);
		g_free(anchor);
		return ref;
	}
	return rdf::uri();
}

TocPane::TocPane()
{
	store = gtk_tree_store_new(TOC_COUNT, G_TYPE_STRING, G_TYPE_STRING);

	GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	for (int i = 0; i < TOC_COUNT; ++i)
	{
		if (i == TOC_ANCHOR) continue;

		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("", renderer, "text", i, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	}

	gtk_widget_set_name(view, "toc");
	gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(view), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

	toc_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(toc_selection, GTK_SELECTION_MULTIPLE);

	layout = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(layout), GTK_WIDGET(view));
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

void TocPane::add(int depth, const rdf::uri &location, const std::string &title)
{
	GtkTreeIter row;
	gtk_tree_store_append(store, &row, NULL);
	gtk_tree_store_set(store, &row,
		TOC_TITLE,  title.c_str(),
		TOC_ANCHOR, location.str().c_str(),
		-1);
}

TocSelection TocPane::selection() const
{
	GList *selected = gtk_tree_selection_get_selected_rows(toc_selection, NULL);
	rdf::uri from;
	rdf::uri to;

	switch (g_list_length(selected))
	{
	case 0: // read all ...
		break;
	case 1: // read from selected item ...
		from = uri_from_selected_item(GTK_TREE_MODEL(store), g_list_first(selected));
		break;
	default: // read selected range ...
		from = uri_from_selected_item(GTK_TREE_MODEL(store), g_list_first(selected));
		to   = uri_from_selected_item(GTK_TREE_MODEL(store), g_list_last(selected));
		break;
	}

	g_list_foreach(selected, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(selected);

	return TocSelection(from, to);
}
