/* Library Entry Cell Renderer
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

#include "config.h"
#include "compatibility.hpp"
#include "i18n.h"

#include "libraryentrycellrenderer.h"

enum
{
	PROP_TITLE = 1,
};

static void
cainteoir_library_entry_cell_renderer_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *psec)
{
	CainteoirLibraryEntryCellRenderer *cellprogress = CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER(object);
	switch (param_id)
	{
	case PROP_TITLE:
		g_value_set_string(value, cellprogress->title);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, psec);
		break;
	}
}

static void
cainteoir_library_entry_cell_renderer_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
	CainteoirLibraryEntryCellRenderer *cellprogress = CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER (object);
	switch (param_id)
	{
	case PROP_TITLE:
		g_free(cellprogress->title);
		cellprogress->title = g_strdup(g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
		break;
	}
}

#define FIXED_WIDTH  400
#define FIXED_HEIGHT 25

static void
cainteoir_library_entry_cell_renderer_get_size (GtkCellRenderer *cell, GtkWidget *widget, const GdkRectangle *cell_area, gint *x_offset, gint *y_offset, gint *width, gint *height)
{
	gint calc_width  = FIXED_WIDTH;
	gint calc_height = FIXED_HEIGHT;

	if (width)  *width  = calc_width;
	if (height) *height = calc_height;

	if (cell_area)
	{
		if (x_offset) *x_offset = 0;
		if (y_offset) *y_offset = 0;
	}
}

static void
cainteoir_library_entry_cell_renderer_render (GtkCellRenderer *cell, cairo_t *cr, GtkWidget *widget, const GdkRectangle *background_area, const GdkRectangle *cell_area, GtkCellRendererState state)
{
	CainteoirLibraryEntryCellRenderer *cellprogress = CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER (cell);
	gint width, height;
	gint x_offset, y_offset;

	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	PangoLayout *layout = gtk_widget_create_pango_layout(widget, cellprogress->title);

	cainteoir_library_entry_cell_renderer_get_size(cell, widget, cell_area, &x_offset, &y_offset, &width, &height);

	int x_pos = cell_area->x;
	int y_pos = cell_area->y;

	gtk_style_context_save(context);
	gtk_style_context_add_class(context, GTK_STYLE_CLASS_TROUGH);

	cairo_save(cr);
	gdk_cairo_rectangle(cr, cell_area);
	cairo_clip(cr);

	gtk_render_layout (context, cr, x_pos, y_pos, layout);

	cairo_restore(cr);

	gtk_style_context_restore(context);
	g_object_unref(layout);
}

static gpointer parent_class;

static void
cainteoir_library_entry_cell_renderer_finalize(GObject *object)
{
	CainteoirLibraryEntryCellRenderer *cellprogress = CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER (object);

	g_free(cellprogress->title);

	G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void
cainteoir_library_entry_cell_renderer_init(CainteoirLibraryEntryCellRenderer *cellrendererprogress)
{
}

static void
custom_cell_renderer_progress_class_init(CainteoirLibraryEntryCellRendererClass *klass)
{
	GtkCellRendererClass *cell_class   = GTK_CELL_RENDERER_CLASS(klass);
	GObjectClass         *object_class = G_OBJECT_CLASS(klass);

	parent_class           = g_type_class_peek_parent (klass);
	object_class->finalize = cainteoir_library_entry_cell_renderer_finalize;

	object_class->get_property = cainteoir_library_entry_cell_renderer_get_property;
	object_class->set_property = cainteoir_library_entry_cell_renderer_set_property;

	cell_class->get_size = cainteoir_library_entry_cell_renderer_get_size;
	cell_class->render   = cainteoir_library_entry_cell_renderer_render;

	g_object_class_install_property(object_class, PROP_TITLE,
		g_param_spec_string (
			"title",
			"Title",
			"The document title",
			NULL,
			(GParamFlags)G_PARAM_READWRITE));
}

GType cainteoir_library_entry_cell_renderer_get_type()
{
	static GType library_entry_type = 0;
	if (library_entry_type == 0)
	{
		static const GTypeInfo cell_progress_info =
		{
			sizeof(CainteoirLibraryEntryCellRendererClass),
			NULL, // base_init
			NULL, // base_finalize
			(GClassInitFunc) custom_cell_renderer_progress_class_init,
			NULL, // class_finalize
			NULL, // class_data
			sizeof(CainteoirLibraryEntryCellRenderer),
			0, // n_preallocs
			(GInstanceInitFunc)cainteoir_library_entry_cell_renderer_init,
		};

		library_entry_type = g_type_register_static(
			GTK_TYPE_CELL_RENDERER,
			"CainteoirLibraryEntryCellRenderer",
			&cell_progress_info,
			(GTypeFlags)0);
	}
	return library_entry_type;
}

GtkCellRenderer *cainteoir_library_entry_cell_renderer_new()
{
	return (GtkCellRenderer *)g_object_new(CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_TYPE, NULL);
}
