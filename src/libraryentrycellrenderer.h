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

#ifndef CAINTEOIRGTK_SRC_LIBRARYENTRYCELLRENDERER_H
#define CAINTEOIRGTK_SRC_LIBRARYENTRYCELLRENDERER_H

#define CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_TYPE \
	(cainteoir_library_entry_cell_renderer_get_type())

#define CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_TYPE, CainteoirLibraryEntryCellRenderer))

#define CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_TYPE, CainteoirLibraryEntryCellRendererClass))

#define IS_CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_TYPE))

#define IS_CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass),  CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_TYPE))

#define CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj),  CAINTEOIR_LIBRARY_ENTRY_CELL_RENDERER_TYPE, CainteoirLibraryEntryCellRendererClass))

typedef struct _CainteoirLibraryEntryCellRenderer
{
	GtkCellRenderer parent;
	gchar *title;
} CainteoirLibraryEntryCellRenderer;

typedef struct _CainteoirLibraryEntryCellRendererClass
{
	GtkCellRendererClass parent_class;
} CainteoirLibraryEntryCellRendererClass;

GType cainteoir_library_entry_cell_renderer_get_type();

GtkCellRenderer *cainteoir_library_entry_cell_renderer_new();

#endif
