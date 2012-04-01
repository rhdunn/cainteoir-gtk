/* Application Settings
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

#include "config.h"
#include "compatibility.hpp"

#include <cainteoir/metadata.hpp>
#include <fstream>

#include "settings.hpp"

application_settings::application_settings(const std::string &aFilename)
	: filename(aFilename)
{
	std::ifstream is(filename.c_str());
	std::string key;
	std::string value;
	while (std::getline(is, key, '=') && std::getline(is, value))
		values[key] = rdf::literal(value);
}

void application_settings::save()
{
	std::ofstream os(filename.c_str());
	for (auto item = values.begin(), last = values.end(); item != last; ++item)
		os << item->first << '=' << item->second.value << std::endl;
}

rdf::literal & application_settings::operator()(const std::string & name, const rdf::literal & default_value)
{
	for (auto item = values.begin(), last = values.end(); item != last; ++item)
	{
		if (item->first == name)
			return item->second;
	}

	rdf::literal & value = values[name];
	value = default_value;
	return value;
}
