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

#include <cainteoir-gtk/cainteoir_document_view.h>
#include <cainteoir-gtk/cainteoir_document_index.h>
#include <cainteoir-gtk/cainteoir_supported_formats.h>
#include <cainteoir-gtk/cainteoir_speech_synthesizers.h>
#include <cainteoir-gtk/cainteoir_settings.h>
#include <cainteoir-gtk/cainteoir_timebar.h>

static constexpr int INDEX_PANE_WIDTH = 265;
static constexpr int DOCUMENT_PANE_WIDTH = 300;

#if GTK_CHECK_VERSION(3, 14, 0)
#define HAMBURGER_MENU_ICON "open-menu-symbolic"
#else
#define HAMBURGER_MENU_ICON "view-list-symbolic"
#endif

enum IndexTypeColumns
{
	INDEX_TYPE_LABEL,
	INDEX_TYPE_ID,
	INDEX_TYPE_COUNT,
};

struct _ReaderWindowPrivate
{
	GtkWidget *doc_pane;
	GtkWidget *index_type;
	GtkWidget *index;
	GtkWidget *view;
	GtkWidget *timebar;

	GSimpleActionGroup *actions;
	GSimpleAction *index_pane_action;
	GSimpleAction *open_action;
	GSimpleAction *play_stop_action;
	GSimpleAction *record_action;

	GtkToolItem *play_stop;

	CainteoirSettings *settings;
	CainteoirSupportedFormats *document_formats;
	CainteoirSupportedFormats *audio_formats;
	CainteoirSpeechSynthesizers *tts;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderWindow, reader_window, GTK_TYPE_WINDOW)

static void
reader_window_finalize(GObject *object)
{
	ReaderWindow *reader = READER_WINDOW(object);

	G_OBJECT_CLASS(reader_window_parent_class)->finalize(object);
}

static void
reader_window_class_init(ReaderWindowClass *klass)
{
	GObjectClass *object = G_OBJECT_CLASS(klass);
	object->finalize = reader_window_finalize;
}

static void
reader_window_init(ReaderWindow *reader)
{
	reader->priv = (ReaderWindowPrivate *)reader_window_get_instance_private(reader);
	reader->priv->settings = cainteoir_settings_new("settings.dat");
	reader->priv->document_formats = cainteoir_supported_formats_new(CAINTEOIR_DOCUMENT_FORMATS);
	reader->priv->audio_formats = cainteoir_supported_formats_new(CAINTEOIR_AUDIO_FORMATS);
	reader->priv->tts = cainteoir_speech_synthesizers_new();
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
reset_timebar(ReaderWindow *reader)
{
	CainteoirSpeechParameter *param = cainteoir_speech_synthesizers_get_parameter(reader->priv->tts, CAINTEOIR_SPEECH_RATE);
	gint rate = cainteoir_speech_parameter_get_value(param);
	g_object_unref(G_OBJECT(param));

	CainteoirDocument *doc = cainteoir_document_view_get_document(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view));

	cainteoir_timebar_set_time(CAINTEOIR_TIMEBAR(reader->priv->timebar),
	                           0.0,
	                           cainteoir_document_estimate_duration(doc, rate));
	cainteoir_timebar_set_progress(CAINTEOIR_TIMEBAR(reader->priv->timebar), 0.0);

	g_object_unref(G_OBJECT(doc));
}

static gboolean
on_speaking_timer(ReaderWindow *reader)
{
	if (cainteoir_speech_synthesizers_is_speaking(reader->priv->tts))
	{
		cainteoir_timebar_set_time(CAINTEOIR_TIMEBAR(reader->priv->timebar),
		                           cainteoir_speech_synthesizers_get_elapsed_time(reader->priv->tts),
		                           cainteoir_speech_synthesizers_get_total_time(reader->priv->tts));
		cainteoir_timebar_set_progress(CAINTEOIR_TIMEBAR(reader->priv->timebar),
		                               cainteoir_speech_synthesizers_get_percentage_complete(reader->priv->tts));
		return TRUE;
	}

	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(reader->priv->play_stop), "media-playback-start-symbolic");
	reset_timebar(reader);

	return FALSE;
}

