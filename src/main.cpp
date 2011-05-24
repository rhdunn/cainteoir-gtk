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
#include <cainteoir/engines.hpp>
#include <cainteoir/document.hpp>
#include <cainteoir/platform.hpp>
#include <cainteoir/languages.hpp>
#include <locale.h>
#include <map>

#undef foreach_iter

#include <gtkmm.h>

namespace rdf = cainteoir::rdf;
namespace rql = cainteoir::rdf::query;

void format_time(char *s, int n, double seconds)
{
	int ms = int(seconds * 10.0) % 10;

	int minutes = floor(seconds / 60.0);
	seconds = seconds - (minutes * 60.0);

	int hours = floor(minutes / 60.0);
	minutes = minutes - (hours * 60.0);

	snprintf(s, n, "%02d:%02d:%02d.%01d", hours, minutes, (int)floor(seconds), ms);
}

void create_recent_filter(Gtk::RecentFilter & filter, const rdf::graph & aMetadata)
{
	rql::results formats = rql::select(
		rql::select(aMetadata, rql::predicate, rdf::rdf("type")),
		rql::object, rdf::tts("DocumentFormat"));

	for(auto format = formats.begin(), last = formats.end(); format != last; ++format)
	{
		const rdf::uri * uri = rql::subject(*format);

		rql::results mimetypes = rql::select(
			rql::select(aMetadata, rql::predicate, rdf::tts("mimetype")),
			rql::subject, *uri);

		for(auto mimetype = mimetypes.begin(), last = mimetypes.end(); mimetype != last; ++mimetype)
			filter.add_mime_type(rql::value(*mimetype));
	}
}

class application_settings
{
public:
	rdf::literal & operator()(const std::string & name, const rdf::literal & default_value = rdf::literal());
private:
	std::map<std::string, rdf::literal> values;
};

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

struct document : public cainteoir::document_events
{
	document()
		: tts(m_metadata)
		, m_doc(new cainteoir::document())
	{
	}

	void metadata(const rdf::statement &aStatement)
	{
		m_metadata.push_back(aStatement);
	}

	const rdf::bnode genid()
	{
		return m_metadata.genid();
	}

	void text(std::tr1::shared_ptr<cainteoir::buffer> aText)
	{
		m_doc->add(aText);
	}

	std::tr1::shared_ptr<const rdf::uri> subject;
	rdf::graph m_metadata;
	cainteoir::tts::engines tts;
	std::tr1::shared_ptr<cainteoir::document> m_doc;
};

class MetadataView : public Gtk::Frame
{
public:
	MetadataView(cainteoir::languages & lang);

	void clear();

	void add_metadata(const rdf::graph & aMetadata, const rdf::uri & aUri, const rdf::uri & aPredicate);
private:
	void create_entry(const rdf::uri & aPredicate, const char * labelText, int row);

	Gtk::Table metadata;
	std::map<std::string, std::pair<Gtk::Label *, Gtk::Label *> > values;
	cainteoir::languages & languages;
};

MetadataView::MetadataView(cainteoir::languages & lang)
	: metadata(5, 2, false)
	, languages(lang)
{
	set_label(_("Information"));
	add(metadata);

	create_entry(rdf::dc("title"), _("Title"), 0);
	create_entry(rdf::dc("creator"), _("Author"), 1);
	create_entry(rdf::dc("publisher"), _("Publisher"), 2);
	create_entry(rdf::dc("description"), _("Description"), 3);
	create_entry(rdf::dc("language"), _("Language"), 4);
}

void MetadataView::clear()
{
	for(auto item = values.begin(), last = values.end(); item != last; ++item)
		item->second.second->set_label("");
}

void MetadataView::add_metadata(const rdf::graph & aMetadata, const rdf::uri & aUri, const rdf::uri & aPredicate)
{
	rql::results selection = rql::select(aMetadata, rql::subject, aUri);
	for(auto query = selection.begin(), last = selection.end(); query != last; ++query)
	{
		const std::string &object = rql::value(*query);
		if (!object.empty())
		{
			if (rql::predicate(*query) == aPredicate)
			{
				if (aPredicate == rdf::dc("language"))
					values[aPredicate.str()].second->set_label(languages(object));
				else
					values[aPredicate.str()].second->set_label(object);
			}
		}
		else if (rql::predicate(*query) == rdf::dc("creator") && aPredicate == rdf::dc("creator"))
		{
			std::string role;
			std::string author;

			rql::results selection = rql::select(aMetadata, rql::subject, *rql::object(*query).as<rdf::bnode>());
			for(auto data = selection.begin(), last = selection.end(); data != last; ++data)
			{
				const std::string &object = rql::value(*data);
				if (rql::predicate(*data) == rdf::rdf("value"))
					author = object;
				else if (rql::predicate(*data) == rdf::opf("role"))
					role = object;
			}

			if (!author.empty() && (role == "aut" || role.empty()))
				values[aPredicate.str()].second->set_label(author);
		}
	}
}

