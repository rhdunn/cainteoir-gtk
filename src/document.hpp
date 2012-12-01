/* Document Processing
 *
 * Copyright (C) 2011-2012 Reece H. Dunn
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

#ifndef CAINTEOIRGTK_SRC_DOCUMENT_HPP
#define CAINTEOIRGTK_SRC_DOCUMENT_HPP

#include <cainteoir/document.hpp>
#include <cainteoir/engines.hpp>
#include <cainteoir/content.hpp>

#include "toc.hpp"
#include <vector>

GtkTextTag *create_text_tag_from_style(const cainteoir::styles &aStyles);

struct toc_entry_data
{
	int depth;
	rdf::uri location;
	std::string title;

	toc_entry_data(int aDepth, const rdf::uri &aLocation, const std::string &aTitle)
		: depth(aDepth)
		, location(aLocation)
		, title(aTitle)
	{
	}
};

struct document : public cainteoir::document
{
	std::vector<toc_entry_data> toc;
	std::shared_ptr<const rdf::uri> subject;
	rdf::graph metadata;
};

#endif
