/* Document Library View
 *
 * Copyright (C) 2012 Reece H. Dunn
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

#ifndef CAINTEOIRGTK_SRC_DOCUMENTLIBRARY_HPP
#define CAINTEOIRGTK_SRC_DOCUMENTLIBRARY_HPP

#include <cainteoir/languages.hpp>
#include <cainteoir/document.hpp>

#include "settings.hpp"

class DocumentLibrary
{
public:
	DocumentLibrary(cainteoir::languages &aLanguages, GtkRecentManager *aRecent, rdf::graph &aMetadata);

	operator GtkWidget *() { return layout; }

	void update_recent(GtkRecentManager *aRecent, rdf::graph &aMetadata, int max_items_to_show);

	std::string get_filename() const;
private:
	GtkWidget *layout;
	GtkTreeStore *store;
	GtkTreeSelection *lib_selection;

	rdf::graph metadata;
};

#endif
