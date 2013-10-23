/* Voice Selection View
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

#ifndef CAINTEOIRGTK_SRC_VOICESELECTION_HPP
#define CAINTEOIRGTK_SRC_VOICESELECTION_HPP

#include <cainteoir/engines.hpp>
#include <cainteoir/languages.hpp>

#include "settings.hpp"

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;
namespace tts = cainteoir::tts;

class VoiceList
{
public:
	VoiceList(application_settings &aSettings, rdf::graph &aMetadata, cainteoir::languages &languages);

	void set_voice(const rdf::uri &voice);

	const rdf::uri get_voice() const;

	void set_language(const std::string &language);

	void filter_by_doc_lang(bool filter);

	operator GtkWidget *() { return layout; }
private:
	void refresh();

	void add_voice(rdf::graph &aMetadata, rql::results &voice, cainteoir::languages &languages);

	GtkWidget *layout;
	GtkTreeStore *store;
	GtkTreeSelection *selection;

	application_settings &settings;
	rdf::graph &mMetadata;

	cainteoir::languages &languages;
	cainteoir::language::tag doc_lang;
	bool filter_by_doc_language;
	rdf::uri selected_voice;
};

struct VoiceParameter
{
	tts::parameter::type type;
	const char *id;
	GtkWidget *label;
	GtkWidget *param;
	GtkWidget *units;
};

class VoiceSelectionView
{
public:
	VoiceSelectionView(application_settings &aSettings, tts::engines &aEngines, rdf::graph &aMetadata, cainteoir::languages &aLanguages, GtkBuilder *ui);

	void show(const rdf::uri &voice);

	void set_language(const std::string &lang) { voices.set_language(lang); }

	sigc::signal<bool, const rdf::uri &> &signal_on_voice_change() { return on_voice_change; }

	void apply();
private:
	GtkWidget *layout;

	std::list<VoiceParameter> parameters;
	tts::engines *mEngines;
	application_settings &settings;

	VoiceList voices;
	GtkWidget *parameterView;

	sigc::signal<bool, const rdf::uri &> on_voice_change;
};

#endif
