/* Display a cainteoir_document object.
 *
 * Copyright (C) 2014-2015 Reece H. Dunn
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

#include <cainteoir-gtk/cainteoir_document_view.h>
#include <cainteoir-gtk/cainteoir_document.h>

#include "cainteoir_document_private.h"
#include "extensions/glib.h"

struct CainteoirDocumentViewPrivate
{
	GtkWidget *text_view;
	CainteoirDocument *doc;

	CainteoirDocumentViewPrivate()
		: text_view(gtk_text_view_new())
		, doc(nullptr)
	{
	}

	~CainteoirDocumentViewPrivate()
	{
		if (doc) g_object_unref(doc);
	}
};

enum
{
	PROP_0,
	// GtkScrollable interface:
	PROP_HADJUSTMENT,
	PROP_VADJUSTMENT,
	PROP_HSCROLL_POLICY,
	PROP_VSCROLL_POLICY,
};

G_DEFINE_TYPE_WITH_CODE(CainteoirDocumentView, cainteoir_document_view, GTK_TYPE_BIN,
	G_ADD_PRIVATE(CainteoirDocumentView)
	G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, nullptr))

#define CAINTEOIR_DOCUMENT_VIEW_PRIVATE(object) \
	((CainteoirDocumentViewPrivate *)cainteoir_document_view_get_instance_private(CAINTEOIR_DOCUMENT_VIEW(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(CainteoirDocumentView, cainteoir_document_view, CAINTEOIR_DOCUMENT_VIEW)

static void
cainteoir_document_view_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	CainteoirDocumentViewPrivate *priv = CAINTEOIR_DOCUMENT_VIEW_PRIVATE(object);
	GtkScrollable *scroll = GTK_SCROLLABLE(priv->text_view);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	// GtkScrollable interface:
	case PROP_HADJUSTMENT:
		gtk_scrollable_set_hadjustment(scroll, GTK_ADJUSTMENT(g_value_get_object(value)));
		break;
	case PROP_VADJUSTMENT:
		gtk_scrollable_set_vadjustment(scroll, GTK_ADJUSTMENT(g_value_get_object(value)));
		break;
	case PROP_HSCROLL_POLICY:
		gtk_scrollable_set_hscroll_policy(scroll, (GtkScrollablePolicy)g_value_get_enum(value));
		break;
	case PROP_VSCROLL_POLICY:
		gtk_scrollable_set_vscroll_policy(scroll, (GtkScrollablePolicy)g_value_get_enum(value));
		break;
	}
}

static void
cainteoir_document_view_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CainteoirDocumentViewPrivate *priv = CAINTEOIR_DOCUMENT_VIEW_PRIVATE(object);
	GtkScrollable *scroll = GTK_SCROLLABLE(priv->text_view);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	// GtkScrollable interface:
	case PROP_HADJUSTMENT:
		g_value_set_object(value, gtk_scrollable_get_hadjustment(scroll));
		break;
	case PROP_VADJUSTMENT:
		g_value_set_object(value, gtk_scrollable_get_vadjustment(scroll));
		break;
	case PROP_HSCROLL_POLICY:
		g_value_set_enum(value, gtk_scrollable_get_hscroll_policy(scroll));
		break;
	case PROP_VSCROLL_POLICY:
		g_value_set_enum(value, gtk_scrollable_get_vscroll_policy(scroll));
		break;
	}
}

static void
cainteoir_document_view_class_init(CainteoirDocumentViewClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->set_property = cainteoir_document_view_set_property;
	object->get_property = cainteoir_document_view_get_property;
	object->finalize = cainteoir_document_view_finalize;

	// GtkScrollable interface:
	g_object_class_override_property(object, PROP_HADJUSTMENT,    "hadjustment");
	g_object_class_override_property(object, PROP_VADJUSTMENT,    "vadjustment");
	g_object_class_override_property(object, PROP_HSCROLL_POLICY, "hscroll-policy");
	g_object_class_override_property(object, PROP_VSCROLL_POLICY, "vscroll-policy");
}

GtkWidget *
cainteoir_document_view_new()
{
	CainteoirDocumentView *view = CAINTEOIR_DOCUMENT_VIEW(g_object_new(CAINTEOIR_TYPE_DOCUMENT_VIEW, nullptr));
	CainteoirDocumentViewPrivate *priv = CAINTEOIR_DOCUMENT_VIEW_PRIVATE(view);
	gtk_container_add(GTK_CONTAINER(view), priv->text_view);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->text_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv->text_view), GTK_WRAP_WORD_CHAR);
	return GTK_WIDGET(view);
}

void
cainteoir_document_view_set_document(CainteoirDocumentView *view, CainteoirDocument *doc)
{
	CainteoirDocumentViewPrivate *priv = CAINTEOIR_DOCUMENT_VIEW_PRIVATE(view);

	if (priv->doc) g_object_unref(priv->doc);
	priv->doc = CAINTEOIR_DOCUMENT(g_object_ref(doc));

	GtkTextBuffer *buffer = cainteoir_document_create_buffer(doc);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(priv->text_view), buffer);
}

CainteoirDocument *
cainteoir_document_view_get_document(CainteoirDocumentView *view)
{
	CainteoirDocumentViewPrivate *priv = CAINTEOIR_DOCUMENT_VIEW_PRIVATE(view);
	return priv->doc ? CAINTEOIR_DOCUMENT(g_object_ref(priv->doc)) : nullptr;
}

void
cainteoir_document_view_scroll_to_anchor(CainteoirDocumentView *view, const gchar *anchor)
{
	GtkTextView *text_view = GTK_TEXT_VIEW(CAINTEOIR_DOCUMENT_VIEW_PRIVATE(view)->text_view);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
	GtkTextMark *mark = gtk_text_buffer_get_mark(buffer, anchor);

	GtkTextIter position;
	if (mark)
	{
		gtk_text_buffer_get_iter_at_mark(buffer, &position, mark);
		gtk_text_iter_forward_char(&position);
	}
	else
		gtk_text_buffer_get_start_iter(buffer, &position);

	gtk_text_view_scroll_to_iter(text_view, &position, 0, TRUE, 0.0, 0.0);
}
