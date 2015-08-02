/* Cainteoir Reader application.
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

#ifndef CAINTEOIR_READER_READER_APPLICATION_H
#define CAINTEOIR_READER_READER_APPLICATION_H

typedef struct _CainteoirDocument           CainteoirDocument;

G_BEGIN_DECLS

#define READER_TYPE_APPLICATION \
	(reader_application_get_type())
#define READER_APPLICATION(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), READER_TYPE_APPLICATION, ReaderApplication))
#define READER_APPLICATION_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), READER_TYPE_APPLICATION, ReaderApplicationClass))
#define READER_IS_APPLICATION(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), READER_TYPE_APPLICATION))
#define READER_IS_APPLICATION_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), READER_TYPE_APPLICATION))
#define READER_APPLICATION_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), READER_TYPE_APPLICATION, ReaderApplicationClass))

typedef struct _ReaderApplication      ReaderApplication;
typedef struct _ReaderApplicationClass ReaderApplicationClass;

struct _ReaderApplication
{
	GtkApplication parent_instance;
};

struct _ReaderApplicationClass
{
	GtkApplicationClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  reader_application_get_type(void) G_GNUC_CONST;

GApplication *                         reader_application_new(void);

void                                   reader_application_set_active_document(ReaderApplication *application,
                                                                              CainteoirDocument *doc);

G_END_DECLS

#endif
