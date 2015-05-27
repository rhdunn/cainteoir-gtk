/* The Cainteoir Reader document view.
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

#ifndef CAINTEOIR_READER_DOCUMENT_VIEW_H
#define CAINTEOIR_READER_DOCUMENT_VIEW_H

typedef struct _CainteoirSettings      CainteoirSettings;
typedef struct _CainteoirDocument      CainteoirDocument;
typedef struct _CainteoirDocumentIndex CainteoirDocumentIndex;

G_BEGIN_DECLS

#define READER_TYPE_DOCUMENT_VIEW \
	(reader_document_view_get_type())
#define READER_DOCUMENT_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), READER_TYPE_DOCUMENT_VIEW, ReaderDocumentView))
#define READER_DOCUMENT_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), READER_TYPE_DOCUMENT_VIEW, ReaderDocumentViewClass))
#define READER_IS_DOCUMENT_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), READER_TYPE_DOCUMENT_VIEW))
#define READER_IS_DOCUMENT_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), READER_TYPE_DOCUMENT_VIEW))
#define READER_DOCUMENT_VIEW_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), READER_TYPE_DOCUMENT_VIEW, ReaderDocumentViewClass))

typedef struct _ReaderDocumentView        ReaderDocumentView;
typedef struct _ReaderDocumentViewClass   ReaderDocumentViewClass;

struct _ReaderDocumentView
{
	GtkBin parent_instance;
};

struct _ReaderDocumentViewClass
{
	GtkBinClass parent_class;

	/* Padding for future expansion */
	void (*_reserved1)(void);
	void (*_reserved2)(void);
	void (*_reserved3)(void);
	void (*_reserved4)(void);
};

#define INDEX_PANE_WIDTH    265
#define DOCUMENT_PANE_WIDTH 300

GType                                  reader_document_view_get_type(void) G_GNUC_CONST;

GtkWidget *                            reader_document_view_new(CainteoirSettings *settings);

void                                   reader_document_view_set_index_pane_close_action_name(ReaderDocumentView *view,
                                                                                             const gchar *action_name);

gboolean                               reader_document_view_load_document(ReaderDocumentView *view,
                                                                          const gchar *filename);

CainteoirDocument *                    reader_document_view_get_document(ReaderDocumentView *view);

CainteoirDocumentIndex *               reader_document_view_get_document_index(ReaderDocumentView *view);

const gchar *                          reader_document_view_get_index_type(ReaderDocumentView *view);

void                                   reader_document_view_set_index_type(ReaderDocumentView *view,
                                                                           const gchar *index_type);

gint                                   reader_document_view_get_index_pane_position(ReaderDocumentView *view);

void                                   reader_document_view_set_index_pane_position(ReaderDocumentView *view,
                                                                                    gint position);

gboolean                               reader_document_view_get_index_pane_visible(ReaderDocumentView *view);

void                                   reader_document_view_set_index_pane_visible(ReaderDocumentView *view,
                                                                                   gboolean visible);

G_END_DECLS

#endif
