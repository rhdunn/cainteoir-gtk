/* A GTK+ wrapper around the cainteoir::document class.
 *
 * Copyright (C) 2011-2013 Reece H. Dunn
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

#ifndef CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_PRIVATE_H
#define CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_PRIVATE_H

#include <cainteoir/document.hpp>
#include <cainteoir/content.hpp>

GtkTextTag *                           create_text_tag_from_style(const cainteoir::css::styles &aStyles);

GtkTextBuffer *                        cainteoir_document_create_buffer(CainteoirDocument *doc);

cainteoir::document *                  cainteoir_document_get_document(CainteoirDocument *doc);

cainteoir::rdf::graph *                cainteoir_document_get_metadata(CainteoirDocument *doc);

#endif