static void
on_speak(ReaderWindow *reader)
{
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(reader->priv->play_stop), "media-playback-stop-symbolic");

	g_timeout_add(100, (GSourceFunc)on_speaking_timer, reader);
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
	ReaderWindow *reader = (ReaderWindow *)data;

	if (!cainteoir_settings_get_boolean(reader->priv->settings, "window", "maximized", FALSE))
	{
		gint width = 0;
		gint height = 0;
		gint top = 0;
		gint left = 0;

		gtk_window_get_position(GTK_WINDOW(reader), &left, &top);
		gtk_window_get_size(GTK_WINDOW(reader), &width, &height);

		cainteoir_settings_set_integer(reader->priv->settings, "window", "width",  width);
		cainteoir_settings_set_integer(reader->priv->settings, "window", "height", height);
		cainteoir_settings_set_integer(reader->priv->settings, "window", "top",    top);
		cainteoir_settings_set_integer(reader->priv->settings, "window", "left",   left);
	}

	cainteoir_settings_set_string(reader->priv->settings, "index", "type",
	                              gtk_combo_box_get_active_id(GTK_COMBO_BOX(reader->priv->index_type)));

	cainteoir_settings_set_integer(reader->priv->settings, "index", "position",
	                               gtk_paned_get_position(GTK_PANED(reader->priv->doc_pane)));

	cainteoir_settings_set_boolean(reader->priv->settings, "index", "visible",
	                               gtk_widget_get_visible(gtk_paned_get_child1(GTK_PANED(reader->priv->doc_pane))));

	cainteoir_settings_save(reader->priv->settings);
	return FALSE;
}

static void
on_window_show(GtkWidget *window, gpointer data)
{
	ReaderWindow *reader = (ReaderWindow *)data;

	gtk_paned_set_position(GTK_PANED(reader->priv->doc_pane),
	                       cainteoir_settings_get_integer(reader->priv->settings, "index", "position", INDEX_PANE_WIDTH));

	if (cainteoir_settings_get_boolean(reader->priv->settings, "index", "visible", TRUE))
		gtk_widget_show(gtk_paned_get_child1(GTK_PANED(reader->priv->doc_pane)));
	else
		gtk_widget_hide(gtk_paned_get_child1(GTK_PANED(reader->priv->doc_pane)));
}

static void
on_index_pane_close(GtkWidget *window, gpointer data)
{
	ReaderWindow *reader = (ReaderWindow *)data;

	cainteoir_settings_set_boolean(reader->priv->settings, "index", "visible", FALSE);
	gtk_widget_hide(gtk_paned_get_child1(GTK_PANED(reader->priv->doc_pane)));
	g_simple_action_set_state(reader->priv->index_pane_action, g_variant_new_boolean(FALSE));
}

static void
on_index_pane_toggle_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindow *reader = (ReaderWindow *)data;

	GVariant *action_state = g_action_get_state(G_ACTION(action));
	gboolean active = !g_variant_get_boolean(action_state);

	g_simple_action_set_state(action, g_variant_new_boolean(active));
	cainteoir_settings_set_boolean(reader->priv->settings, "index", "visible", active);
	cainteoir_settings_save(reader->priv->settings);

	if (active)
		gtk_widget_show(gtk_paned_get_child1(GTK_PANED(reader->priv->doc_pane)));
	else
		gtk_widget_hide(gtk_paned_get_child1(GTK_PANED(reader->priv->doc_pane)));
}

static void
on_open_file_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindow *reader = (ReaderWindow *)data;
	gchar *current_filename = cainteoir_settings_get_string(reader->priv->settings, "document", "filename", nullptr);
	gchar *current_mimetype = cainteoir_settings_get_string(reader->priv->settings, "document", "mimetype", "text/plain");

	gchar *filename = select_file(GTK_WINDOW(reader),
		i18n("Open Document"),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		i18n("_Open"),
		current_filename,
		current_mimetype,
		reader->priv->document_formats);

	g_free(current_filename);
	g_free(current_mimetype);

	if (filename)
	{
		reader_window_load_document(reader, filename);
		g_free(filename);
	}
}

