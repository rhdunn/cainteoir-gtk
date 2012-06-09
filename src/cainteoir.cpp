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

#include "config.h"
#include "compatibility.hpp"
#include "i18n.h"

#include <sigc++/signal.h>

#include "cainteoir.hpp"

#include <stdexcept>
#include <stack>

#include <sys/stat.h>
#include <sys/types.h>

namespace rql    = cainteoir::rdf::query;
namespace events = cainteoir::events;

static const int CHARACTERS_PER_WORD = 6;

struct tag_block
{
	const char *name;
	int offset;

	tag_block(const char *aName, int aOffset)
		: name(aName)
		, offset(aOffset)
	{
	}
};

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

static GtkTextTagTable *create_document_tags(GtkTextBuffer *buffer,  application_settings &settings)
{
	std::string paragraph = settings("style.paragraph.font", "DejaVu Serif 11").as<std::string>();
	std::string heading   = settings("style.heading.font",   "DejaVu Serif 14").as<std::string>();

	gtk_text_buffer_create_tag(buffer, "paragraph",
		"font", paragraph.c_str(),
		nullptr);
	gtk_text_buffer_create_tag(buffer, "heading1",
		"justification",     GTK_JUSTIFY_CENTER,
		"justification-set", TRUE,
		"font",              heading.c_str(),
		"weight",            PANGO_WEIGHT_BOLD,
		"scale",             PANGO_SCALE_X_LARGE,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "heading2",
		"font",   heading.c_str(),
		"weight", PANGO_WEIGHT_BOLD,
		"scale",  PANGO_SCALE_LARGE,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "heading3",
		"font",   heading.c_str(),
		"weight", PANGO_WEIGHT_BOLD,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "heading4",
		"font",   heading.c_str(),
		"weight", PANGO_WEIGHT_BOLD,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "heading5",
		"font",   heading.c_str(),
		"weight", PANGO_WEIGHT_BOLD,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "heading6",
		"font",   heading.c_str(),
		"weight", PANGO_WEIGHT_BOLD,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "superscript",
		"rise", (PANGO_SCALE*4),
		"rise-set", TRUE,
		"scale", PANGO_SCALE_SMALL,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "subscript",
		"rise", -(PANGO_SCALE*4),
		"rise-set", TRUE,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "emphasized",
		"style", PANGO_STYLE_ITALIC,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "strong",
		"weight", PANGO_WEIGHT_BOLD,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "underline",
		"underline",  PANGO_UNDERLINE_SINGLE,
		nullptr);
	gtk_text_buffer_create_tag(buffer, "reduced",
		"style",  PANGO_STYLE_NORMAL,
		"weight", PANGO_WEIGHT_NORMAL,
		nullptr);
	return gtk_text_buffer_get_tag_table(buffer);
}

