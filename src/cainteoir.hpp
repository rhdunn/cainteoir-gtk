/* Cainteoir Main Window
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

#ifndef CAINTEOIRGTK_SRC_CAINTEOIR_HPP
#define CAINTEOIRGTK_SRC_CAINTEOIR_HPP

#include <cainteoir/languages.hpp>

#include "settings.hpp"
#include "document.hpp"
#include "voice_selection.hpp"
#include "metadata.hpp"
#include "gtk-compatibility.hpp"

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
	GtkWidget *mediabar;

	Gtk::HPaned pane;

	Gtk::ScrolledWindow scrolledTocPane;

	Gtk::ScrolledWindow scrolledView;
	Gtk::VBox view;
	MetadataView doc_metadata;
	VoiceSelectionView voiceSelection;

	Gtk::HBox statusbar;
	Gtk::Label state;

	GtkWidget *progressAlignment;
	GtkWidget *progress;
	GtkWidget *elapsedTime;
	GtkWidget *totalTime;

	Glib::RefPtr<Gtk::UIManager> uiManager;
	Glib::RefPtr<Gtk::ActionGroup> actions;

	GtkObjectRef<Gtk::RecentFilter> recentFilter;
	Glib::RefPtr<Gtk::RecentManager> recentManager;
	Glib::RefPtr<Gtk::Action> recentAction;

	Gtk::ToolButton readButton;
	Gtk::ToolButton stopButton;
	Gtk::ToolButton recordButton;
	Gtk::MenuToolButton openButton;

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

#endif
