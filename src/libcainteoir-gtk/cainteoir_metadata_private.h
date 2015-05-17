/* A GTK+ wrapper around cainteoir RDF metadata results.
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

#ifndef CAINTEOIR_GTK_CAINTEOIR_METADATA_PRIVATE_H
#define CAINTEOIR_GTK_CAINTEOIR_METADATA_PRIVATE_H

#include <cainteoir/metadata.hpp>

CainteoirMetadata *                    cainteoir_metadata_new(const std::shared_ptr<cainteoir::rdf::graph> &metadata,
                                                              const cainteoir::rdf::uri &subject);

#endif
