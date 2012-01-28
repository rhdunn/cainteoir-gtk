/* Document Processing
 *
 * Copyright (C) 2011 Reece H. Dunn
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

#include "toc.hpp"

struct document : public cainteoir::document_events
{
	document()
		: tts(tts_metadata, cainteoir::text_support)
		, doc(new cainteoir::document())
	{
	}

	void metadata(const std::tr1::shared_ptr<const rdf::triple> &aStatement)
	{
		doc_metadata.push_back(aStatement);
	}

	const rdf::uri genid()
	{
		return doc_metadata.genid();
	}

	void text(std::tr1::shared_ptr<cainteoir::buffer> aText)
	{
		doc->add(aText);
	}

	void toc_entry(int depth, const rdf::uri &location, const std::string &title)
	{
		toc.add(depth, location, title);
	}

	void anchor(const rdf::uri &location, const std::string &mimetype)
	{
		doc->add_anchor(location);
	}

	void clear()
	{
		doc->clear();
		subject.reset();
		toc.clear();
	}

	cainteoir::document::range_type selection() const
	{
		return doc->children(toc.selection());
	}

	rdf::graph tts_metadata;
	cainteoir::tts::engines tts;

	std::tr1::shared_ptr<const rdf::uri> subject;
	rdf::graph doc_metadata;
	std::tr1::shared_ptr<cainteoir::document> doc;

	TocPane toc;
};

#endif
