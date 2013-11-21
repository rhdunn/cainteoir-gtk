/* Settings View
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

#ifndef CAINTEOIRGTK_SRC_SETTINGSVIEW_HPP
#define CAINTEOIRGTK_SRC_SETTINGSVIEW_HPP

#include <cainteoir/engines.hpp>
#include <cainteoir/languages.hpp>

#include "settings.hpp"

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;
namespace tts = cainteoir::tts;

struct VoiceParameter
{
	tts::parameter::type type;
	const char *id;
	GtkWidget *label;
	GtkWidget *param;
	GtkWidget *units;
};

class SettingsView
{
public:
	SettingsView(application_settings &aSettings, tts::engines &aEngines, GtkBuilder *ui);

	void show();

	void apply();
private:
	GtkWidget *layout;

	std::list<VoiceParameter> parameters;
	tts::engines *mEngines;
	application_settings &settings;

	GtkWidget *parameterView;
};

#endif
