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

#ifndef CAINTEOIR_READER_READER_WINDOW_H
#define CAINTEOIR_READER_READER_WINDOW_H

G_BEGIN_DECLS

#define READER_TYPE_WINDOW \
	(reader_window_get_type())
#define READER_WINDOW(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), READER_TYPE_WINDOW, ReaderWindow))
#define READER_WINDOW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), READER_TYPE_WINDOW, ReaderWindowClass))
#define READER_IS_WINDOW(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), READER_TYPE_WINDOW))
#define READER_IS_WINDOW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), READER_TYPE_WINDOW))
#define READER_WINDOW_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), READER_TYPE_WINDOW, ReaderWindowClass))

typedef struct _ReaderWindow        ReaderWindow;
typedef struct _ReaderWindowClass   ReaderWindowClass;

struct _ReaderWindow
{
	GtkWindow parent_instance;
};

struct _ReaderWindowClass
{
	GtkWindowClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  reader_window_get_type(void) G_GNUC_CONST;

GtkWidget *                            reader_window_new(const gchar *filename);

gboolean                               reader_window_load_document(ReaderWindow *reader,
                                                                   const gchar *filename);

G_END_DECLS

#endif