void MetadataView::create_entry(const rdf::uri & aPredicate, const char * labelText, int row)
{
	Gtk::Label * label = Gtk::manage(new Gtk::Label(labelText));
	Gtk::Label * value = Gtk::manage(new Gtk::Label());

	values[aPredicate.str()] = std::make_pair(label, value);

	label->set_alignment(0, 0);
	value->set_alignment(0, 0);
	value->set_line_wrap(true);

	metadata.attach(*label, 0, 1, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);
	metadata.attach(*value, 1, 2, row, row+1, Gtk::FILL, Gtk::FILL, 4, 4);
}

class Cainteoir : public Gtk::Window
{
public:
	Cainteoir();

	bool load_document(std::string filename);
protected:
	void on_open_document();
	void on_recent_files_dialog();
	void on_recent_file(Gtk::RecentChooserMenu * recent);
	void on_quit();
	void on_read();
	void on_stop();

	bool on_speaking();
private:
	void updateProgress(double elapsed, double total, double completed);

	Gtk::VBox box;
	Gtk::VBox content;
	Gtk::HBox mediabar;
	MetadataView metadata;

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
	Glib::RefPtr<Gtk::Action> recentDialogAction;

	Gtk::MenuToolButton open;

	Glib::RefPtr<Gtk::Action> readAction;
	Glib::RefPtr<Gtk::Action> stopAction;
	Glib::RefPtr<Gtk::Action> openAction;

	document doc;
	cainteoir::languages languages;
	std::tr1::shared_ptr<cainteoir::tts::speech> speech;
	std::tr1::shared_ptr<cainteoir::audio> out;
	application_settings settings;
};

Cainteoir::Cainteoir()
	: mediabar(Gtk::ORIENTATION_HORIZONTAL, 4)
	, metadata(languages)
	, state(_("stopped"))
	, progressAlignment(0.5, 0.5, 1.0, 0.0)
	, open(Gtk::Stock::OPEN)
	, languages("en")
{
	set_title(_("Cainteoir Text-to-Speech"));
	set_size_request(600, 400);

	content.set_border_width(6);

	actions = Gtk::ActionGroup::create();
	uiManager = Gtk::UIManager::create();
	recentManager = Gtk::RecentManager::get_default();

	create_recent_filter(recentFilter, doc.m_metadata);

	actions->add(Gtk::Action::create("FileMenu", _("_File")));
	actions->add(openAction = Gtk::Action::create("FileOpen", Gtk::Stock::OPEN), sigc::mem_fun(*this, &Cainteoir::on_open_document));
	actions->add(recentAction = Gtk::Action::create("FileRecentFiles", _("_Recent Files")));
	actions->add(recentDialogAction = Gtk::Action::create("FileRecentDialog", _("Recent Files _Dialog")), sigc::mem_fun(*this, &Cainteoir::on_recent_files_dialog));
	actions->add(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT), sigc::mem_fun(*this, &Cainteoir::on_quit));

	actions->add(Gtk::Action::create("ReaderMenu", _("_Reader")));
	actions->add(readAction = Gtk::Action::create("ReaderRead", Gtk::Stock::MEDIA_PLAY), sigc::mem_fun(*this, &Cainteoir::on_read));
	actions->add(stopAction = Gtk::Action::create("ReaderStop", Gtk::Stock::MEDIA_STOP), sigc::mem_fun(*this, &Cainteoir::on_stop));

	uiManager->insert_action_group(actions);
	add_accel_group(uiManager->get_accel_group());

	uiManager->add_ui_from_string(
		"<ui>"
		"	<menubar name='MenuBar'>"
		"		<menu action='FileMenu'>"
		"			<menuitem action='FileOpen'/>"
		"			<menuitem action='FileRecentFiles'/>"
		"			<menuitem action='FileRecentDialog'/>"
		"			<separator/>"
		"			<menuitem action='FileQuit'/>"
		"		</menu>"
		"		<menu action='ReaderMenu'>"
		"			<menuitem action='ReaderRead'/>"
		"			<menuitem action='ReaderStop'/>"
		"		</menu>"
		"	</menubar>"
		"	<toolbar  name='ToolBar'>"
		"		<toolitem action='ReaderRead'/>"
		"		<toolitem action='ReaderStop'/>"
		"	</toolbar>"
		"</ui>");

	Gtk::RecentChooserMenu * recentToolbar = Gtk::manage(new Gtk::RecentChooserMenu(recentManager));
	recentToolbar->signal_item_activated().connect(sigc::bind(sigc::mem_fun(*this, &Cainteoir::on_recent_file), recentToolbar));
	recentToolbar->set_show_numbers(true);
	recentToolbar->set_sort_type(Gtk::RECENT_SORT_MRU);
	recentToolbar->set_filter(recentFilter);
	recentToolbar->set_limit(6);

	Gtk::Toolbar * toolbar = dynamic_cast<Gtk::Toolbar *>(uiManager->get_widget("/ToolBar"));
	toolbar->set_show_arrow(false);

	open.signal_clicked().connect(sigc::mem_fun(*this, &Cainteoir::on_open_document));
	open.set_menu(*recentToolbar);
	toolbar->insert(open, -1);

	Gtk::RecentChooserMenu * recentMenu = Gtk::manage(new Gtk::RecentChooserMenu(recentManager));
	recentMenu->signal_item_activated().connect(sigc::bind(sigc::mem_fun(*this, &Cainteoir::on_recent_file), recentMenu));
	recentMenu->set_show_numbers(true);
	recentMenu->set_sort_type(Gtk::RECENT_SORT_MRU);
	recentMenu->set_filter(recentFilter);
	recentMenu->set_limit(6);

	Gtk::MenuItem * openRecent = dynamic_cast<Gtk::MenuItem *>(uiManager->get_widget("/MenuBar/FileMenu/FileRecentFiles"));
	openRecent->set_submenu(*recentMenu);

	progressAlignment.add(progress);

	mediabar.pack_start(*toolbar, Gtk::PACK_SHRINK);
	mediabar.pack_start(elapsedTime, Gtk::PACK_SHRINK);
	mediabar.pack_start(progressAlignment);
	mediabar.pack_start(totalTime, Gtk::PACK_SHRINK);

	statusbar.pack_start(state, Gtk::PACK_SHRINK);

	content.pack_start(mediabar, Gtk::PACK_SHRINK);
	content.pack_start(metadata);
	content.pack_start(statusbar, Gtk::PACK_SHRINK);

	add(box);
	box.pack_start(*uiManager->get_widget("/MenuBar"), Gtk::PACK_SHRINK);
	box.pack_start(content);

	updateProgress(0.0, 0.0, 0.0);

	show_all_children();

	readAction->set_sensitive(false);
	stopAction->set_visible(false);
}

