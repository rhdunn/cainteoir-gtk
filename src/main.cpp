/* Cainteoir Gtk Application.
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

#include <config.h>
#include <gtkmm.h>
#include <cainteoir/engines.hpp>
#include <cainteoir/document.hpp>
#include <cainteoir/platform.hpp>
#include <cainteoir/languages.hpp>
#include <locale.h>

#include <sys/stat.h>
#include <sys/types.h>

static const int CHARACTERS_PER_WORD = 6;

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;
namespace tts = cainteoir::tts;

#include "settings.hpp"
#include "document.hpp"
#include "voice_selection.hpp"

std::string get_user_file(const char * filename)
{
	std::string root = getenv("HOME") + std::string("/.cainteoir");
	mkdir(root.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);

	return root + "/" + std::string(filename);
}

void format_time(char *s, int n, double seconds)
{
	int ms = int(seconds * 10.0) % 10;

	int minutes = floor(seconds / 60.0);
	seconds = seconds - (minutes * 60.0);

	int hours = floor(minutes / 60.0);
	minutes = minutes - (hours * 60.0);

	snprintf(s, n, "%02d:%02d:%02d.%01d", hours, minutes, (int)floor(seconds), ms);
}

inline double estimate_time(size_t text_length, std::tr1::shared_ptr<tts::parameter> aRate)
{
	return (double)text_length / CHARACTERS_PER_WORD / (aRate ? aRate->value() : 170) * 60.0;
}

void create_recent_filter(Gtk::RecentFilter & filter, const rdf::graph & aMetadata)
{
	rql::results formats = rql::select(aMetadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object, rdf::tts("DocumentFormat"))));

	for(auto format = formats.begin(), last = formats.end(); format != last; ++format)
	{
		const rdf::uri * uri = rql::subject(*format);

		rql::results mimetypes = rql::select(aMetadata,
			rql::both(rql::matches(rql::predicate, rdf::tts("mimetype")),
			          rql::matches(rql::subject, *uri)));

		for(auto mimetype = mimetypes.begin(), last = mimetypes.end(); mimetype != last; ++mimetype)
			filter.add_mime_type(rql::value(*mimetype));
	}
}

class MetadataView : public Gtk::VBox
{
public:
	MetadataView(cainteoir::languages & lang, const char *label, int rows);

	void clear();

	void add_metadata(const rdf::graph & aMetadata, const rdf::uri & aUri, const rdf::uri & aPredicate);

	void add_metadata(const rdf::uri & aPredicate, const char * value);

	void create_entry(const rdf::uri & aPredicate, const char * labelText, int row);

private:
	Gtk::Label header;
	Gtk::Table metadata;
	std::map<std::string, std::pair<Gtk::Label *, Gtk::Label *> > values;
	cainteoir::languages & languages;
};

MetadataView::MetadataView(cainteoir::languages & lang, const char *label, int rows)
	: metadata(rows, 2, false)
	, languages(lang)
{
	pack_start(header, Gtk::PACK_SHRINK);
	pack_start(metadata, Gtk::PACK_SHRINK);

	set_border_width(6);
	metadata.set_border_width(4);

	header.set_alignment(0, 0);
	header.set_markup(label);
}

void MetadataView::clear()
{
	for(auto item = values.begin(), last = values.end(); item != last; ++item)
		item->second.second->set_label("");
}

void MetadataView::add_metadata(const rdf::graph & aMetadata, const rdf::uri & aUri, const rdf::uri & aPredicate)
{
	rql::results selection = rql::select(aMetadata, rql::matches(rql::subject, aUri));
	for(auto query = selection.begin(), last = selection.end(); query != last; ++query)
	{
		if (rql::predicate(*query).as<rdf::uri>()->ns == rdf::dc || rql::predicate(*query).as<rdf::uri>()->ns == rdf::dcterms)
		{
			rdf::any_type object = rql::object(*query);
			if (object.as<rdf::literal>())
			{
				if (rql::predicate(*query).as<rdf::uri>()->ref == aPredicate.ref)
				{
					if (aPredicate == rdf::dc("language"))
						values[aPredicate.str()].second->set_label(languages(rql::value(object)));
					else
						values[aPredicate.str()].second->set_label(rql::value(object));
				}
			}
			else
			{
				rql::results selection = rql::select(aMetadata, rql::matches(rql::subject, object));

				if (rql::predicate(*query).as<rdf::uri>()->ref == "creator" && aPredicate == rdf::dc("creator"))
				{
					std::string role;
					std::string author;

					for(auto data = selection.begin(), last = selection.end(); data != last; ++data)
					{
						const std::string &object = rql::value(*data);
						if (rql::predicate(*data) == rdf::rdf("value"))
							author = object;
						else if (rql::predicate(*data) == rdf::opf("role") || rql::predicate(*data) == rdf::pkg("role"))
							role = object;
					}

					if (!author.empty() && (role == "aut" || role.empty()))
						values[aPredicate.str()].second->set_label(author);
				}
				else if (rql::predicate(*query).as<rdf::uri>()->ref == aPredicate.ref)
				{
					for(auto data = selection.begin(), last = selection.end(); data != last; ++data)
					{
						const std::string &object = rql::value(*data);
						if (rql::predicate(*data) == rdf::rdf("value"))
						{
							if (aPredicate == rdf::dc("language"))
								values[aPredicate.str()].second->set_label(languages(object));
							else
								values[aPredicate.str()].second->set_label(object);
						}
					}
				}
			}
		}
	}
}

void MetadataView::add_metadata(const rdf::uri & aPredicate, const char * value)
{
	values[aPredicate.str()].second->set_label(value);
}

void MetadataView::create_entry(const rdf::uri & aPredicate, const char * labelText, int row)
{
	Gtk::Label * label = Gtk::manage(new Gtk::Label());
	Gtk::Label * value = Gtk::manage(new Gtk::Label());

	values[aPredicate.str()] = std::make_pair(label, value);

	label->set_alignment(0, 0);
	label->set_markup(labelText);

	value->set_alignment(0, 0);
	value->set_line_wrap(true);

	metadata.attach(*label, 0, 1, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);
	metadata.attach(*value, 1, 2, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);
}

class Cainteoir : public Gtk::Window
{
public:
	Cainteoir(const char *filename);

	bool load_document(std::string filename);
protected:
	bool on_window_state_changed(GdkEventWindowState *event);
	bool on_delete(GdkEventAny *event);

	void on_open_document();
	void on_recent_files_dialog();
	void on_recent_file(Gtk::RecentChooserMenu * recent);
	void on_quit();
	void on_read();
	void on_record();
	void on_stop();

	void on_speak(const char * state);
	bool on_speaking();

	enum view_t
	{
		metadata,
		voice_selection,
	};

	void switch_view(int aView);
private:
	void updateProgress(double elapsed, double total, double completed);
	Gtk::Menu *create_file_chooser_menu();

	Gtk::VBox box;
	Gtk::VBox content;
	Gtk::HBox mediabar;

	Gtk::HPaned pane;

	Gtk::ScrolledWindow scrolledView;
	Gtk::VBox view;
	MetadataView doc_metadata;
	VoiceSelectionView voiceSelection;

	Gtk::HBox statusbar;
	Gtk::Label state;

	Gtk::Alignment progressAlignment;
	Gtk::ProgressBar progress;
	Gtk::Label elapsedTime;
	Gtk::Label totalTime;

	Glib::RefPtr<Gtk::UIManager> uiManager;
	Glib::RefPtr<Gtk::ActionGroup> actions;

	Gtk::RecentFilter recentFilter;
	Glib::RefPtr<Gtk::RecentManager> recentManager;
	Glib::RefPtr<Gtk::Action> recentAction;

	Gtk::MenuToolButton open;

	Glib::RefPtr<Gtk::Action> readAction;
	Glib::RefPtr<Gtk::Action> stopAction;
	Glib::RefPtr<Gtk::Action> recordAction;
	Glib::RefPtr<Gtk::Action> openAction;

	document doc;
	cainteoir::languages languages;
	std::tr1::shared_ptr<cainteoir::tts::speech> speech;
	std::tr1::shared_ptr<cainteoir::audio> out;
	application_settings settings;
};

Cainteoir::Cainteoir(const char *filename)
	: mediabar(Gtk::ORIENTATION_HORIZONTAL, 4)
	, doc_metadata(languages, _("<b>Document</b>"), 5)
	, voiceSelection(doc.tts)
	, state(_("stopped"))
	, progressAlignment(0.5, 0.5, 1.0, 0.0)
	, open(Gtk::Stock::OPEN)
	, languages("en")
	, settings(get_user_file("settings.dat"))
{
	set_title(_("Cainteoir Text-to-Speech"));
	set_size_request(500, 300);

	resize(settings("window.width",  700).as<int>(), settings("window.height", 445).as<int>());
	move(settings("window.left", 0).as<int>(), settings("window.top",  0).as<int>());
	if (settings("window.maximized", "false").as<std::string>() == "true")
		maximize();

	signal_window_state_event().connect(sigc::mem_fun(*this, &Cainteoir::on_window_state_changed));
	signal_delete_event().connect(sigc::mem_fun(*this, &Cainteoir::on_delete));

	content.set_border_width(6);

	actions = Gtk::ActionGroup::create();
	uiManager = Gtk::UIManager::create();
	recentManager = Gtk::RecentManager::get_default();

	create_recent_filter(recentFilter, doc.m_metadata);

	actions->add(Gtk::Action::create("FileMenu", _("_File")));
	actions->add(openAction = Gtk::Action::create("FileOpen", Gtk::Stock::OPEN), sigc::mem_fun(*this, &Cainteoir::on_open_document));
	actions->add(recentAction = Gtk::Action::create("FileRecentFiles", _("_Recent Files")));
	actions->add(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT), sigc::mem_fun(*this, &Cainteoir::on_quit));

	actions->add(Gtk::Action::create("ViewMenu", _("_View")));
	actions->add(Gtk::Action::create("ViewMetadata", _("_Information")), sigc::bind(sigc::mem_fun(*this, &Cainteoir::switch_view), metadata));
	actions->add(Gtk::Action::create("SelectVoice", _("_Select Voice")), sigc::bind(sigc::mem_fun(*this, &Cainteoir::switch_view), voice_selection));

	actions->add(Gtk::Action::create("ReaderMenu", _("_Reader")));
	actions->add(readAction = Gtk::Action::create("ReaderRead", Gtk::Stock::MEDIA_PLAY), sigc::mem_fun(*this, &Cainteoir::on_read));
	actions->add(stopAction = Gtk::Action::create("ReaderStop", Gtk::Stock::MEDIA_STOP), sigc::mem_fun(*this, &Cainteoir::on_stop));
	actions->add(recordAction = Gtk::Action::create("ReaderRecord", Gtk::Stock::MEDIA_RECORD), sigc::mem_fun(*this, &Cainteoir::on_record));

	uiManager->insert_action_group(actions);
	add_accel_group(uiManager->get_accel_group());

	uiManager->add_ui_from_string(
		"<ui>"
		"	<menubar name='MenuBar'>"
		"		<menu action='FileMenu'>"
		"			<menuitem action='FileOpen'/>"
		"			<menuitem action='FileRecentFiles'/>"
		"			<separator/>"
		"			<menuitem action='FileQuit'/>"
		"		</menu>"
		"		<menu action='ViewMenu'>"
		"			<menuitem action='ViewMetadata'/>"
		"		</menu>"
		"		<menu action='ReaderMenu'>"
		"			<menuitem action='ReaderRead'/>"
		"			<menuitem action='ReaderStop'/>"
		"			<menuitem action='ReaderRecord'/>"
		"			<separator/>"
		"			<menuitem action='SelectVoice'/>"
		"		</menu>"
		"	</menubar>"
		"	<toolbar  name='ToolBar'>"
		"		<toolitem action='ReaderRead'/>"
		"		<toolitem action='ReaderStop'/>"
		"		<toolitem action='ReaderRecord'/>"
		"	</toolbar>"
		"</ui>");

	Gtk::Toolbar * toolbar = dynamic_cast<Gtk::Toolbar *>(uiManager->get_widget("/ToolBar"));
	toolbar->set_show_arrow(false);

	open.signal_clicked().connect(sigc::mem_fun(*this, &Cainteoir::on_open_document));
	open.set_menu(*create_file_chooser_menu());
	toolbar->insert(open, -1);

	Gtk::MenuItem * openRecent = dynamic_cast<Gtk::MenuItem *>(uiManager->get_widget("/MenuBar/FileMenu/FileRecentFiles"));
	openRecent->set_submenu(*create_file_chooser_menu());

	progressAlignment.add(progress);

	mediabar.pack_start(*toolbar, Gtk::PACK_SHRINK);
	mediabar.pack_start(elapsedTime, Gtk::PACK_SHRINK);
	mediabar.pack_start(progressAlignment);
	mediabar.pack_start(totalTime, Gtk::PACK_SHRINK);

	statusbar.pack_start(state, Gtk::PACK_SHRINK);

	doc_metadata.create_entry(rdf::dc("title"), _("<i>Title:</i>"), 0);
	doc_metadata.create_entry(rdf::dc("creator"), _("<i>Author:</i>"), 1);
	doc_metadata.create_entry(rdf::dc("publisher"), _("<i>Publisher:</i>"), 2);
	doc_metadata.create_entry(rdf::dc("description"), _("<i>Description:</i>"), 3);
	doc_metadata.create_entry(rdf::dc("language"), _("<i>Language:</i>"), 4);
	doc_metadata.create_entry(rdf::tts("length"), _("<i>Length:</i>"), 5);

	view.pack_start(doc_metadata, Gtk::PACK_SHRINK);
	view.pack_start(voiceSelection, Gtk::PACK_SHRINK);

	scrolledView.add(view);
	scrolledView.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	((Gtk::Viewport *)scrolledView.get_child())->set_shadow_type(Gtk::SHADOW_NONE);

	pane.add1(doc.toc);
	pane.add2(scrolledView);

	pane.set_position(settings("toc.width", 150).as<int>());

	content.pack_start(mediabar, Gtk::PACK_SHRINK);
	content.pack_start(pane);
	content.pack_start(statusbar, Gtk::PACK_SHRINK);

	add(box);
	box.pack_start(*uiManager->get_widget("/MenuBar"), Gtk::PACK_SHRINK);
	box.pack_start(content);

	updateProgress(0.0, estimate_time(doc.m_doc->text_length(), doc.tts.parameter(tts::parameter::rate)), 0.0);

	show_all_children();
	switch_view(settings("cainteoir.active-view",  metadata).as<int>());

	readAction->set_sensitive(false);
	stopAction->set_visible(false);

	load_document(filename ? std::string(filename) : settings("document.filename").as<std::string>());
}

bool Cainteoir::on_window_state_changed(GdkEventWindowState *event)
{
	settings("window.maximized") = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) ? "true" : "false";
	settings.save();
	return true;
}

bool Cainteoir::on_delete(GdkEventAny *event)
{
	on_quit();
	return true;
}

void Cainteoir::on_open_document()
{
	Gtk::FileChooserDialog dialog(_("Open Document"), Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_transient_for(*this);
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	dialog.set_filename(settings("document.filename").as<std::string>());

	rql::results formats = rql::select(doc.m_metadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object, rdf::tts("DocumentFormat"))));

	std::string default_mimetype = settings("document.mimetype", "text/plain").as<std::string>();

	for(auto format = formats.begin(), last = formats.end(); format != last; ++format)
	{
		rql::results data = rql::select(doc.m_metadata, rql::matches(rql::subject, rql::subject(*format)));

		Gtk::FileFilter filter;
		filter.set_name(rql::select_value<std::string>(data, rql::matches(rql::predicate, rdf::dc("title"))));

		rql::results mimetypes = rql::select(data, rql::matches(rql::predicate, rdf::tts("mimetype")));

		bool active_filter = false;
		for(auto item = mimetypes.begin(), last = mimetypes.end(); item != last; ++item)
		{
			const std::string & mimetype = rql::value(*item);
			filter.add_mime_type(mimetype);
			if (default_mimetype == mimetype)
				active_filter = true;
		}

 		dialog.add_filter(filter);
		if (active_filter)
			dialog.set_filter(filter);
	}

	if (dialog.run() == Gtk::RESPONSE_OK)
		load_document(dialog.get_filename());
}

void Cainteoir::on_recent_files_dialog()
{
	Gtk::RecentChooserDialog dialog(*this, _("Recent Files"), recentManager);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	dialog.set_filter(recentFilter);
	dialog.set_sort_type(Gtk::RECENT_SORT_MRU);

	const int response = dialog.run();
	dialog.hide();
	if (response == Gtk::RESPONSE_OK)
		load_document(dialog.get_current_uri());
}

void Cainteoir::on_recent_file(Gtk::RecentChooserMenu * recent)
{
	load_document(recent->get_current_uri());
}

void Cainteoir::on_quit()
{
	if (speech)
		speech->stop();

	settings("toc.width") = pane.get_position();
	if (settings("window.maximized", "false").as<std::string>() == "false")
	{
		int width = 0;
		int height = 0;
		int top = 0;
		int left = 0;

		get_position(left, top);
		get_size(width, height);

		settings("window.width")  = width;
		settings("window.height") = height;
		settings("window.top")    = top;
		settings("window.left")   = left;
	}
	settings.save();

	hide();
}

void Cainteoir::on_read()
{
	out = cainteoir::open_audio_device(NULL, "pulse", 0.3, doc.m_metadata, *doc.subject, doc.tts.voice());
	on_speak(_("reading"));
}

void Cainteoir::on_record()
{
	// TODO: Generate a default name from the file metadata ($(recording.basepath)/author - title.ogg)

	Gtk::FileChooserDialog dialog(_("Record Document"), Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_transient_for(*this);
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::MEDIA_RECORD, Gtk::RESPONSE_OK);
	dialog.set_filename(settings("recording.filename").as<std::string>());

	rql::results formats = rql::select(doc.m_metadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object, rdf::tts("AudioFormat"))));

	std::string default_mimetype = settings("recording.mimetype", "audio/ogg").as<std::string>();

	for(auto format = formats.begin(), last = formats.end(); format != last; ++format)
	{
		rql::results data = rql::select(doc.m_metadata, rql::matches(rql::subject, rql::subject(*format)));

		Gtk::FileFilter filter;
		filter.set_name(rql::select_value<std::string>(data, rql::matches(rql::predicate, rdf::dc("title"))));

		rql::results mimetypes = rql::select(data, rql::matches(rql::predicate, rdf::tts("mimetype")));

		bool active_filter = false;
		for(auto item = mimetypes.begin(), last = mimetypes.end(); item != last; ++item)
		{
			const std::string & mimetype = rql::value(*item);
			filter.add_mime_type(mimetype);
			if (default_mimetype == mimetype)
				active_filter = true;
		}

 		dialog.add_filter(filter);
		if (active_filter)
			dialog.set_filter(filter);
	}

	if (dialog.run() != Gtk::RESPONSE_OK)
		return;

	// FIXME: Get the correct mimetype for the recording file.
	// FIXME: Get the correct audio type (wav, ogg, ...) for the recording file.

	std::string filename = dialog.get_filename();
	std::string mimetype = "audio/ogg";
	std::string type = "ogg";

	settings("recording.filename") = filename;
	settings("recordig.mimetype") = mimetype;
	settings.save();

	out = cainteoir::create_audio_file(filename.c_str(), type.c_str(), 0.3, doc.m_metadata, *doc.subject, doc.tts.voice());
	on_speak(_("recording"));
}

void Cainteoir::on_speak(const char * status)
{
	cainteoir::document::range_type selection = doc.selection();
	speech = doc.tts.speak(doc.m_doc, out, selection.first, selection.second);

	readAction->set_visible(false);
	stopAction->set_visible(true);
	recordAction->set_sensitive(false);

	open.set_sensitive(false);
	openAction->set_sensitive(false);
	recentAction->set_sensitive(false);

	state.set_label(status);
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Cainteoir::on_speaking), 100);
}

void Cainteoir::on_stop()
{
	speech->stop();
}

bool Cainteoir::on_speaking()
{
	if (speech->is_speaking())
	{
		updateProgress(speech->elapsedTime(), speech->totalTime(), speech->completed());
		return true;
	}

	speech.reset();
	out.reset();

	updateProgress(0.0, estimate_time(doc.m_doc->text_length(), doc.tts.parameter(tts::parameter::rate)), 0.0);

	state.set_label(_("stopped"));

	readAction->set_visible(true);
	stopAction->set_visible(false);
	recordAction->set_sensitive(true);

	open.set_sensitive(true);
	openAction->set_sensitive(true);
	recentAction->set_sensitive(true);

	return false;
}

bool Cainteoir::load_document(std::string filename)
{
	if (speech || filename.empty()) return false;

	readAction->set_sensitive(false);

	doc.clear();
	doc_metadata.clear();

	try
	{
		if (filename.find("file://") == 0)
			filename.erase(0, 7);

		if (cainteoir::parseDocument(filename.c_str(), doc))
		{
			doc.subject = std::tr1::shared_ptr<const rdf::uri>(new rdf::uri(filename, std::string()));
			recentManager->add_item("file://" + filename);

			rql::results data = rql::select(doc.m_metadata, rql::matches(rql::subject, *doc.subject));
			std::string mimetype = rql::select_value<std::string>(data, rql::matches(rql::predicate, rdf::tts("mimetype")));
			std::string title    = rql::select_value<std::string>(data, rql::matches(rql::predicate, rdf::dc("title")));

			if (title.empty())
				set_title(_("Cainteoir Text-to-Speech"));
			else
			{
				char buf[1024];
				snprintf(buf, sizeof(buf), _("%1$s - Cainteoir Text-to-Speech"), title.c_str());
				set_title(buf);
			}

			settings("document.filename") = filename;
			if (!mimetype.empty())
				settings("document.mimetype") = mimetype;
			settings.save();

			doc_metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("title"));
			doc_metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("creator"));
			doc_metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("publisher"));
			doc_metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("description"));
			doc_metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("language"));

			std::ostringstream length;
			length << (doc.m_doc->text_length() / CHARACTERS_PER_WORD) << _(" words (approx.)");

			doc_metadata.add_metadata(rdf::tts("length"), length.str().c_str());

			updateProgress(0.0, estimate_time(doc.m_doc->text_length(), doc.tts.parameter(tts::parameter::rate)), 0.0);

			readAction->set_sensitive(true);
			return true;
		}
	}
	catch (const std::exception & e)
	{
		Gtk::MessageDialog dialog(*this, _("Unable to load the document"), false, Gtk::MESSAGE_ERROR);
		dialog.set_title(_("Open Document"));
		dialog.set_secondary_text(e.what());
		dialog.run();
	}

	return false;
}

void Cainteoir::updateProgress(double elapsed, double total, double completed)
{
	char percentage[20];
	char elapsed_time[80];
	char total_time[80];

	sprintf(percentage, "%0.2f%%", completed);
	format_time(elapsed_time, 80, elapsed);
	format_time(total_time, 80, total);

	progress.set_text(percentage);
	progress.set_fraction(completed / 100.0);

	elapsedTime.set_text(elapsed_time);
	totalTime.set_text(total_time);
}

Gtk::Menu *Cainteoir::create_file_chooser_menu()
{
	Gtk::RecentChooserMenu *recent = Gtk::manage(new Gtk::RecentChooserMenu(recentManager));

	recent->signal_item_activated().connect(sigc::bind(sigc::mem_fun(*this, &Cainteoir::on_recent_file), recent));
	recent->set_show_numbers(true);
	recent->set_sort_type(Gtk::RECENT_SORT_MRU);
	recent->set_filter(recentFilter);
	recent->set_limit(6);

	Gtk::MenuItem *separator = Gtk::manage(new Gtk::SeparatorMenuItem());
	recent->append(*separator);
	separator->show();

	Gtk::MenuItem *more = Gtk::manage(new Gtk::MenuItem(_("_More..."), true));
	more->signal_activate().connect(sigc::mem_fun(*this, &Cainteoir::on_recent_files_dialog));
	recent->append(*more);
	more->show();

	return recent;
}

void Cainteoir::switch_view(int aView)
{
	switch (aView)
	{
	case voice_selection:
		doc_metadata.hide();
		voiceSelection.show();
		break;
	default: // metadata
		aView = metadata;
		doc_metadata.show();
		voiceSelection.hide();
		break;
	}

	settings("cainteoir.active-view") = aView;
	settings.save();
}

int main(int argc, char ** argv)
{
	cainteoir::initialise();

	setlocale(LC_MESSAGES, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	Gtk::Main app(argc, argv);

	Cainteoir window(argc > 1 ? argv[1] : NULL);
	Gtk::Main::run(window);

	cainteoir::cleanup();
	return 0;
}
