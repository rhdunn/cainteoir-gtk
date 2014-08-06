/* A GTK+ wrapper around the cainteoir::document class.
 *
 * Copyright (C) 2014 Reece H. Dunn
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

#ifndef CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_H
#define CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_H

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_DOCUMENT \
	(cainteoir_document_get_type())
#define CAINTEOIR_DOCUMENT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_DOCUMENT, CainteoirDocument))
#define CAINTEOIR_DOCUMENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_DOCUMENT, CainteoirDocumentClass))
#define CAINTEOIR_IS_DOCUMENT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_DOCUMENT))
#define CAINTEOIR_IS_DOCUMENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_DOCUMENT))
#define CAINTEOIR_DOCUMENT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_DOCUMENT, CainteoirDocumentClass))

typedef struct _CainteoirDocument        CainteoirDocument;
typedef struct _CainteoirDocumentPrivate CainteoirDocumentPrivate;
typedef struct _CainteoirDocumentClass   CainteoirDocumentClass;

struct _CainteoirDocument
{
	GObject parent_instance;

	/*< private >*/
	CainteoirDocumentPrivate *priv;
};

struct _CainteoirDocumentClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  cainteoir_document_get_type(void) G_GNUC_CONST;

CainteoirDocument *                    cainteoir_document_new(const gchar *filename);

size_t                                 cainteoir_document_get_text_length(CainteoirDocument *doc);

G_END_DECLS

#endif
