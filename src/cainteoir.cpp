/* Cainteoir Main Window
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

#include <config.h>
#include <gtk/gtk.h>
#include <sigc++/signal.h>
#include <cainteoir/platform.hpp>

#include "cainteoir.hpp"
#include "gtk-compatibility.hpp"

#include <stdexcept>

#include <sys/stat.h>
#include <sys/types.h>

namespace rql = cainteoir::rdf::query;

static const int CHARACTERS_PER_WORD = 6;

static std::string get_user_file(const char * filename)
{
	std::string root = getenv("HOME") + std::string("/.cainteoir");
	mkdir(root.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);

	return root + "/" + std::string(filename);
}

static double estimate_time(size_t text_length, std::shared_ptr<tts::parameter> aRate)
{
	return (double)text_length / CHARACTERS_PER_WORD / (aRate ? aRate->value() : 170) * 60.0;
}

static void display_error_message(GtkWindow *window, const char *title, const char *text, const char *secondary_text)
{
	GtkWidget *dialog = gtk_message_dialog_new(window,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		"%s", text);
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", secondary_text);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

static std::string select_file(
	GtkWindow *window,
	const char *title,
	GtkFileChooserAction action,
	const char *open_id,
	const char *filename,
	std::string default_mimetype,
	rdf::graph &metadata,
	rql::results &formats)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new(title, window, action,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		open_id,          GTK_RESPONSE_OK,
		NULL);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), filename);

	for(auto format = formats.begin(), last = formats.end(); format != last; ++format)
	{
		rql::results data = rql::select(metadata, rql::matches(rql::subject, rql::subject(*format)));

		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, rql::select_value<std::string>(data, rql::matches(rql::predicate, rdf::dc("title"))).c_str());

		rql::results mimetypes = rql::select(data, rql::matches(rql::predicate, rdf::tts("mimetype")));

		bool active_filter = false;
		for(auto item = mimetypes.begin(), last = mimetypes.end(); item != last; ++item)
		{
			const std::string & mimetype = rql::value(*item);
			gtk_file_filter_add_mime_type(filter, mimetype.c_str());
			if (default_mimetype == mimetype)
				active_filter = true;
		}

		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
		if (active_filter)
			gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);
	}

	std::string ret;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
	{
		char *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (path)
		{
			ret = path;
			g_free(path);
		}
	}

	gtk_widget_destroy(dialog);
	return ret;
}

static GtkRecentFilter *create_recent_filter(const rdf::graph & aMetadata)
{
	GtkRecentFilter *filter = gtk_recent_filter_new();

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
			gtk_recent_filter_add_mime_type(filter, rql::value(*mimetype).c_str());
	}

	return filter;
}

static GtkWidget *create_padded_container(GtkWidget *child, int padding_width, int padding_height)
{
	GtkWidget *left_right = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(left_right), child, TRUE, TRUE, padding_width);

	GtkWidget *top_bottom = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(top_bottom), left_right, TRUE, TRUE, padding_height);

	return top_bottom;
}

typedef void (Cainteoir::*callback_function)();

struct CallbackData
{
	Cainteoir *window;
	callback_function callback;
};

static void on_button_clicked(GtkWidget *widget, void *data)
{
	CallbackData *cbd = (CallbackData *)data;
	((cbd->window)->*(cbd->callback))();
}

static GtkWidget *create_stock_button(const char *stock, Cainteoir *window, callback_function callback, GtkWidget *menu=NULL)
{
	CallbackData *data = g_slice_new(CallbackData);
	data->window   = window;
	data->callback = callback;

	GtkToolItem *button = NULL;
	if (menu)
	{
		button = gtk_menu_tool_button_new_from_stock(stock);
		gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(button), menu);
	}
	else
	{
		button = gtk_tool_button_new_from_stock(stock);
	}
	gtk_container_set_border_width(GTK_CONTAINER(button), 0);
	g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), data);
	return GTK_WIDGET(button);
}

static void on_recent_item_activated(GtkRecentChooser *chooser, void *data)
{
	GtkRecentInfo *info = gtk_recent_chooser_get_current_item(chooser);
	gchar *filename = gtk_recent_info_get_uri_display(info);
	if (filename)
	{
		((Cainteoir *)data)->load_document(filename);
		g_free(filename);
	}
	gtk_recent_info_unref(info);
}

static void on_recent_files_dialog(GtkMenuItem *item, void *data)
{
	Cainteoir &window = *(Cainteoir *)data;

	GtkWidget *dialog = gtk_recent_chooser_dialog_new(_("Recent Documents"), window,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN,   GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_window_resize(GTK_WINDOW(dialog), 500, 200);
	gtk_recent_chooser_set_filter(GTK_RECENT_CHOOSER(dialog), window.recent_filter());
	gtk_recent_chooser_set_sort_type(GTK_RECENT_CHOOSER(dialog), GTK_RECENT_SORT_MRU);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		on_recent_item_activated(GTK_RECENT_CHOOSER(dialog), data);

	gtk_widget_destroy(dialog);
}

static gboolean on_window_state_changed(GtkWidget *widget, GdkEvent *event, void *data)
{
	application_settings &settings = *(application_settings *)data;
	settings("window.maximized") = (((GdkEventWindowState *)event)->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) ? "true" : "false";
	settings.save();
	return TRUE;
}

static gboolean on_window_delete(GtkWidget *widget, GdkEvent *event, void *data)
{
	Cainteoir &app = *(Cainteoir *)data;
	app.stop();
	app.save_settings();

	gtk_widget_hide(widget);
	gtk_main_quit();
	return TRUE;
}

static gboolean on_timer_speaking(Cainteoir *window)
{
	return window->on_speaking();
}

Cainteoir::Cainteoir(const char *filename)
	: tts(tts_metadata, cainteoir::text_support)
	, doc(new document())
	, doc_metadata(languages, _("<b>Document</b>"), 5)
	, voice_metadata(languages, _("<b>Voice</b>"), 2)
	, engine_metadata(languages, _("<b>Engine</b>"), 2)
	, languages("en")
	, settings(get_user_file("settings.dat"))
{
	voiceSelection = std::shared_ptr<VoiceSelectionView>(new VoiceSelectionView(settings, tts, tts_metadata, languages));
	voiceSelection->signal_on_voice_change().connect(sigc::mem_fun(*this, &Cainteoir::switch_voice));

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), _("Cainteoir Text-to-Speech"));
	gtk_window_set_hide_titlebar_when_maximized(GTK_WINDOW(window), TRUE);

	gtk_widget_set_size_request(window, 500, 300);

	gtk_window_resize(GTK_WINDOW(window), settings("window.width",  700).as<int>(), settings("window.height", 445).as<int>());
	gtk_window_move(GTK_WINDOW(window), settings("window.left", 0).as<int>(), settings("window.top",  0).as<int>());
	if (settings("window.maximized", "false").as<std::string>() == "true")
		gtk_window_maximize(GTK_WINDOW(window));

	g_signal_connect(window, "window-state-event", G_CALLBACK(on_window_state_changed), &settings);
	g_signal_connect(window, "delete-event", G_CALLBACK(on_window_delete), this);

	recentManager = gtk_recent_manager_get_default();
	recentFilter = create_recent_filter(tts_metadata);

	readButton   = create_stock_button(GTK_STOCK_MEDIA_PLAY,   this, &Cainteoir::read);
	stopButton   = create_stock_button(GTK_STOCK_MEDIA_STOP,   this, &Cainteoir::stop);
	recordButton = create_stock_button(GTK_STOCK_MEDIA_RECORD, this, &Cainteoir::record);
	openButton   = create_stock_button(GTK_STOCK_OPEN,         this, &Cainteoir::on_open_document, create_file_chooser_menu());

	GtkWidget *topbar = gtk_toolbar_new();
	gtk_widget_set_size_request(topbar, 0, 40);
	gtk_style_context_add_class(gtk_widget_get_style_context(topbar), GTK_STYLE_CLASS_MENUBAR);

	GtkToolItem *navbar_item = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(navbar_item), navbar);
	gtk_container_add(GTK_CONTAINER(topbar), GTK_WIDGET(navbar_item));

	GtkToolItem *expander = gtk_tool_item_new();
	gtk_tool_item_set_expand(GTK_TOOL_ITEM(expander), TRUE);
	gtk_container_add(GTK_CONTAINER(topbar), GTK_WIDGET(expander));

	gtk_container_add(GTK_CONTAINER(topbar), GTK_WIDGET(openButton));

	GtkWidget *bottombar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_size_request(bottombar, 0, 40);
	gtk_box_pack_start(GTK_BOX(bottombar), GTK_WIDGET(readButton), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(bottombar), GTK_WIDGET(stopButton), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(bottombar), GTK_WIDGET(recordButton), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(bottombar), timebar, TRUE, TRUE, 0);

	doc_metadata.create_entry(rdf::dc("title"), _("Title"), 0);
	doc_metadata.create_entry(rdf::dc("creator"), _("Author"), 1);
	doc_metadata.create_entry(rdf::dc("publisher"), _("Publisher"), 2);
	doc_metadata.create_entry(rdf::dc("description"), _("Description"), 3);
	doc_metadata.create_entry(rdf::dc("language"), _("Language"), 4);
	doc_metadata.create_entry(rdf::tts("length"), _("Length"), 5);

	voice_metadata.create_entry(rdf::tts("name"), _("Name"), 0);
	voice_metadata.create_entry(rdf::dc("language"), _("Language"), 1);

	engine_metadata.create_entry(rdf::tts("name"), _("Name"), 0);
	engine_metadata.create_entry(rdf::tts("version"), _("Version"), 1);

	metadata_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(metadata_view), doc_metadata, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(metadata_view), voice_metadata, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(metadata_view), engine_metadata, FALSE, FALSE, 0);

	GtkWidget *toc_pane = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_size_request(toc_pane, 300, 0);
	gtk_box_pack_start(GTK_BOX(toc_pane), toc, TRUE, TRUE, 0);

	GtkWidget *pane = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
	gtk_box_pack_start(GTK_BOX(pane), toc_pane, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pane), metadata_view, TRUE, TRUE, 0);

	view = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(view), FALSE);

	int doc_page = gtk_notebook_append_page(GTK_NOTEBOOK(view), create_padded_container(pane, 5, 5), NULL);
	navbar.set_active_button(navbar.add_paged_button(_("Document"), GTK_NOTEBOOK(view), doc_page));

	int voice_page = gtk_notebook_append_page(GTK_NOTEBOOK(view), GTK_WIDGET(voiceSelection->gobj()),  NULL);
	navbar.add_paged_button(_("Voice"), GTK_NOTEBOOK(view), voice_page);

	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), box);
	gtk_box_pack_start(GTK_BOX(box), topbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), view, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), bottombar, FALSE, FALSE, 0);

	timebar.update(0.0, estimate_time(doc->text_length(), tts.parameter(tts::parameter::rate)), 0.0);

	gtk_widget_show_all(window);

	gtk_widget_set_sensitive(readButton, FALSE);
	gtk_widget_set_sensitive(recordButton, FALSE);
	gtk_widget_set_visible(stopButton, FALSE);

	load_document(filename ? std::string(filename) : settings("document.filename").as<std::string>(), true);
	switch_voice(tts.voice());
}

void Cainteoir::on_open_document()
{
	rql::results formats = rql::select(tts_metadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object, rdf::tts("DocumentFormat"))));

	std::string filename = select_file(GTK_WINDOW(window),
		_("Open Document"),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_OPEN,
		settings("document.filename").as<std::string>().c_str(),
		settings("document.mimetype", "text/plain").as<std::string>(),
		tts_metadata, formats);

	if (!filename.empty())
		load_document(filename);
}

void Cainteoir::save_settings()
{
	if (settings("window.maximized", "false").as<std::string>() == "false")
	{
		int width = 0;
		int height = 0;
		int top = 0;
		int left = 0;

		gtk_window_get_position(GTK_WINDOW(window), &left, &top);
		gtk_window_get_size(GTK_WINDOW(window), &width, &height);

		settings("window.width")  = width;
		settings("window.height") = height;
		settings("window.top")    = top;
		settings("window.left")   = left;
	}
	settings.save();
}

void Cainteoir::read()
{
	out = cainteoir::open_audio_device(NULL, "pulse", 0.3, doc->metadata, *doc->subject, tts_metadata, tts.voice());
	on_speak(_("reading"));
}

void Cainteoir::record()
{
	// TODO: Generate a default name from the file metadata ($(recording.basepath)/author - title.ogg)

	rql::results formats = rql::select(tts_metadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object, rdf::tts("AudioFormat"))));

	std::string filename = select_file(GTK_WINDOW(window),
		_("Record Document"),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_MEDIA_RECORD,
		settings("recording.filename").as<std::string>().c_str(),
		settings("recording.mimetype", "audio/x-vorbis+ogg").as<std::string>(),
		tts_metadata, formats);

	if (filename.empty())
		return;

	std::string::size_type extpos = filename.rfind('.');
	if (extpos != std::string::npos)
	{
		std::string ext = '*' + filename.substr(extpos);

		rql::results filetypes = rql::select(tts_metadata,
			rql::both(rql::matches(rql::predicate, rdf::tts("extension")),
			          rql::matches(rql::object, rdf::literal(ext))));

		if (filetypes.size() == 1)
		{
			const rdf::uri *uri = rql::subject(filetypes.front());
			if (uri)
			{
				std::string type = rql::select_value<std::string>(tts_metadata,
					rql::both(rql::matches(rql::subject, *uri),
					          rql::matches(rql::predicate, rdf::tts("name"))));

				std::string mimetype = rql::select_value<std::string>(tts_metadata,
					rql::both(rql::matches(rql::subject, *uri),
					          rql::matches(rql::predicate, rdf::tts("mimetype"))));

				settings("recording.filename") = filename;
				settings("recording.mimetype") = mimetype;
				settings.save();

				out = cainteoir::create_audio_file(filename.c_str(), type.c_str(), 0.3, doc->metadata, *doc->subject, tts_metadata, tts.voice());
				on_speak(_("recording"));
				return;
			}
		}
	}

	display_error_message(GTK_WINDOW(window),
		_("Record Document"),
		_("Unable to record the document"),
		_("Unsupported file type."));
}

void Cainteoir::stop()
{
	if (speech)
		speech->stop();
}

void Cainteoir::on_speak(const char * status)
{
	cainteoir::document::range_type selection = doc->children(toc.selection());
	speech = tts.speak(doc, out, selection.first, selection.second);

	gtk_widget_set_visible(readButton, FALSE);
	gtk_widget_set_visible(stopButton, TRUE);
	gtk_widget_set_sensitive(recordButton, FALSE);
	gtk_widget_set_sensitive(openButton, FALSE);

	g_timeout_add(100, (GSourceFunc)on_timer_speaking, this);
}

bool Cainteoir::on_speaking()
{
	if (speech->is_speaking())
	{
		timebar.update(speech->elapsedTime(), speech->totalTime(), speech->completed());
		return true;
	}

	std::string error = speech->error_message();
	if (!error.empty())
	{
		display_error_message(GTK_WINDOW(window),
			_("Cainteoir Text-to-Speech"),
			_("Error speaking the document"),
			error.c_str());
	}

	speech.reset();
	out.reset();

	timebar.update(0.0, estimate_time(doc->text_length(), tts.parameter(tts::parameter::rate)), 0.0);

	gtk_widget_set_visible(readButton, TRUE);
	gtk_widget_set_visible(stopButton, FALSE);
	gtk_widget_set_sensitive(recordButton, TRUE);
	gtk_widget_set_sensitive(openButton, TRUE);

	return false;
}

bool Cainteoir::load_document(std::string filename, bool from_constructor)
{
	if (speech || filename.empty()) return false;

	gtk_widget_set_sensitive(readButton, FALSE);
	gtk_widget_set_sensitive(recordButton, FALSE);

	bool ret = true;
	try
	{
		if (filename.find("file://") == 0)
			filename.erase(0, 7);

		document_builder newdoc;
		if (!cainteoir::parseDocument(filename.c_str(), newdoc, newdoc.doc->metadata))
			throw std::runtime_error(_("Document type is not supported"));
		doc = newdoc.doc;

		toc.clear();
		doc_metadata.clear();

		doc->subject = std::shared_ptr<const rdf::uri>(new rdf::uri(filename, std::string()));
		gtk_recent_manager_add_item(recentManager, ("file://" + filename).c_str());

		rql::results data = rql::select(doc->metadata, rql::matches(rql::subject, *doc->subject));
		std::string mimetype = rql::select_value<std::string>(data, rql::matches(rql::predicate, rdf::tts("mimetype")));
		std::string title    = rql::select_value<std::string>(data, rql::matches(rql::predicate, rdf::dc("title")));

		foreach_iter(entry, doc->toc)
			toc.add(entry->depth, entry->location, entry->title);

		if (toc.empty())
			gtk_widget_hide(toc);
		else
			gtk_widget_show(toc);

		settings("document.filename") = filename;
		if (!mimetype.empty())
			settings("document.mimetype") = mimetype;
		settings.save();

		doc_metadata.add_metadata(doc->metadata, *doc->subject, rdf::dc("title"));
		doc_metadata.add_metadata(doc->metadata, *doc->subject, rdf::dc("creator"));
		doc_metadata.add_metadata(doc->metadata, *doc->subject, rdf::dc("publisher"));
		doc_metadata.add_metadata(doc->metadata, *doc->subject, rdf::dc("description"));
		doc_metadata.add_metadata(doc->metadata, *doc->subject, rdf::dc("language"));

		std::ostringstream length;
		length << (doc->text_length() / CHARACTERS_PER_WORD) << _(" words (approx.)");

		doc_metadata.add_metadata(rdf::tts("length"), length.str().c_str());

		timebar.update(0.0, estimate_time(doc->text_length(), tts.parameter(tts::parameter::rate)), 0.0);

		std::string lang = rql::select_value<std::string>(doc->metadata,
			rql::both(rql::matches(rql::subject, *doc->subject),
			          rql::matches(rql::predicate, rdf::dc("language"))));
		if (lang.empty())
			lang = "en";

		switch_voice_by_language(lang);
	}
	catch (const std::exception & e)
	{
		if (!from_constructor)
		{
			display_error_message(GTK_WINDOW(window),
				_("Open Document"),
				_("Unable to open the document"),
				e.what());
		}
		ret = false;
	}

	if (doc->text_length() != 0)
	{
		gtk_widget_set_sensitive(readButton, TRUE);
		gtk_widget_set_sensitive(recordButton, TRUE);
	}
	return ret;
}

GtkWidget *Cainteoir::create_file_chooser_menu()
{
	GtkWidget *recent = gtk_recent_chooser_menu_new_for_manager(recentManager);
	gtk_recent_chooser_menu_set_show_numbers(GTK_RECENT_CHOOSER_MENU(recent), TRUE);
	gtk_recent_chooser_set_sort_type(GTK_RECENT_CHOOSER(recent), GTK_RECENT_SORT_MRU);
	gtk_recent_chooser_set_filter(GTK_RECENT_CHOOSER(recent), recentFilter);
	gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(recent), 10);

	GtkWidget *separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(recent), separator);
	gtk_widget_show(separator);

	GtkWidget *more = gtk_menu_item_new_with_mnemonic(_("_More Documents..."));
	gtk_menu_shell_append(GTK_MENU_SHELL(recent), more);
	gtk_widget_show(more);

	g_signal_connect(recent, "item-activated", G_CALLBACK(on_recent_item_activated), this);
	g_signal_connect(more, "activate", G_CALLBACK(on_recent_files_dialog), this);

	return recent;
}

bool Cainteoir::switch_voice(const rdf::uri &voice)
{
	voice_metadata.add_metadata(tts_metadata, voice, rdf::tts("name"));
	voice_metadata.add_metadata(tts_metadata, voice, rdf::dc("language"));

	foreach_iter(statement, rql::select(tts_metadata, rql::matches(rql::subject, voice)))
	{
		if (rql::predicate(*statement) == rdf::tts("voiceOf"))
		{
			const rdf::uri *engine = rql::object(*statement);
			if (engine)
			{
				engine_metadata.add_metadata(tts_metadata, *engine, rdf::tts("name"));
				engine_metadata.add_metadata(tts_metadata, *engine, rdf::tts("version"));
			}
		}
	}

	if (tts.select_voice(tts_metadata, voice))
	{
		voiceSelection->show(tts.voice());
		return true;
	}

	return false;
}

bool Cainteoir::switch_voice_by_language(const std::string &lang)
{
	std::string language = cainteoir::language::make_lang(lang).lang;

	// Does the current voice support this language? ...

	std::string current = rql::select_value<std::string>(tts_metadata,
		rql::both(rql::matches(rql::subject, tts.voice()),
		          rql::matches(rql::predicate, rdf::dc("language"))));

	current = cainteoir::language::make_lang(current).lang;
	if (current == language)
		return true;

	// The current voice does not support this language, so search the available voices ...

	rql::results voicelist = rql::select(tts_metadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object, rdf::tts("Voice"))));

	foreach_iter(voice, voicelist)
	{
		const rdf::uri *uri = rql::subject(*voice);
		if (uri)
		{
			std::string lang = rql::select_value<std::string>(tts_metadata,
				rql::both(rql::matches(rql::subject, *uri),
				          rql::matches(rql::predicate, rdf::dc("language"))));

			lang = cainteoir::language::make_lang(lang).lang;
			if (lang == language && switch_voice(*uri))
				return true;
		}
	}
	return false;
}
