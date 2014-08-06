/* Display a cainteoir_document object.
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

#ifndef CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_VIEW_H
#define CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_VIEW_H

#include "cainteoir_document.h"

G_BEGIN_DECLS

#define CAINTEOIR_TYPE_DOCUMENT_VIEW \
	(cainteoir_document_view_get_type())
#define CAINTEOIR_DOCUMENT_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CAINTEOIR_TYPE_DOCUMENT_VIEW, CainteoirDocumentView))
#define CAINTEOIR_DOCUMENT_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CAINTEOIR_TYPE_DOCUMENT_VIEW, CainteoirDocumentViewClass))
#define CAINTEOIR_IS_DOCUMENT_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CAINTEOIR_TYPE_DOCUMENT_VIEW))
#define CAINTEOIR_IS_DOCUMENT_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), CAINTEOIR_TYPE_DOCUMENT_VIEW))
#define CAINTEOIR_DOCUMENT_VIEW_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), CAINTEOIR_TYPE_DOCUMENT_VIEW, CainteoirDocumentViewClass))

typedef struct _CainteoirDocumentView        CainteoirDocumentView;
typedef struct _CainteoirDocumentViewPrivate CainteoirDocumentViewPrivate;
typedef struct _CainteoirDocumentViewClass   CainteoirDocumentViewClass;

struct _CainteoirDocumentView
{
	GtkBin widget;

	/*< private >*/
	CainteoirDocumentViewPrivate *priv;
};

struct _CainteoirDocumentViewClass
{
	GtkBinClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

GType                                  cainteoir_document_view_get_type(void) G_GNUC_CONST;

GtkWidget *                            cainteoir_document_view_new(void);

void                                   cainteoir_document_view_set_document(CainteoirDocumentView *view,
                                                                            CainteoirDocument *doc);

CainteoirDocument *                    cainteoir_document_view_get_document(CainteoirDocumentView *view);

G_END_DECLS

#endif