void Cainteoir::on_open_document()
{
	Gtk::FileChooserDialog dialog(_("Open Document"), Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_transient_for(*this);
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	dialog.set_filename(settings("document.filename").as<std::string>());

	rql::results formats = rql::select(
		rql::select(doc.m_metadata, rql::predicate, rdf::rdf("type")),
		rql::object, rdf::tts("DocumentFormat"));

	std::string default_mimetype = settings("document.mimetype", "text/plain").as<std::string>();

	for(auto format = formats.begin(), last = formats.end(); format != last; ++format)
	{
		const rdf::uri * uri = rql::subject(*format);

		Gtk::FileFilter filter;
		filter.set_name(rql::select_value<std::string>(doc.m_metadata, *uri, rdf::dc("title")));

		rql::results mimetypes = rql::select(
			rql::select(doc.m_metadata, rql::predicate, rdf::tts("mimetype")),
			rql::subject, *uri);

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
	dialog.add_button(_("Select Document"), Gtk::RESPONSE_OK);
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

	hide();
}

void Cainteoir::on_read()
{
	out = cainteoir::open_audio_device(NULL, "pulse", 0.3, doc.m_metadata, *doc.subject, doc.tts.voice());
	speech = doc.tts.speak(doc.m_doc, out, 0);

	readAction->set_visible(false);
	stopAction->set_visible(true);

	openAction->set_sensitive(false);
	recentAction->set_sensitive(false);
	recentDialogAction->set_sensitive(false);

	state.set_label(_("reading"));
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Cainteoir::on_speaking), 100);
}

void Cainteoir::on_stop()
{
	speech->stop();
	state.set_label(_("stopped"));
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

	updateProgress(0.0, 0.0, 0.0);

	readAction->set_visible(true);
	stopAction->set_visible(false);

	openAction->set_sensitive(true);
	recentAction->set_sensitive(true);
	recentDialogAction->set_sensitive(true);

	return false;
}

bool Cainteoir::load_document(std::string filename)
{
	if (speech) return false;

	readAction->set_sensitive(false);

	doc.m_doc->clear();
	doc.subject.reset();
	metadata.clear();

	try
	{
		if (filename.find("file://") == 0)
			filename.erase(0, 7);

		if (cainteoir::parseDocument(filename.c_str(), doc))
		{
			doc.subject = std::tr1::shared_ptr<const rdf::uri>(new rdf::uri(filename, std::string()));
			recentManager->add_item("file://" + filename);

			std::string mimetype = rql::select_value<std::string>(doc.m_metadata, *doc.subject, rdf::tts("mimetype"));

			settings("document.filename") = filename;
			if (!mimetype.empty())
				settings("document.mimetype") = mimetype;

			metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("title"));
			metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("creator"));
			metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("publisher"));
			metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("description"));
			metadata.add_metadata(doc.m_metadata, *doc.subject, rdf::dc("language"));

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

int main(int argc, char ** argv)
{
	cainteoir::initialise();

	setlocale(LC_MESSAGES, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	Gtk::Main app(argc, argv);

	Cainteoir window;
	if (argc > 1)
		window.load_document(argv[1]);

	Gtk::Main::run(window);

	cainteoir::cleanup();
	return 0;
}
