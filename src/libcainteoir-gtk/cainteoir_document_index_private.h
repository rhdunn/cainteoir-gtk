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

#ifndef CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_INDEX_PRIVATE_H
#define CAINTEOIR_GTK_CAINTEOIR_DOCUMENT_INDEX_PRIVATE_H

#include <cainteoir/document.hpp>

typedef std::pair<const cainteoir::rdf::uri, const cainteoir::rdf::uri>
        cainteoir_document_index_selection;

const std::vector<cainteoir::ref_entry> *
                                       cainteoir_document_index_get_listing(CainteoirDocumentIndex *index);

void                                   cainteoir_document_index_set_playing(CainteoirDocumentIndex *index,
                                                                            const cainteoir::ref_entry &entry);

cainteoir_document_index_selection     cainteoir_document_index_get_selection_uri(CainteoirDocumentIndex *index);

#endif
