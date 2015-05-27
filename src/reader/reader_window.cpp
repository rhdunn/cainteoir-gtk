/* The Cainteoir Reader main window.
 *
 * Copyright (C) 2015 Reece H. Dunn
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
#include "i18n.h"

#include <gtk/gtk.h>

#include "reader_window.h"
#include "reader_document_view.h"
#include "reader_settings_view.h"

#include <cainteoir-gtk/cainteoir_supported_formats.h>
#include <cainteoir-gtk/cainteoir_speech_synthesizers.h>
#include <cainteoir-gtk/cainteoir_speech_voice_view.h>
#include <cainteoir-gtk/cainteoir_speech_parameter.h>
#include <cainteoir-gtk/cainteoir_timebar.h>
#include <cainteoir-gtk/cainteoir_document.h>
#include <cainteoir-gtk/cainteoir_settings.h>

#include "extensions/glib.h"

#include <stack>

#if GTK_CHECK_VERSION(3, 14, 0)
#define HAMBURGER_MENU_ICON "open-menu-symbolic"
#else
#define HAMBURGER_MENU_ICON "view-list-symbolic"
#endif

typedef struct _ReaderWindowPrivate ReaderWindowPrivate;

struct _ReaderWindowPrivate
{
	GtkWidget *self;
	GtkWidget *stack;
	GtkWidget *previous;
	GtkWidget *view;
	GtkWidget *settings_view;
	GtkWidget *voice_view;
	GtkWidget *timebar;

	GtkToolItem *play_stop;

	GSimpleActionGroup *actions;
	GSimpleAction *index_pane_action;
	GSimpleAction *open_action;
	GSimpleAction *play_stop_action;
	GSimpleAction *record_action;
	GSimpleAction *previous_action;
	GSimpleAction *view_change_action;

	CainteoirSettings *settings;
	CainteoirSupportedFormats *document_formats;
	CainteoirSupportedFormats *audio_formats;
	CainteoirSpeechSynthesizers *tts;

	std::stack<GtkWidget *> view_history;

	_ReaderWindowPrivate()
		: settings(cainteoir_settings_new("settings.dat"))
		, document_formats(cainteoir_supported_formats_new(CAINTEOIR_DOCUMENT_FORMATS))
		, audio_formats(cainteoir_supported_formats_new(CAINTEOIR_AUDIO_FORMATS))
		, tts(cainteoir_speech_synthesizers_new())
	{
	}

	~_ReaderWindowPrivate()
	{
		g_object_unref(G_OBJECT(settings));
	}
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderWindow, reader_window, GTK_TYPE_WINDOW)

#define READER_WINDOW_PRIVATE(object) \
	((ReaderWindowPrivate *)reader_window_get_instance_private(READER_WINDOW(object)))

GXT_DEFINE_TYPE_CONSTRUCTION(ReaderWindow, reader_window, READER_WINDOW)

static void
reader_window_class_init(ReaderWindowClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = reader_window_finalize;
}

static gchar *
select_file(GtkWindow *window,
            const gchar *title,
            GtkFileChooserAction action,
            const gchar *open_id,
            const gchar *filename,
            const gchar *default_mimetype,
            CainteoirSupportedFormats *formats)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new(title, window, action,
		i18n("_Cancel"), GTK_RESPONSE_CANCEL,
		open_id,         GTK_RESPONSE_OK,
		nullptr);
	if (filename)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), filename);
	cainteoir_supported_formats_add_file_filters(formats, GTK_FILE_CHOOSER(dialog), default_mimetype);

	gchar *path = nullptr;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
		path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	gtk_widget_destroy(dialog);
	return path;
}

static void
reset_timebar(ReaderWindowPrivate *priv)
{
	CainteoirSpeechParameter *param = cainteoir_speech_synthesizers_get_parameter(priv->tts, CAINTEOIR_SPEECH_RATE);
	gint rate = cainteoir_speech_parameter_get_value(param);
	g_object_unref(G_OBJECT(param));

	CainteoirDocument *doc = reader_document_view_get_document(READER_DOCUMENT_VIEW(priv->view));

	cainteoir_timebar_set_time(CAINTEOIR_TIMEBAR(priv->timebar),
	                           0.0,
	                           cainteoir_document_estimate_duration(doc, rate));
	cainteoir_timebar_set_progress(CAINTEOIR_TIMEBAR(priv->timebar), 0.0);

	g_object_unref(G_OBJECT(doc));
}

static gboolean
on_speaking_timer(ReaderWindowPrivate *priv)
{
	if (cainteoir_speech_synthesizers_is_speaking(priv->tts))
	{
		cainteoir_timebar_set_time(CAINTEOIR_TIMEBAR(priv->timebar),
		                           cainteoir_speech_synthesizers_get_elapsed_time(priv->tts),
		                           cainteoir_speech_synthesizers_get_total_time(priv->tts));
		cainteoir_timebar_set_progress(CAINTEOIR_TIMEBAR(priv->timebar),
		                               cainteoir_speech_synthesizers_get_percentage_complete(priv->tts));
		return TRUE;
	}

	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(priv->play_stop), "media-playback-start-symbolic");
	reset_timebar(priv);

	return FALSE;
}

static void
on_speak(ReaderWindowPrivate *priv)
{
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(priv->play_stop), "media-playback-stop-symbolic");

	g_timeout_add(100, (GSourceFunc)on_speaking_timer, priv);
}

static gboolean
on_window_state_changed(GtkWidget *widget, GdkEvent *event, void *data)
{
	CainteoirSettings *settings = (CainteoirSettings *)data;
	gboolean maximized = (((GdkEventWindowState *)event)->new_window_state & GDK_WINDOW_STATE_MAXIMIZED);
	cainteoir_settings_set_boolean(settings, "window", "maximized", maximized);
	cainteoir_settings_save(settings);
	return TRUE;
}

static gboolean
on_window_delete(GtkWidget *window, GdkEvent *event, gpointer data)
{
	ReaderWindowPrivate *priv = (ReaderWindowPrivate *)data;

	if (!cainteoir_settings_get_boolean(priv->settings, "window", "maximized", FALSE))
	{
		gint width = 0;
		gint height = 0;
		gint top = 0;
		gint left = 0;

		gtk_window_get_position(GTK_WINDOW(priv->self), &left, &top);
		gtk_window_get_size(GTK_WINDOW(priv->self), &width, &height);

		cainteoir_settings_set_integer(priv->settings, "window", "width",  width);
		cainteoir_settings_set_integer(priv->settings, "window", "height", height);
		cainteoir_settings_set_integer(priv->settings, "window", "top",    top);
		cainteoir_settings_set_integer(priv->settings, "window", "left",   left);
	}

	cainteoir_settings_set_string(priv->settings, "index", "type",
	                              reader_document_view_get_index_type(READER_DOCUMENT_VIEW(priv->view)));

	cainteoir_settings_set_integer(priv->settings, "index", "position",
	                               reader_document_view_get_index_pane_position(READER_DOCUMENT_VIEW(priv->view)));

	cainteoir_settings_set_boolean(priv->settings, "index", "visible",
	                               reader_document_view_get_index_pane_visible(READER_DOCUMENT_VIEW(priv->view)));

	cainteoir_settings_save(priv->settings);
	return FALSE;
}

static void
on_window_show(GtkWidget *widget, gpointer data)
{
	ReaderWindowPrivate *priv = (ReaderWindowPrivate *)data;

	gtk_widget_hide(priv->previous);
}

static void
on_index_pane_toggle_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindowPrivate *priv = (ReaderWindowPrivate *)data;

	GVariant *action_state = g_action_get_state(G_ACTION(action));
	gboolean active = !g_variant_get_boolean(action_state);

	g_simple_action_set_state(action, g_variant_new_boolean(active));
	cainteoir_settings_set_boolean(priv->settings, "index", "visible", active);
	cainteoir_settings_save(priv->settings);

	reader_document_view_set_index_pane_visible(READER_DOCUMENT_VIEW(priv->view), active);
}

static void
on_open_file_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindowPrivate *priv = (ReaderWindowPrivate *)data;
	gchar *current_filename = cainteoir_settings_get_string(priv->settings, "document", "filename", nullptr);
	gchar *current_mimetype = cainteoir_settings_get_string(priv->settings, "document", "mimetype", "text/plain");

	gchar *filename = select_file(GTK_WINDOW(priv->self),
		i18n("Open Document"),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		i18n("_Open"),
		current_filename,
		current_mimetype,
		priv->document_formats);

	g_free(current_filename);
	g_free(current_mimetype);

	if (filename)
	{
		reader_window_load_document(READER_WINDOW(priv->self), filename);
		g_free(filename);
	}
}

static void
on_play_stop_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindowPrivate *priv = (ReaderWindowPrivate *)data;

	if (cainteoir_speech_synthesizers_is_speaking(priv->tts))
	{
		cainteoir_speech_synthesizers_stop(priv->tts);
	}
	else
	{
		CainteoirDocument *doc = reader_document_view_get_document(READER_DOCUMENT_VIEW(priv->view));
		CainteoirDocumentIndex *index = reader_document_view_get_document_index(READER_DOCUMENT_VIEW(priv->view));
		gchar *device_name = cainteoir_settings_get_string(priv->settings, "audio", "device-name", nullptr);

		cainteoir_speech_synthesizers_read(priv->tts, doc, index, device_name);
		on_speak(priv);

		g_free(device_name);
		g_object_unref(G_OBJECT(index));
		g_object_unref(G_OBJECT(doc));
	}
}

static void
on_record_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindowPrivate *priv = (ReaderWindowPrivate *)data;
	gchar *current_filename = cainteoir_settings_get_string(priv->settings, "recording", "filename", nullptr);
	gchar *current_mimetype = cainteoir_settings_get_string(priv->settings, "recording", "mimetype", "audio/x-vorbis+ogg");

	gchar *filename = select_file(GTK_WINDOW(priv->self),
		i18n("Record Document"),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		i18n("_Record"),
		current_filename,
		current_mimetype,
		priv->audio_formats);

	g_free(current_filename);
	g_free(current_mimetype);

	if (!filename)
		return;

	gchar *type = nullptr;
	gchar *mimetype = nullptr;
	if (cainteoir_supported_formats_file_info(priv->audio_formats, filename, &type, &mimetype))
	{
		CainteoirDocument *doc = reader_document_view_get_document(READER_DOCUMENT_VIEW(priv->view));
		CainteoirDocumentIndex *index = reader_document_view_get_document_index(READER_DOCUMENT_VIEW(priv->view));

		cainteoir_settings_set_string(priv->settings, "recording", "filename", filename);
		cainteoir_settings_set_string(priv->settings, "recording", "mimetype", mimetype);
		cainteoir_settings_save(priv->settings);

		cainteoir_speech_synthesizers_record(priv->tts, doc, index, filename, type, 0.3);
		on_speak(priv);

		g_free(type);
		g_free(mimetype);
		g_object_unref(G_OBJECT(index));
		g_object_unref(G_OBJECT(doc));
	}

	g_free(filename);
}

static void
on_previous_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindowPrivate *priv = (ReaderWindowPrivate *)data;

	if (priv->view_history.empty()) return;

	GtkWidget *view = priv->view_history.top();
	priv->view_history.pop();

	gtk_stack_set_visible_child(GTK_STACK(priv->stack), view);

	if (priv->view_history.empty())
		gtk_widget_hide(priv->previous);
}

static void
on_view_change_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindowPrivate *priv = (ReaderWindowPrivate *)data;

	GtkWidget *current = gtk_stack_get_visible_child(GTK_STACK(priv->stack));
	gtk_stack_set_visible_child_name(GTK_STACK(priv->stack), g_variant_get_string(parameter, nullptr));

	if (current != gtk_stack_get_visible_child(GTK_STACK(priv->stack)))
	{
		priv->view_history.push(current);

		gtk_widget_show(priv->previous);
	}
}

static GSimpleActionGroup *
create_action_group(ReaderWindowPrivate *priv)
{
	GSimpleActionGroup *actions = g_simple_action_group_new();

	gboolean side_pane = cainteoir_settings_get_boolean(priv->settings, "index", "visible", TRUE);

	priv->index_pane_action = g_simple_action_new_stateful("side-pane", nullptr, g_variant_new_boolean(side_pane));
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(priv->index_pane_action));
	g_signal_connect(priv->index_pane_action, "activate", G_CALLBACK(on_index_pane_toggle_action), priv);

	priv->open_action = g_simple_action_new("open", nullptr);
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(priv->open_action));
	g_signal_connect(priv->open_action, "activate", G_CALLBACK(on_open_file_action), priv);

	priv->play_stop_action = g_simple_action_new("play-stop", nullptr);
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(priv->play_stop_action));
	g_signal_connect(priv->play_stop_action, "activate", G_CALLBACK(on_play_stop_action), priv);

	priv->record_action = g_simple_action_new("record", nullptr);
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(priv->record_action));
	g_signal_connect(priv->record_action, "activate", G_CALLBACK(on_record_action), priv);

	priv->previous_action = g_simple_action_new("view-previous", nullptr);
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(priv->previous_action));
	g_signal_connect(priv->previous_action, "activate", G_CALLBACK(on_previous_action), priv);

	priv->view_change_action = g_simple_action_new("view-change", G_VARIANT_TYPE_STRING);
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(priv->view_change_action));
	g_signal_connect(priv->view_change_action, "activate", G_CALLBACK(on_view_change_action), priv);

	return actions;
}

static GMenuModel *
create_main_menu()
{
	GMenu *menu = g_menu_new();

	g_menu_append(menu, i18n("Side Pane"), "cainteoir.side-pane");

	GMenuItem *document = g_menu_item_new(i18n("Document"), nullptr);
	g_menu_item_set_action_and_target_value(document, "cainteoir.view-change", g_variant_new_string("document"));
	g_menu_append_item(menu, document);

	GMenuItem *settings = g_menu_item_new(i18n("Settings"), nullptr);
	g_menu_item_set_action_and_target_value(settings, "cainteoir.view-change", g_variant_new_string("settings"));
	g_menu_append_item(menu, settings);

	GMenuItem *voices = g_menu_item_new(i18n("Voices"), nullptr);
	g_menu_item_set_action_and_target_value(voices, "cainteoir.view-change", g_variant_new_string("voices"));
	g_menu_append_item(menu, voices);

	return G_MENU_MODEL(menu);
}

GtkWidget *
reader_window_new(const gchar *filename)
{
	ReaderWindow *reader = READER_WINDOW(g_object_new(READER_TYPE_WINDOW, nullptr));
	ReaderWindowPrivate *priv = READER_WINDOW_PRIVATE(reader);
	priv->self = GTK_WIDGET(reader);

	gtk_window_set_default_size(GTK_WINDOW(reader), INDEX_PANE_WIDTH + DOCUMENT_PANE_WIDTH + 5, 300);
	gtk_window_set_title(GTK_WINDOW(reader), i18n("Cainteoir Text-to-Speech"));

	GtkWidget *header = gtk_header_bar_new();
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), i18n("Cainteoir Text-to-Speech"));
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_window_set_titlebar(GTK_WINDOW(reader), header);

	priv->actions = create_action_group(priv);
	gtk_widget_insert_action_group(GTK_WIDGET(reader), "cainteoir", G_ACTION_GROUP(priv->actions));

	GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(reader), layout);

	priv->stack = gtk_stack_new();
	gtk_box_pack_start(GTK_BOX(layout), priv->stack, TRUE, TRUE, 0);

	priv->view = reader_document_view_new(priv->settings);
	gtk_stack_add_titled(GTK_STACK(priv->stack), priv->view, "document", i18n("Document"));
	reader_document_view_set_index_pane_close_action_name(READER_DOCUMENT_VIEW(priv->view), "cainteoir.side-pane");

	priv->settings_view = reader_settings_view_new(priv->settings, priv->tts);
	gtk_stack_add_titled(GTK_STACK(priv->stack), priv->settings_view, "settings", i18n("Settings"));

	GtkWidget *scroll = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_stack_add_titled(GTK_STACK(priv->stack), scroll, "voices", i18n("Voices"));

	priv->voice_view = cainteoir_speech_voice_view_new(priv->tts);
	gtk_container_add(GTK_CONTAINER(scroll), priv->voice_view);

	GtkWidget *bottombar = gtk_toolbar_new();
	gtk_widget_set_size_request(bottombar, -1, 45);
	gtk_style_context_add_class(gtk_widget_get_style_context(bottombar), "bottombar");
	gtk_box_pack_start(GTK_BOX(layout), bottombar, FALSE, FALSE, 0);

	GtkToolItem *record = gtk_tool_button_new(gtk_image_new_from_icon_name("media-record-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), nullptr);
	gtk_toolbar_insert(GTK_TOOLBAR(bottombar), record, -1);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(record), "cainteoir.record");

	priv->play_stop = gtk_tool_button_new(nullptr, nullptr);
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(priv->play_stop), "media-playback-start-symbolic");
	gtk_toolbar_insert(GTK_TOOLBAR(bottombar), priv->play_stop, -1);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(priv->play_stop), "cainteoir.play-stop");

	GtkToolItem *open = gtk_tool_button_new(gtk_image_new_from_icon_name("document-open-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), nullptr);
	gtk_toolbar_insert(GTK_TOOLBAR(bottombar), open, -1);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(open), "cainteoir.open");

	GtkToolItem *timebar = gtk_tool_item_new();
	gtk_tool_item_set_expand(GTK_TOOL_ITEM(timebar), TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(bottombar), timebar, -1);

	priv->timebar = cainteoir_timebar_new();
	gtk_container_add(GTK_CONTAINER(timebar), priv->timebar);

	priv->previous = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(priv->previous), gtk_image_new_from_icon_name("go-previous-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR));
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), priv->previous);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(priv->previous), "cainteoir.view-previous");

	GtkWidget *menu_button = gtk_menu_button_new();
	gtk_button_set_image(GTK_BUTTON(menu_button), gtk_image_new_from_icon_name(HAMBURGER_MENU_ICON, GTK_ICON_SIZE_SMALL_TOOLBAR));
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menu_button), create_main_menu());
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header), menu_button);

	g_signal_connect(reader, "window-state-event", G_CALLBACK(on_window_state_changed), priv->settings);
	g_signal_connect(reader, "delete_event", G_CALLBACK(on_window_delete), priv);
	g_signal_connect(reader, "show", G_CALLBACK(on_window_show), priv);

	gtk_window_resize(GTK_WINDOW(reader),
	                  cainteoir_settings_get_integer(priv->settings, "window", "width",  700),
	                  cainteoir_settings_get_integer(priv->settings, "window", "height", 445));
	gtk_window_move(GTK_WINDOW(reader),
	                cainteoir_settings_get_integer(priv->settings, "window", "left", 0),
	                cainteoir_settings_get_integer(priv->settings, "window", "top",  0));
	if (cainteoir_settings_get_boolean(priv->settings, "window", "maximized", FALSE))
		gtk_window_maximize(GTK_WINDOW(reader));

	if (filename)
		reader_window_load_document(reader, filename);
	else
	{
		gchar *prev_filename = cainteoir_settings_get_string(priv->settings, "document", "filename", nullptr);
		if (prev_filename)
		{
			reader_window_load_document(reader, prev_filename);
			g_free(prev_filename);
		}
	}

	return GTK_WIDGET(reader);
}

gboolean
reader_window_load_document(ReaderWindow *reader,
                            const gchar *filename)
{
	ReaderWindowPrivate *priv = READER_WINDOW_PRIVATE(reader);
	if (reader_document_view_load_document(READER_DOCUMENT_VIEW(priv->view), filename))
	{
		reset_timebar(priv);
		return TRUE;
	}
	return FALSE;
}
