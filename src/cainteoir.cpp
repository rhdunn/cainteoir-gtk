/* Cainteoir Main Window
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

#include "config.h"
#include "compatibility.hpp"
#include "i18n.h"

#include <sigc++/signal.h>

#include "cainteoir.hpp"

#include <stdexcept>

#include <sys/stat.h>
#include <sys/types.h>

#define UIDIR DATADIR "/" PACKAGE "/ui"

namespace rql = cainteoir::rdf::query;

static const int CHARACTERS_PER_WORD = 6;

enum
{
	TARGET_STRING,
	TARGET_URI,
};

static GtkTargetEntry dnd_drop_targets[] = {
	{ (gchar *)"STRING",        0, TARGET_STRING },
	{ (gchar *)"text/plain",    0, TARGET_STRING },
	{ (gchar *)"text/uri-list", 0, TARGET_URI },
};

static void dnd_data_received(GtkWidget *, GdkDragContext *context, gint, gint, GtkSelectionData *selection, guint, guint time, gpointer data)
{
	guchar *seldata = gtk_selection_data_get_text(selection);
	if (seldata)
	{
		std::istringstream uris((char *)seldata);
		std::string uri;
		bool loaded = false;
		while (!loaded && std::getline(uris, uri, '\r'))
			loaded = ((Cainteoir *)data)->load_document(uri, true);
		g_free(seldata);
	}
	gtk_drag_finish(context, FALSE, FALSE, time);
}

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
		nullptr);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), filename);

	for (auto &format : formats)
	{
		rql::results data = rql::select(metadata, rql::subject == rql::subject(format));

		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, rql::select_value<std::string>(data, rql::predicate == rdf::dc("title")).c_str());

		bool active_filter = false;
		for (auto &item : rql::select(data, rql::predicate == rdf::tts("mimetype")))
		{
			const std::string &mimetype = rql::value(item);
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

	for (auto &format : rql::select(aMetadata,
	                                rql::predicate == rdf::rdf("type") &&
	                                rql::object    == rdf::tts("DocumentFormat")))
	{
		for (auto &mimetype : rql::select(aMetadata,
		                                  rql::predicate == rdf::tts("mimetype") &&
		                                  rql::subject   == rql::subject(format)))
			gtk_recent_filter_add_mime_type(filter, rql::value(mimetype).c_str());
	}

	return filter;
}

struct ViewCallbackData
{
	GtkWidget *document_button;
	GtkWidget *document_pane;
	GtkWidget *info_button;
	GtkWidget *info_pane;
};

static void on_view_changed(GtkToggleButton *button, void *data)
{
	if (gtk_toggle_button_get_active(button))
	{
		ViewCallbackData *view = (ViewCallbackData *)data;
		if (button == GTK_TOGGLE_BUTTON(view->document_button))
		{
			gtk_widget_show(view->document_pane);
			gtk_widget_hide(view->info_pane);
		}
		else if (button == GTK_TOGGLE_BUTTON(view->info_button))
		{
			gtk_widget_hide(view->document_pane);
			gtk_widget_show(view->info_pane);
		}
	}
}

typedef void (Cainteoir::*callback_function)();

struct CallbackData
{
	Cainteoir *window;
	callback_function callback;
};

static void on_action_activate(GtkAction *action, void *data)
{
	CallbackData *cbd = (CallbackData *)data;
	((cbd->window)->*(cbd->callback))();
}

static void bind_action_activate(GtkAction *action, Cainteoir *window, callback_function callback)
{
	CallbackData *data = g_slice_new(CallbackData);
	data->window   = window;
	data->callback = callback;
	g_signal_connect(action, "activate", G_CALLBACK(on_action_activate), data);
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

	GtkWidget *dialog = gtk_recent_chooser_dialog_new(i18n("Recent Documents"), window,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN,   GTK_RESPONSE_ACCEPT,
		nullptr);
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
	: tts(tts_metadata)
	, doc_metadata(languages, i18n("Document"), 5)
	, voice_metadata(languages, i18n("Voice"), 2)
	, engine_metadata(languages, i18n("Engine"), 2)
	, settings(get_user_file("settings.dat"))
{
	cainteoir::supportedDocumentFormats(tts_metadata, cainteoir::text_support);
	cainteoir::supportedAudioFormats(tts_metadata);

	GtkBuilder *ui = gtk_builder_new();
	if (!gtk_builder_add_from_file(ui, UIDIR "/cainteoir-gtk.ui", NULL))
		throw std::runtime_error("unable to load the cainteoir-gtk UI file.");

	window = GTK_WIDGET(gtk_builder_get_object(ui, "cainteoir-gtk"));
	gtk_window_set_hide_titlebar_when_maximized(GTK_WINDOW(window), TRUE);

	gtk_window_resize(GTK_WINDOW(window), settings("window.width",  700).as<int>(), settings("window.height", 445).as<int>());
	gtk_window_move(GTK_WINDOW(window), settings("window.left", 0).as<int>(), settings("window.top",  0).as<int>());
	if (settings("window.maximized", "false").as<std::string>() == "true")
		gtk_window_maximize(GTK_WINDOW(window));

	g_signal_connect(window, "window-state-event", G_CALLBACK(on_window_state_changed), &settings);
	g_signal_connect(window, "delete-event", G_CALLBACK(on_window_delete), this);

	voiceSelection = std::shared_ptr<VoiceSelectionView>(new VoiceSelectionView(settings, tts, tts_metadata, languages, ui));
	voiceSelection->signal_on_voice_change().connect(sigc::mem_fun(*this, &Cainteoir::switch_voice));

	recentManager = gtk_recent_manager_get_default();
	recentFilter = create_recent_filter(tts_metadata);

	GtkWidget *library_view = GTK_WIDGET(gtk_builder_get_object(ui, "library-view"));
	library = std::make_shared<DocumentLibrary>(languages, recentManager, tts_metadata);
	gtk_container_add(GTK_CONTAINER(library_view), *library);

	readAction   = GTK_ACTION(gtk_builder_get_object(ui, "play-action"));
	stopAction   = GTK_ACTION(gtk_builder_get_object(ui, "stop-action"));
	recordAction = GTK_ACTION(gtk_builder_get_object(ui, "record-action"));
	openAction   = GTK_ACTION(gtk_builder_get_object(ui, "open-action"));

	bind_action_activate(readAction,   this, &Cainteoir::read);
	bind_action_activate(stopAction,   this, &Cainteoir::stop);
	bind_action_activate(recordAction, this, &Cainteoir::record);
	bind_action_activate(openAction,   this, &Cainteoir::on_open_document);

	timebar = std::make_shared<TimeBar>(
		GTK_WIDGET(gtk_builder_get_object(ui, "timebar-progress")),
		GTK_WIDGET(gtk_builder_get_object(ui, "timebar-elapsed")),
		GTK_WIDGET(gtk_builder_get_object(ui, "timebar-total")));

	doc_metadata.create_entry(rdf::dc("title"), i18n("Title"), 0);
	doc_metadata.create_entry(rdf::dc("creator"), i18n("Author"), 1);
	doc_metadata.create_entry(rdf::dc("publisher"), i18n("Publisher"), 2);
	doc_metadata.create_entry(rdf::dc("description"), i18n("Description"), 3);
	doc_metadata.create_entry(rdf::dc("language"), i18n("Language"), 4);
	doc_metadata.create_entry(rdf::tts("length"), i18n("Length"), 5);

	voice_metadata.create_entry(rdf::tts("name"), i18n("Name"), 0);
	voice_metadata.create_entry(rdf::dc("language"), i18n("Language"), 1);

	engine_metadata.create_entry(rdf::tts("name"), i18n("Name"), 0);
	engine_metadata.create_entry(rdf::tts("version"), i18n("Version"), 1);

	GtkWidget *toc_view = GTK_WIDGET(gtk_builder_get_object(ui, "toc-view"));
	gtk_container_add(GTK_CONTAINER(toc_view), toc);

	metadata_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(metadata_view), doc_metadata, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(metadata_view), voice_metadata, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(metadata_view), engine_metadata, FALSE, FALSE, 0);

	GtkWidget *metadata_pane = GTK_WIDGET(gtk_builder_get_object(ui, "metadata-pane"));
#if GTK_CHECK_VERSION(3, 8, 0)
	gtk_container_add(GTK_CONTAINER(metadata_pane), metadata_view);
#else
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(metadata_pane), metadata_view);
#endif

	docview = GTK_WIDGET(gtk_builder_get_object(ui, "document"));
	toc.connect(docview);

	GtkWidget *pane = GTK_WIDGET(gtk_builder_get_object(ui, "document-page"));
	gtk_drag_dest_set(pane, GTK_DEST_DEFAULT_ALL, dnd_drop_targets, 3, (GdkDragAction)(GDK_ACTION_COPY|GDK_ACTION_MOVE|GDK_ACTION_LINK));
	g_signal_connect(pane, "drag-data-received", G_CALLBACK(dnd_data_received), this);

	view = GTK_WIDGET(gtk_builder_get_object(ui, "view"));

	int doc_page   = 0;
	int lib_page   = 1;
	int voice_page = 2;

	ViewCallbackData *data = g_slice_new(ViewCallbackData);
	data->document_pane = GTK_WIDGET(gtk_builder_get_object(ui, "doc-pane"));
	data->info_pane = metadata_pane;

	library_button = navbar.add_paged_button(GTK_WIDGET(gtk_builder_get_object(ui, "library-button")),  GTK_NOTEBOOK(view), lib_page);
	info_button = data->info_button = navbar.add_paged_button(GTK_WIDGET(gtk_builder_get_object(ui, "info-button")), GTK_NOTEBOOK(view), doc_page);
	document_button = data->document_button = navbar.add_paged_button(GTK_WIDGET(gtk_builder_get_object(ui, "document-button")), GTK_NOTEBOOK(view), doc_page);
	navbar.add_paged_button(GTK_WIDGET(gtk_builder_get_object(ui, "voice-button")), GTK_NOTEBOOK(view), voice_page);
	navbar.set_active_button(info_button);

	g_signal_connect(data->document_button, "toggled", G_CALLBACK(on_view_changed), data);
	g_signal_connect(data->info_button,     "toggled", G_CALLBACK(on_view_changed), data);

	timebar->update(0.0, 0.0, 0.0);

	gtk_widget_show_all(window);
	gtk_widget_hide(data->document_pane);

	gtk_action_set_sensitive(readAction, FALSE);
	gtk_action_set_sensitive(recordAction, FALSE);
	gtk_action_set_visible(stopAction, FALSE);

	load_document(filename ? std::string(filename) : settings("document.filename").as<std::string>(), true);

	rdf::uri voice = rdf::href(settings("voice.name", std::string()).as<std::string>());
	bool set_voice = false;
	if (!voice.empty())
		set_voice = switch_voice(voice);
	if (!set_voice)
		switch_voice(tts.voice());
}

void Cainteoir::on_open_document()
{
	std::string filename;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(library_button)))
	{
		filename = library->get_filename();
	}
	else
	{
		rql::results formats = rql::select(tts_metadata,
		                                   rql::predicate == rdf::rdf("type") &&
		                                   rql::object    == rdf::tts("DocumentFormat"));

		filename = select_file(GTK_WINDOW(window),
			i18n("Open Document"),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_OPEN,
			settings("document.filename").as<std::string>().c_str(),
			settings("document.mimetype", "text/plain").as<std::string>(),
			tts_metadata, formats);
	}

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
	out = cainteoir::open_audio_device(nullptr, rdf_metadata, subject, tts_metadata, tts.voice());
	on_speak(i18n("reading"));
}

void Cainteoir::record()
{
	// TODO: Generate a default name from the file metadata ($(recording.basepath)/author - title.ogg)

	rql::results formats = rql::select(tts_metadata,
	                                   rql::predicate == rdf::rdf("type") &&
	                                   rql::object    == rdf::tts("AudioFormat"));

	std::string filename = select_file(GTK_WINDOW(window),
		i18n("Record Document"),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_MEDIA_RECORD,
		settings("recording.filename").as<std::string>().c_str(),
		settings("recording.mimetype", "audio/x-vorbis+ogg").as<std::string>(),
		tts_metadata, formats);

	if (filename.empty())
		return;

	try
	{
		std::string::size_type extpos = filename.rfind('.');
		if (extpos != std::string::npos)
		{
			std::string ext = '*' + filename.substr(extpos);

			for (auto &filetype : rql::select(tts_metadata, rql::predicate == rdf::tts("extension")))
			{
				if (rql::value(filetype) == ext)
				{
					const rdf::uri &uri = rql::subject(filetype);

					auto type     = rql::select_value<std::string>(tts_metadata,
					                rql::subject == uri && rql::predicate == rdf::tts("name"));

					auto mimetype = rql::select_value<std::string>(tts_metadata,
					                rql::subject == uri && rql::predicate == rdf::tts("mimetype"));

					settings("recording.filename") = filename;
					settings("recording.mimetype") = mimetype;
					settings.save();

					out = cainteoir::create_audio_file(filename.c_str(), type.c_str(), 0.3, rdf_metadata, subject, tts_metadata, tts.voice());
					on_speak(i18n("recording"));
					return;
				}
			}
		}

		throw std::runtime_error(i18n("Unsupported file type."));
	}
	catch (const std::runtime_error &e)
	{
		display_error_message(GTK_WINDOW(window),
			i18n("Record Document"),
			i18n("Unable to record the document"),
			e.what());
	}
}

void Cainteoir::stop()
{
	if (speech)
		speech->stop();
}

void Cainteoir::on_speak(const char * status)
{
	speech = tts.speak(out, doc->toc(), doc->children(toc.selection()));

	gtk_action_set_visible(readAction, FALSE);
	gtk_action_set_visible(stopAction, TRUE);
	gtk_action_set_sensitive(recordAction, FALSE);
	gtk_action_set_sensitive(openAction, FALSE);

	g_timeout_add(100, (GSourceFunc)on_timer_speaking, this);
}

bool Cainteoir::on_speaking()
{
	if (speech->is_speaking())
	{
		timebar->update(speech->elapsedTime(), speech->totalTime(), speech->completed());
		toc.set_playing(speech->toc_entry());
		return true;
	}

	std::string error = speech->error_message();
	if (!error.empty())
	{
		display_error_message(GTK_WINDOW(window),
			i18n("Cainteoir Text-to-Speech"),
			i18n("Error speaking the document"),
			error.c_str());
	}

	speech.reset();
	out.reset();

	timebar->update(0.0, estimate_time(doc->text_length(), tts.parameter(tts::parameter::rate)), 0.0);

	gtk_action_set_visible(readAction, TRUE);
	gtk_action_set_visible(stopAction, FALSE);
	gtk_action_set_sensitive(recordAction, TRUE);
	gtk_action_set_sensitive(openAction, TRUE);

	return false;
}

bool Cainteoir::load_document(std::string filename, bool suppress_error_message)
{
	if (speech || filename.empty()) return false;

	gtk_action_set_sensitive(readAction, FALSE);
	gtk_action_set_sensitive(recordAction, FALSE);

	bool ret = true;
	try
	{
		if (filename.find("file://") == 0)
			filename.erase(0, 7);

		auto reader = cainteoir::createDocumentReader(filename.c_str(), rdf_metadata, std::string());
		if (!reader)
			throw std::runtime_error(i18n("Document type is not supported"));

		auto newdoc = std::make_shared<cainteoir::document>(reader);
		GtkTextBuffer *buffer = create_buffer_from_document(newdoc);
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(docview), buffer);

		doc = newdoc;

		toc.clear();
		doc_metadata.clear();

		subject = rdf::uri(filename, std::string());
		gtk_recent_manager_add_item(recentManager, ("file://" + filename).c_str());

		rql::results data     = rql::select(rdf_metadata, rql::subject == subject);
		std::string  mimetype = rql::select_value<std::string>(data, rql::predicate == rdf::tts("mimetype"));
		std::string  title    = rql::select_value<std::string>(data, rql::predicate == rdf::dc("title"));

		for (auto &entry : doc->toc())
			toc.add(entry);

		if (toc.empty())
			gtk_widget_hide(toc);
		else
			gtk_widget_show(toc);

		settings("document.filename") = filename;
		if (!mimetype.empty())
			settings("document.mimetype") = mimetype;
		settings.save();

		doc_metadata.add_metadata(rdf_metadata, subject, rdf::dc("title"));
		doc_metadata.add_metadata(rdf_metadata, subject, rdf::dc("creator"));
		doc_metadata.add_metadata(rdf_metadata, subject, rdf::dc("publisher"));
		doc_metadata.add_metadata(rdf_metadata, subject, rdf::dc("description"));
		doc_metadata.add_metadata(rdf_metadata, subject, rdf::dc("language"));

		std::ostringstream length;
		length << (doc->text_length() / CHARACTERS_PER_WORD) << i18n(" words (approx.)");

		doc_metadata.add_metadata(rdf::tts("length"), length.str().c_str());

		timebar->update(0.0, estimate_time(doc->text_length(), tts.parameter(tts::parameter::rate)), 0.0);

		std::string lang = rql::select_value<std::string>(rdf_metadata,
		                   rql::subject == subject && rql::predicate == rdf::dc("language"));
		if (lang.empty())
			lang = "en";

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(library_button)))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info_button), TRUE);

		voiceSelection->set_language(lang);
		switch_voice_by_language(lang);
	}
	catch (const std::exception & e)
	{
		if (!suppress_error_message)
		{
			display_error_message(GTK_WINDOW(window),
				i18n("Open Document"),
				i18n("Unable to open the document"),
				e.what());
		}
		ret = false;
	}

	if (doc && doc->text_length() != 0)
	{
		gtk_action_set_sensitive(readAction, TRUE);
		gtk_action_set_sensitive(recordAction, TRUE);
	}
	return ret;
}

bool Cainteoir::switch_voice(const rdf::uri &voice)
{
	voice_metadata.add_metadata(tts_metadata, voice, rdf::tts("name"));
	voice_metadata.add_metadata(tts_metadata, voice, rdf::dc("language"));

	auto voiceof = rql::select(tts_metadata,
	                           rql::subject == voice && rql::predicate == rdf::tts("voiceOf"));
	if (voiceof.empty())
		return false;

	const rdf::uri &engine = rql::object(voiceof.front());
	if (!engine.empty())
	{
		engine_metadata.add_metadata(tts_metadata, engine, rdf::tts("name"));
		engine_metadata.add_metadata(tts_metadata, engine, rdf::tts("version"));
	}

	if (tts.select_voice(tts_metadata, voice))
	{
		voiceSelection->show(tts.voice());
		if ((!speech || !speech->is_speaking()) && doc)
			timebar->update(0.0, estimate_time(doc->text_length(), tts.parameter(tts::parameter::rate)), 0.0);
		settings("voice.name") = voice.str();
		return true;
	}

	return false;
}

bool Cainteoir::switch_voice_by_language(const std::string &lang)
{
	auto language = cainteoir::language::make_lang(lang);

	// Does the current voice support this language? ...

	std::string current = rql::select_value<std::string>(tts_metadata,
	                      rql::subject == tts.voice() && rql::predicate == rdf::dc("language"));

	if (cainteoir::language::make_lang(current) == language)
		return true;

	// The current voice does not support this language, so search the available voices ...

	for (auto &voice : rql::select(tts_metadata,
	                               rql::predicate == rdf::rdf("type") && rql::object == rdf::tts("Voice")))
	{
		const rdf::uri &uri = rql::subject(voice);

		std::string lang = rql::select_value<std::string>(tts_metadata,
		                   rql::subject == uri && rql::predicate == rdf::dc("language"));

		if (cainteoir::language::make_lang(lang) == language && switch_voice(uri))
			return true;
	}
	return false;
}