static void
on_play_stop_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindow *reader = (ReaderWindow *)data;

	if (cainteoir_speech_synthesizers_is_speaking(reader->priv->tts))
	{
		cainteoir_speech_synthesizers_stop(reader->priv->tts);
	}
	else
	{
		CainteoirDocument *doc = cainteoir_document_view_get_document(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view));
		gchar *device_name = cainteoir_settings_get_string(reader->priv->settings, "audio", "device-name", nullptr);

		cainteoir_speech_synthesizers_read(reader->priv->tts, doc, CAINTEOIR_DOCUMENT_INDEX(reader->priv->index), device_name);
		on_speak(reader);

		g_free(device_name);
		g_object_unref(G_OBJECT(doc));
	}
}

static void
on_record_action(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	ReaderWindow *reader = (ReaderWindow *)data;
	gchar *current_filename = cainteoir_settings_get_string(reader->priv->settings, "recording", "filename", nullptr);
	gchar *current_mimetype = cainteoir_settings_get_string(reader->priv->settings, "recording", "mimetype", "audio/x-vorbis+ogg");

	gchar *filename = select_file(GTK_WINDOW(reader),
		i18n("Record Document"),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		i18n("_Record"),
		current_filename,
		current_mimetype,
		reader->priv->audio_formats);

	g_free(current_filename);
	g_free(current_mimetype);

	if (!filename)
		return;

	gchar *type = nullptr;
	gchar *mimetype = nullptr;
	if (cainteoir_supported_formats_file_info(reader->priv->audio_formats, filename, &type, &mimetype))
	{
		CainteoirDocument *doc = cainteoir_document_view_get_document(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view));

		cainteoir_settings_set_string(reader->priv->settings, "recording", "filename", filename);
		cainteoir_settings_set_string(reader->priv->settings, "recording", "mimetype", mimetype);
		cainteoir_settings_save(reader->priv->settings);

		cainteoir_speech_synthesizers_record(reader->priv->tts, doc, CAINTEOIR_DOCUMENT_INDEX(reader->priv->index), filename, type, 0.3);
		on_speak(reader);

		g_free(type);
		g_free(mimetype);
		g_object_unref(G_OBJECT(doc));
	}

	g_free(filename);
}

static GtkWidget *
create_index_type_combo(void)
{
	GtkTreeStore *store = gtk_tree_store_new(INDEX_TYPE_COUNT, G_TYPE_STRING, G_TYPE_STRING);
	GtkTreeIter row;

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Index"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_TOC,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Index (Abridged)"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_TOC_BRIEF,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Landmarks"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_LANDMARKS,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Pages"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_PAGE_LIST,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Illustrations"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_ILLUSTRATIONS,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Tables"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_TABLES,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Audio"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_AUDIO,
	                   -1);

	gtk_tree_store_append(store, &row, nullptr);
	gtk_tree_store_set(store, &row,
	                   INDEX_TYPE_LABEL, i18n("Video"),
	                   INDEX_TYPE_ID,    CAINTEOIR_INDEXTYPE_VIDEO,
	                   -1);

	GtkWidget *combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_combo_box_set_id_column(GTK_COMBO_BOX(combo), INDEX_TYPE_ID);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", INDEX_TYPE_LABEL, nullptr);

	g_object_unref(G_OBJECT(store));
	return combo;
}

static GSimpleActionGroup *
create_action_group(ReaderWindow *reader)
{
	GSimpleActionGroup *actions = g_simple_action_group_new();

	gboolean side_pane = cainteoir_settings_get_boolean(reader->priv->settings, "index", "visible", TRUE);

	reader->priv->index_pane_action = g_simple_action_new_stateful("side-pane", nullptr, g_variant_new_boolean(side_pane));
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(reader->priv->index_pane_action));
	g_signal_connect(reader->priv->index_pane_action, "activate", G_CALLBACK(on_index_pane_toggle_action), reader);

	reader->priv->open_action = g_simple_action_new("open", nullptr);
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(reader->priv->open_action));
	g_signal_connect(reader->priv->open_action, "activate", G_CALLBACK(on_open_file_action), reader);

	reader->priv->play_stop_action = g_simple_action_new("play-stop", nullptr);
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(reader->priv->play_stop_action));
	g_signal_connect(reader->priv->play_stop_action, "activate", G_CALLBACK(on_play_stop_action), reader);

	reader->priv->record_action = g_simple_action_new("record", nullptr);
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(reader->priv->record_action));
	g_signal_connect(reader->priv->record_action, "activate", G_CALLBACK(on_record_action), reader);

	return actions;
}

