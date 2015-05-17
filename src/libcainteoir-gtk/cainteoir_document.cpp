/* A GTK+ wrapper around the cainteoir::document class.
 *
 * Copyright (C) 2012-2014 Reece H. Dunn
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

#include <cainteoir-gtk/cainteoir_document.h>
#include <cainteoir/document.hpp>

#include "cainteoir_document_private.h"

#include <stack>

namespace rdf    = cainteoir::rdf;
namespace rql    = cainteoir::rdf::query;
namespace css    = cainteoir::css;
namespace events = cainteoir::events;

static constexpr int CHARACTERS_PER_WORD = 6;

struct _CainteoirDocumentPrivate
{
	std::shared_ptr<cainteoir::document> doc;
	rdf::graph metadata;
	rdf::uri subject;
};

G_DEFINE_TYPE_WITH_PRIVATE(CainteoirDocument, cainteoir_document, G_TYPE_OBJECT)

static void
cainteoir_document_finalize(GObject *object)
{
	CainteoirDocument *doc = CAINTEOIR_DOCUMENT(object);
	doc->priv->~CainteoirDocumentPrivate();

	G_OBJECT_CLASS(cainteoir_document_parent_class)->finalize(object);
}

static void
cainteoir_document_class_init(CainteoirDocumentClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = cainteoir_document_finalize;
}

static void
cainteoir_document_init(CainteoirDocument *doc)
{
	void * data = cainteoir_document_get_instance_private(doc);
	doc->priv = new (data)CainteoirDocumentPrivate();
}

CainteoirDocument *
cainteoir_document_new(const char *filename)
{
	CainteoirDocument *self = CAINTEOIR_DOCUMENT(g_object_new(CAINTEOIR_TYPE_DOCUMENT, nullptr));

	try
	{
		auto reader = cainteoir::createDocumentReader(filename, self->priv->metadata, std::string());
		if (!reader)
			throw std::runtime_error("the document type is not supported");

		self->priv->doc = std::make_shared<cainteoir::document>(reader, self->priv->metadata);
		self->priv->subject = rdf::uri(filename, std::string());
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "error: %s\n", e.what());
		g_object_unref(self);
		return nullptr;
	}

	return self;
}

size_t
cainteoir_document_get_text_length(CainteoirDocument *doc)
{
	if (!doc) return 0;
	return doc->priv->doc->text_length();
}

size_t
cainteoir_document_estimate_word_count(CainteoirDocument *doc)
{
	if (!doc) return 0;
	return doc->priv->doc->text_length() / CHARACTERS_PER_WORD;
}

double
cainteoir_document_estimate_duration(CainteoirDocument *doc, double words_per_minute)
{
	if (!doc) return 0.0;
	return ((double)doc->priv->doc->text_length() / CHARACTERS_PER_WORD / words_per_minute) * 60.0;
}

gchar *
cainteoir_document_get_mimetype(CainteoirDocument *doc)
{
	if (!doc) return nullptr;
	rql::results data = rql::select(doc->priv->metadata, rql::subject == doc->priv->subject);
	return g_strdup(rql::select_value<std::string>(data, rql::predicate == rdf::tts("mimetype")).c_str());
}

// private api ////////////////////////////////////////////////////////////////

struct tag_block
{
	GtkTextTag *tag;
	int offset;

	tag_block(GtkTextTag *aTag, int aOffset)
		: tag(aTag)
		, offset(aOffset)
	{
	}
};

GtkTextTag *
create_text_tag_from_style(const css::styles &aStyles)
{
	GtkTextTag *tag = gtk_text_tag_new(aStyles.name.c_str());

	switch (aStyles.vertical_align)
	{
	case css::vertical_align::inherit:
		break;
	case css::vertical_align::baseline:
		g_object_set(G_OBJECT(tag), "rise-set", TRUE, "rise", 0, NULL);
		break;
	case css::vertical_align::sub:
		g_object_set(G_OBJECT(tag), "rise-set", TRUE, "rise", -4096, NULL);
		break;
	case css::vertical_align::super:
		g_object_set(G_OBJECT(tag), "rise-set", TRUE, "rise", 4096, NULL);
		break;
	}

	switch (aStyles.text_align)
	{
	case css::text_align::inherit:
		break;
	case css::text_align::left:
		g_object_set(G_OBJECT(tag), "justification-set", TRUE, "justification", GTK_JUSTIFY_LEFT, NULL);
		break;
	case css::text_align::right:
		g_object_set(G_OBJECT(tag), "justification-set", TRUE, "justification", GTK_JUSTIFY_RIGHT, NULL);
		break;
	case css::text_align::center:
		g_object_set(G_OBJECT(tag), "justification-set", TRUE, "justification", GTK_JUSTIFY_CENTER, NULL);
		break;
	case css::text_align::justify:
		g_object_set(G_OBJECT(tag), "justification-set", TRUE, "justification", GTK_JUSTIFY_FILL, NULL);
		break;
	}

	switch (aStyles.text_decoration)
	{
	case css::text_decoration::inherit:
		break;
	case css::text_decoration::none:
		g_object_set(G_OBJECT(tag), "strikethrough-set", TRUE, "strikethrough", FALSE, NULL);
		g_object_set(G_OBJECT(tag), "underline-set", TRUE, "underline", PANGO_UNDERLINE_NONE, NULL);
		break;
	case css::text_decoration::underline:
		g_object_set(G_OBJECT(tag), "underline-set", TRUE, "underline", PANGO_UNDERLINE_SINGLE, NULL);
		break;
	case css::text_decoration::line_through:
		g_object_set(G_OBJECT(tag), "strikethrough-set", TRUE, "strikethrough", TRUE, NULL);
		break;
	}

	switch (aStyles.font_style)
	{
	case css::font_style::inherit:
		break;
	case css::font_style::normal:
		g_object_set(G_OBJECT(tag), "style-set", TRUE, "style", PANGO_STYLE_NORMAL, NULL);
		break;
	case css::font_style::italic:
		g_object_set(G_OBJECT(tag), "style-set", TRUE, "style", PANGO_STYLE_ITALIC, NULL);
		break;
	case css::font_style::oblique:
		g_object_set(G_OBJECT(tag), "style-set", TRUE, "style", PANGO_STYLE_OBLIQUE, NULL);
		break;
	}

	switch (aStyles.font_variant_caps)
	{
	case css::font_variant_caps::inherit:
		break;
	case css::font_variant_caps::normal:
		g_object_set(G_OBJECT(tag), "variant-set", TRUE, "variant", PANGO_VARIANT_NORMAL, NULL);
		break;
	case css::font_variant_caps::small_caps:
		g_object_set(G_OBJECT(tag), "variant-set", TRUE, "variant", PANGO_VARIANT_SMALL_CAPS, NULL);
		break;
	}

	switch (aStyles.font_weight)
	{
	case css::font_weight::inherit:
		break;
	case css::font_weight::normal:
		g_object_set(G_OBJECT(tag), "weight-set", TRUE, "weight", PANGO_WEIGHT_NORMAL, NULL);
		break;
	case css::font_weight::bold:
		g_object_set(G_OBJECT(tag), "weight-set", TRUE, "weight", PANGO_WEIGHT_BOLD, NULL);
		break;
	}

	if (!aStyles.font_family.empty())
		g_object_set(G_OBJECT(tag), "family-set", TRUE, "family", aStyles.font_family.c_str(), NULL);

	if (aStyles.font_size.units() != css::length::inherit)
		g_object_set(G_OBJECT(tag), "size-set", TRUE, "size-points", gdouble(aStyles.font_size.as(css::length::points).value()), NULL);

	if (aStyles.margin.top.units() != css::length::inherit)
		g_object_set(G_OBJECT(tag), "pixels-above-lines-set", TRUE, "pixels-above-lines", gint(aStyles.margin.top.as(css::length::pixels).value()), NULL);

	if (aStyles.margin.bottom.units() != css::length::inherit)
		g_object_set(G_OBJECT(tag), "pixels-below-lines-set", TRUE, "pixels-below-lines", gint(aStyles.margin.bottom.as(css::length::pixels).value()), NULL);

	if (aStyles.margin.left.units() != css::length::inherit)
		g_object_set(G_OBJECT(tag), "left-margin-set", TRUE, "left-margin", gint(aStyles.margin.left.as(css::length::pixels).value()), NULL);

	if (aStyles.margin.right.units() != css::length::inherit)
		g_object_set(G_OBJECT(tag), "right-margin-set", TRUE, "right-margin", gint(aStyles.margin.right.as(css::length::pixels).value()), NULL);

	return tag;
}

GtkTextBuffer *
cainteoir_document_create_buffer(CainteoirDocument *doc)
{
	GtkTextTagTable *tags = gtk_text_tag_table_new();
	GtkTextBuffer *buffer = gtk_text_buffer_new(tags);

	GtkTextIter position;
	gtk_text_buffer_get_end_iter(buffer, &position);

	std::list<std::string> anchor;
	std::stack<tag_block> contexts;
	bool need_linebreak = false;
	for (auto &entry : doc->priv->doc->children())
	{
		if (entry.type & events::begin_context)
		{
			GtkTextTag *tag = nullptr;
			if (entry.styles)
			{
				tag = gtk_text_tag_table_lookup(tags, entry.styles->name.c_str());
				if (!tag)
				{
					tag = create_text_tag_from_style(*entry.styles);
					gtk_text_tag_table_add(tags, tag);
				}
			}

			if (need_linebreak && entry.styles) switch (entry.styles->display)
			{
			case cainteoir::css::display::block:
			case cainteoir::css::display::line_break:
			case cainteoir::css::display::list_item:
			case cainteoir::css::display::table:
			case cainteoir::css::display::table_row:
			case cainteoir::css::display::table_cell:
				gtk_text_buffer_insert(buffer, &position, "\n", 1);
				gtk_text_buffer_get_end_iter(buffer, &position);
				need_linebreak = false;
				break;
			}
			contexts.push({ tag, gtk_text_iter_get_offset(&position) });
		}
		if (entry.type & cainteoir::events::anchor)
			anchor.push_back(entry.anchor.str());
		if (entry.type & cainteoir::events::text)
		{
			for (auto &a : anchor)
				(void)gtk_text_buffer_create_mark(buffer, a.c_str(), &position, TRUE);
			anchor.clear();

			const gchar *start = entry.content->begin();
			const gchar *end   = entry.content->end();
			while (start < end)
			{
				const gchar *next  = start;
				bool valid = g_utf8_validate(start, end - start, &next);
				if (start != next)
					gtk_text_buffer_insert(buffer, &position, start, next - start);
				if (!valid)
				{
					char text[20];
					int n = snprintf(text, 20, "<%02X>", (uint8_t)*next++);
					gtk_text_buffer_insert(buffer, &position, text, n);
				}
				start = next;
			}

			gtk_text_buffer_get_end_iter(buffer, &position);
			need_linebreak = true;
		}
		if (entry.type & events::end_context)
		{
			if (!contexts.empty())
			{
				GtkTextTag *tag = contexts.top().tag;
				if (tag)
				{
					GtkTextIter start;
					gtk_text_buffer_get_iter_at_offset(buffer, &start, contexts.top().offset);
					gtk_text_buffer_apply_tag(buffer, tag, &start, &position);
				}
				contexts.pop();
			}
		}
	}

	return buffer;
}

cainteoir::document *
cainteoir_document_get_document(CainteoirDocument *doc)
{
	if (!doc) return nullptr;
	return doc->priv->doc.get();
}

cainteoir::rdf::graph *
cainteoir_document_get_metadata(CainteoirDocument *doc)
{
	if (!doc) return nullptr;
	return &doc->priv->metadata;
}