static GtkRecentFilter *create_recent_filter(const rdf::graph & aMetadata)
{
	GtkRecentFilter *filter = gtk_recent_filter_new();

	rql::results formats = rql::select(aMetadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object, rdf::tts("DocumentFormat"))));

	for(auto format = formats.begin(), last = formats.end(); format != last; ++format)
	{
		rql::results mimetypes = rql::select(aMetadata,
			rql::both(rql::matches(rql::predicate, rdf::tts("mimetype")),
			          rql::matches(rql::subject, rql::subject(*format))));

		for(auto mimetype = mimetypes.begin(), last = mimetypes.end(); mimetype != last; ++mimetype)
			gtk_recent_filter_add_mime_type(filter, rql::value(*mimetype).c_str());
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

static void on_button_clicked(GtkWidget *widget, void *data)
{
	CallbackData *cbd = (CallbackData *)data;
	((cbd->window)->*(cbd->callback))();
}

static GtkWidget *create_stock_button(const char *stock, Cainteoir *window, callback_function callback)
{
	CallbackData *data = g_slice_new(CallbackData);
	data->window   = window;
	data->callback = callback;

	GtkToolItem *button = gtk_tool_button_new_from_stock(stock);
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
	: tts(tts_metadata, cainteoir::text_support)
	, doc(new document())
	, doc_metadata(languages, i18n("Document"), 5)
	, voice_metadata(languages, i18n("Voice"), 2)
	, engine_metadata(languages, i18n("Engine"), 2)
	, settings(get_user_file("settings.dat"))
	, tags(nullptr)
{
	voiceSelection = std::shared_ptr<VoiceSelectionView>(new VoiceSelectionView(settings, tts, tts_metadata, languages));
	voiceSelection->signal_on_voice_change().connect(sigc::mem_fun(*this, &Cainteoir::switch_voice));

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), i18n("Cainteoir Text-to-Speech"));
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
	library = std::make_shared<DocumentLibrary>(languages, recentManager, tts_metadata);

	readButton   = create_stock_button(GTK_STOCK_MEDIA_PLAY,   this, &Cainteoir::read);
	stopButton   = create_stock_button(GTK_STOCK_MEDIA_STOP,   this, &Cainteoir::stop);
	recordButton = create_stock_button(GTK_STOCK_MEDIA_RECORD, this, &Cainteoir::record);
	openButton   = create_stock_button(GTK_STOCK_OPEN,         this, &Cainteoir::on_open_document);

	GtkWidget *topbar = gtk_toolbar_new();
	gtk_widget_set_name(topbar, "topbar");
	gtk_widget_set_size_request(topbar, 0, 40);
	gtk_style_context_add_class(gtk_widget_get_style_context(topbar), GTK_STYLE_CLASS_MENUBAR);

	GtkToolItem *navbar_item = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(navbar_item), navbar);
	gtk_container_add(GTK_CONTAINER(topbar), GTK_WIDGET(navbar_item));

	GtkToolItem *expander = gtk_tool_item_new();
	gtk_tool_item_set_expand(GTK_TOOL_ITEM(expander), TRUE);
	gtk_container_add(GTK_CONTAINER(topbar), GTK_WIDGET(expander));

	gtk_container_add(GTK_CONTAINER(topbar), GTK_WIDGET(openButton));

	GtkWidget *bottombar = gtk_toolbar_new();
	gtk_widget_set_name(bottombar, "bottombar");
	gtk_widget_set_size_request(bottombar, 0, 40);
	gtk_style_context_add_class(gtk_widget_get_style_context(bottombar), GTK_STYLE_CLASS_PRIMARY_TOOLBAR);

	gtk_container_add(GTK_CONTAINER(bottombar), GTK_WIDGET(readButton));
	gtk_container_add(GTK_CONTAINER(bottombar), GTK_WIDGET(stopButton));
	gtk_container_add(GTK_CONTAINER(bottombar), GTK_WIDGET(recordButton));

	GtkToolItem *timebar_item = gtk_tool_item_new();
	gtk_tool_item_set_expand(GTK_TOOL_ITEM(timebar_item), TRUE);
	gtk_container_add(GTK_CONTAINER(timebar_item), timebar);

	gtk_container_add(GTK_CONTAINER(bottombar), GTK_WIDGET(timebar_item));

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

	metadata_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(metadata_view), doc_metadata, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(metadata_view), voice_metadata, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(metadata_view), engine_metadata, FALSE, FALSE, 0);

	GtkWidget *metadata_pane = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(metadata_pane), metadata_view);

	docview = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(docview), GTK_WRAP_WORD);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(docview), FALSE);
	toc.connect(docview);

	GtkWidget *docpane = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_container_add(GTK_CONTAINER(docpane), docview);

	GtkWidget *document_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(document_view), metadata_pane, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(document_view), docpane, TRUE, TRUE, 0);

	GtkWidget *toc_pane = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_size_request(toc_pane, 200, 0);
	gtk_box_pack_start(GTK_BOX(toc_pane), toc, TRUE, TRUE, 0);

	GtkWidget *pane = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
	gtk_box_pack_start(GTK_BOX(pane), toc_pane, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pane), document_view, TRUE, TRUE, 0);

	view = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(view), FALSE);

	int doc_page = gtk_notebook_append_page(GTK_NOTEBOOK(view), pane, nullptr);
	int voice_page = gtk_notebook_append_page(GTK_NOTEBOOK(view), GTK_WIDGET(voiceSelection->gobj()),  nullptr);
	int lib_page = gtk_notebook_append_page(GTK_NOTEBOOK(view), *library, nullptr);

	ViewCallbackData *data = g_slice_new(ViewCallbackData);
	data->document_pane = docpane;
	data->info_pane = metadata_pane;

	library_button  = navbar.add_paged_button(i18n("Library"),  GTK_NOTEBOOK(view), lib_page);
	info_button = data->info_button = navbar.add_paged_button(i18n("Info"), GTK_NOTEBOOK(view), doc_page);
	document_button = data->document_button = navbar.add_paged_button(i18n("Document"), GTK_NOTEBOOK(view), doc_page);
	navbar.add_paged_button(i18n("Voice"), GTK_NOTEBOOK(view), voice_page);
	navbar.set_active_button(info_button);

	g_signal_connect(data->document_button, "toggled", G_CALLBACK(on_view_changed), data);
	g_signal_connect(data->info_button,     "toggled", G_CALLBACK(on_view_changed), data);

	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), box);
	gtk_box_pack_start(GTK_BOX(box), topbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), view, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), bottombar, FALSE, FALSE, 0);

	timebar.update(0.0, estimate_time(doc->text_length(), tts.parameter(tts::parameter::rate)), 0.0);

	gtk_widget_show_all(window);
	gtk_widget_hide(docpane);

	gtk_widget_set_sensitive(readButton, FALSE);
	gtk_widget_set_sensitive(recordButton, FALSE);
	gtk_widget_set_visible(stopButton, FALSE);

	load_document(filename ? std::string(filename) : settings("document.filename").as<std::string>(), true);
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
			rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
			          rql::matches(rql::object, rdf::tts("DocumentFormat"))));

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
	out = cainteoir::open_audio_device(nullptr, "pulse", 0.3, doc->metadata, *doc->subject, tts_metadata, tts.voice());
	on_speak(i18n("reading"));
}