static GMenuModel *
create_main_menu(ReaderWindow *reader)
{
	GMenu *menu = g_menu_new();
	g_menu_append(menu, "Side Pane", "cainteoir.side-pane");
	return G_MENU_MODEL(menu);
}

static void
on_index_type_changed(GtkWidget *window, gpointer data)
{
	ReaderWindow *reader = (ReaderWindow *)data;

	CainteoirDocument *doc = cainteoir_document_view_get_document(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view));
	if (doc)
	{
		cainteoir_document_index_build(CAINTEOIR_DOCUMENT_INDEX(reader->priv->index), doc,
		                               gtk_combo_box_get_active_id(GTK_COMBO_BOX(reader->priv->index_type)));
		g_object_unref(G_OBJECT(doc));
	}
}

GtkWidget *
reader_window_new(const gchar *filename)
{
	ReaderWindow *reader = READER_WINDOW(g_object_new(READER_TYPE_WINDOW, nullptr));
	gtk_window_set_default_size(GTK_WINDOW(reader), INDEX_PANE_WIDTH + DOCUMENT_PANE_WIDTH + 5, 300);
	gtk_window_set_title(GTK_WINDOW(reader), i18n("Cainteoir Text-to-Speech"));

	GtkWidget *header = gtk_header_bar_new();
	gtk_header_bar_set_title(GTK_HEADER_BAR(header), i18n("Cainteoir Text-to-Speech"));
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
	gtk_window_set_titlebar(GTK_WINDOW(reader), header);

	reader->priv->actions = create_action_group(reader);
	gtk_widget_insert_action_group(GTK_WIDGET(reader), "cainteoir", G_ACTION_GROUP(reader->priv->actions));

	GtkWidget *menu_button = gtk_menu_button_new();
	gtk_button_set_image(GTK_BUTTON(menu_button), gtk_image_new_from_icon_name(HAMBURGER_MENU_ICON, GTK_ICON_SIZE_SMALL_TOOLBAR));
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menu_button), create_main_menu(reader));
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header), menu_button);

	GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(reader), layout);

	reader->priv->doc_pane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(layout), reader->priv->doc_pane, TRUE, TRUE, 0);

	GtkWidget *view_scroll = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_widget_set_size_request(view_scroll, DOCUMENT_PANE_WIDTH, -1);
	gtk_paned_pack2(GTK_PANED(reader->priv->doc_pane), view_scroll, TRUE, FALSE);

	reader->priv->view  = cainteoir_document_view_new();
	gtk_container_add(GTK_CONTAINER(view_scroll), reader->priv->view);

	GtkWidget *index_pane = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_widget_set_size_request(index_pane, INDEX_PANE_WIDTH, -1);
	gtk_paned_pack1(GTK_PANED(reader->priv->doc_pane), index_pane, TRUE, FALSE);

	GtkWidget *index_pane_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_pack_start(GTK_BOX(index_pane), index_pane_header, FALSE, FALSE, 0);

	reader->priv->index_type = create_index_type_combo();
	gtk_box_pack_start(GTK_BOX(index_pane_header), reader->priv->index_type, TRUE, TRUE, 0);
	g_signal_connect(reader->priv->index_type, "changed", G_CALLBACK(on_index_type_changed), reader);

	GtkWidget *index_pane_close = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_button_set_always_show_image(GTK_BUTTON(index_pane_close), TRUE);
	gtk_button_set_relief(GTK_BUTTON(index_pane_close), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(index_pane_header), index_pane_close, FALSE, FALSE, 0);
	g_signal_connect(index_pane_close, "clicked", G_CALLBACK(on_index_pane_close), reader);

	GtkWidget *index_scroll = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_box_pack_start(GTK_BOX(index_pane), index_scroll, TRUE, TRUE, 0);

	reader->priv->index = cainteoir_document_index_new(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view));
	gtk_container_add(GTK_CONTAINER(index_scroll), reader->priv->index);

	GtkWidget *bottombar = gtk_toolbar_new();
	gtk_widget_set_size_request(bottombar, -1, 45);
	gtk_box_pack_start(GTK_BOX(layout), bottombar, FALSE, FALSE, 0);

	GtkToolItem *record = gtk_tool_button_new(gtk_image_new_from_icon_name("media-record-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), nullptr);
	gtk_toolbar_insert(GTK_TOOLBAR(bottombar), record, -1);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(record), "cainteoir.record");

	reader->priv->play_stop = gtk_tool_button_new(nullptr, nullptr);
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(reader->priv->play_stop), "media-playback-start-symbolic");
	gtk_toolbar_insert(GTK_TOOLBAR(bottombar), reader->priv->play_stop, -1);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(reader->priv->play_stop), "cainteoir.play-stop");

	GtkToolItem *open = gtk_tool_button_new(gtk_image_new_from_icon_name("document-open-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), nullptr);
	gtk_toolbar_insert(GTK_TOOLBAR(bottombar), open, -1);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(open), "cainteoir.open");

	GtkToolItem *timebar = gtk_tool_item_new();
	gtk_tool_item_set_expand(GTK_TOOL_ITEM(timebar), TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(bottombar), timebar, -1);

	reader->priv->timebar = cainteoir_timebar_new();
	gtk_container_add(GTK_CONTAINER(timebar), reader->priv->timebar);

	g_signal_connect(reader, "window-state-event", G_CALLBACK(on_window_state_changed), reader->priv->settings);
	g_signal_connect(reader, "delete_event", G_CALLBACK(on_window_delete), reader);
	g_signal_connect(reader, "show", G_CALLBACK(on_window_show), reader);

	gtk_window_resize(GTK_WINDOW(reader),
	                  cainteoir_settings_get_integer(reader->priv->settings, "window", "width",  700),
	                  cainteoir_settings_get_integer(reader->priv->settings, "window", "height", 445));
	gtk_window_move(GTK_WINDOW(reader),
	                cainteoir_settings_get_integer(reader->priv->settings, "window", "left", 0),
	                cainteoir_settings_get_integer(reader->priv->settings, "window", "top",  0));
	if (cainteoir_settings_get_boolean(reader->priv->settings, "window", "maximized", FALSE))
		gtk_window_maximize(GTK_WINDOW(reader));

	gchar *active_index = cainteoir_settings_get_string(reader->priv->settings, "index", "type", CAINTEOIR_INDEXTYPE_TOC);
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(reader->priv->index_type), active_index);
	g_free(active_index);

	if (filename)
		reader_window_load_document(reader, filename);
	else
	{
		gchar *prev_filename = cainteoir_settings_get_string(reader->priv->settings, "document", "filename", nullptr);
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
	CainteoirDocument *doc = cainteoir_document_new(filename);
	if (doc)
	{
		cainteoir_document_view_set_document(CAINTEOIR_DOCUMENT_VIEW(reader->priv->view), doc);

		CainteoirMetadata *metadata = cainteoir_document_get_metadata(doc);
		gchar *mimetype = cainteoir_metadata_get_string(metadata, CAINTEOIR_METADATA_MIMETYPE);

		cainteoir_settings_set_string(reader->priv->settings, "document", "filename", filename);
		cainteoir_settings_set_string(reader->priv->settings, "document", "mimetype", mimetype);
		cainteoir_settings_save(reader->priv->settings);

		reset_timebar(reader);

		cainteoir_document_index_build(CAINTEOIR_DOCUMENT_INDEX(reader->priv->index), doc,
		                               gtk_combo_box_get_active_id(GTK_COMBO_BOX(reader->priv->index_type)));

		if (mimetype) g_free(mimetype);
		g_object_unref(metadata);
		g_object_unref(doc);
		return TRUE;
	}
	return FALSE;
}
