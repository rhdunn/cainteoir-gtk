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

#ifndef CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_INDEX_H
#define CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_INDEX_H

#include "cainteoir_document.h"

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_DOCUMENT_INDEX \
	(cainteoir_document_index_get_type())
#define CAINTEOIR_DOCUMENT_INDEX(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_DOCUMENT_INDEX, CainteoirDocumentIndex))
#define CAINTEOIR_DOCUMENT_INDEX_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_DOCUMENT_INDEX, CainteoirDocumentIndexClass))
#define CAINTEOIR_IS_DOCUMENT_INDEX(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_DOCUMENT_INDEX))
#define CAINTEOIR_IS_DOCUMENT_INDEX_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_DOCUMENT_INDEX))
#define CAINTEOIR_DOCUMENT_INDEX_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_DOCUMENT_INDEX, CainteoirDocumentIndexClass))

typedef struct _CainteoirDocumentIndex        CainteoirDocumentIndex;
typedef struct _CainteoirDocumentIndexPrivate CainteoirDocumentIndexPrivate;
typedef struct _CainteoirDocumentIndexClass   CainteoirDocumentIndexClass;

struct _CainteoirDocumentIndex
{
	GtkTreeView parent_instance;

	/*< private >*/
	CainteoirDocumentIndexPrivate *priv;
};

struct _CainteoirDocumentIndexClass
{
	GtkTreeViewClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

#define CAINTEOIR_INDEXTYPE_TOC        "http://www.idpf.org/epub/vocab/structure/#toc"

GType                                  cainteoir_document_index_get_type(void) G_GNUC_CONST;

GtkWidget *                            cainteoir_document_index_new(void);

void                                   cainteoir_document_index_build(CainteoirDocumentIndex *index,
                                                                      CainteoirDocument *doc,
                                                                      const gchar *index_type);

G_END_DECLS

#endif