void Cainteoir::record()
{
	// TODO: Generate a default name from the file metadata ($(recording.basepath)/author - title.ogg)

	rql::results formats = rql::select(tts_metadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object, rdf::tts("AudioFormat"))));

	std::string filename = select_file(GTK_WINDOW(window),
		i18n("Record Document"),
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
			rql::matches(rql::predicate, rdf::tts("extension")));

		foreach_iter(filetype, filetypes)
		{
			if (rql::value(*filetype) == ext)
			{
				const rdf::uri &uri = rql::subject(*filetype);

				std::string type = rql::select_value<std::string>(tts_metadata,
					rql::both(rql::matches(rql::subject,   uri),
					          rql::matches(rql::predicate, rdf::tts("name"))));

				std::string mimetype = rql::select_value<std::string>(tts_metadata,
					rql::both(rql::matches(rql::subject,   uri),
					          rql::matches(rql::predicate, rdf::tts("mimetype"))));

				settings("recording.filename") = filename;
				settings("recording.mimetype") = mimetype;
				settings.save();

				out = cainteoir::create_audio_file(filename.c_str(), type.c_str(), 0.3, doc->metadata, *doc->subject, tts_metadata, tts.voice());
				on_speak(i18n("recording"));
				return;
			}
		}
	}

	display_error_message(GTK_WINDOW(window),
		i18n("Record Document"),
		i18n("Unable to record the document"),
		i18n("Unsupported file type."));
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
			i18n("Cainteoir Text-to-Speech"),
			i18n("Error speaking the document"),
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

		std::shared_ptr<document> newdoc = std::make_shared<document>();
		auto reader = cainteoir::createDocumentReader(filename.c_str(), newdoc->metadata, std::string());
		if (!reader)
			throw std::runtime_error(i18n("Document type is not supported"));

		GtkTextBuffer *buffer = gtk_text_buffer_new(tags);
		if (tags == nullptr)
			tags = create_document_tags(buffer, settings);

		std::stack<tag_block> contexts;

		bool need_linebreak = false;
		GtkTextIter position;
		gtk_text_buffer_get_end_iter(buffer, &position);
		while (reader->read())
		{
			if (reader->type & events::begin_context)
			{
				const char *name = nullptr;
				bool need_newline = false;
				switch (reader->context)
				{
				case events::span:
					switch (reader->parameter)
					{
					case events::overunder:   break;
					case events::superscript: name = "superscript"; break;
					case events::subscript:   name = "subscript";   break;
					case events::emphasized:  name = "emphasized";  break;
					case events::strong:      name = "strong";      break;
					case events::underline:   name = "underline";   break;
					case events::monospace:   name = "monospace";   break;
					case events::reduced:     name = "reduced";     break;
					};
					break;
				case events::paragraph:
					name = "paragraph";
					need_newline = true;
					break;
				case events::heading:
					switch (reader->parameter)
					{
					case 1:  name = "heading1"; break;
					case 2:  name = "heading2"; break;
					case 3:  name = "heading3"; break;
					case 4:  name = "heading4"; break;
					case 5:  name = "heading5"; break;
					default: name = "heading6"; break;
					}
					need_newline = true;
					break;
				case events::list:
				case events::list_item:
					need_newline = true;
					break;
				}
				if (need_newline && need_linebreak)
				{
					gtk_text_buffer_insert(buffer, &position, "\n\n", 2);
					gtk_text_buffer_get_end_iter(buffer, &position);
					need_linebreak = false;
				}
				contexts.push({ name, gtk_text_iter_get_offset(&position) });
			}
			if (reader->type & cainteoir::events::toc_entry)
				newdoc->toc.push_back(toc_entry_data((int)reader->parameter, reader->anchor, reader->text->str()));
			if (reader->type & cainteoir::events::anchor)
			{
				newdoc->add_anchor(reader->anchor);
				GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, reader->anchor.str().c_str(), &position, TRUE);
			}
			if (reader->type & cainteoir::events::text)
			{
				newdoc->add(reader->text);

				gtk_text_buffer_insert(buffer, &position, reader->text->begin(), reader->text->size());
				gtk_text_buffer_get_end_iter(buffer, &position);
				need_linebreak = true;
			}
			if (reader->type & events::end_context)
			{
				const char *name = contexts.top().name;
				if (name)
				{
					GtkTextIter start;
					gtk_text_buffer_get_iter_at_offset(buffer, &start, contexts.top().offset);
					gtk_text_buffer_apply_tag_by_name(buffer, name, &start, &position);
				}
				contexts.pop();
			}
		}
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(docview), buffer);

		doc = newdoc;

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
		length << (doc->text_length() / CHARACTERS_PER_WORD) << i18n(" words (approx.)");

		doc_metadata.add_metadata(rdf::tts("length"), length.str().c_str());

		timebar.update(0.0, estimate_time(doc->text_length(), tts.parameter(tts::parameter::rate)), 0.0);

		std::string lang = rql::select_value<std::string>(doc->metadata,
			rql::both(rql::matches(rql::subject,   *doc->subject),
			          rql::matches(rql::predicate, rdf::dc("language"))));
		if (lang.empty())
			lang = "en";

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info_button), TRUE);

		voiceSelection->set_language(lang);
		switch_voice_by_language(lang);
	}
	catch (const std::exception & e)
	{
		if (!from_constructor)
		{
			display_error_message(GTK_WINDOW(window),
				i18n("Open Document"),
				i18n("Unable to open the document"),
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

bool Cainteoir::switch_voice(const rdf::uri &voice)
{
	voice_metadata.add_metadata(tts_metadata, voice, rdf::tts("name"));
	voice_metadata.add_metadata(tts_metadata, voice, rdf::dc("language"));

	foreach_iter(statement, rql::select(tts_metadata, rql::matches(rql::subject, voice)))
	{
		if (rql::predicate(*statement) == rdf::tts("voiceOf"))
		{
			const rdf::uri &engine = rql::object(*statement);
			if (!engine.empty())
			{
				engine_metadata.add_metadata(tts_metadata, engine, rdf::tts("name"));
				engine_metadata.add_metadata(tts_metadata, engine, rdf::tts("version"));
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
	auto language = cainteoir::language::make_lang(lang);

	// Does the current voice support this language? ...

	std::string current = rql::select_value<std::string>(tts_metadata,
		rql::both(rql::matches(rql::subject,   tts.voice()),
		          rql::matches(rql::predicate, rdf::dc("language"))));

	if (cainteoir::language::make_lang(current) == language)
		return true;

	// The current voice does not support this language, so search the available voices ...

	rql::results voicelist = rql::select(tts_metadata,
		rql::both(rql::matches(rql::predicate, rdf::rdf("type")),
		          rql::matches(rql::object,    rdf::tts("Voice"))));

	foreach_iter(voice, voicelist)
	{
		const rdf::uri &uri = rql::subject(*voice);

		std::string lang = rql::select_value<std::string>(tts_metadata,
			rql::both(rql::matches(rql::subject,   uri),
			          rql::matches(rql::predicate, rdf::dc("language"))));

		if (cainteoir::language::make_lang(lang) == language && switch_voice(uri))
			return true;
	}
	return false;
}
