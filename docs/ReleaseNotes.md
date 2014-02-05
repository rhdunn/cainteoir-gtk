Release Notes
=============

Version 0.8: Saturday December 1st 2012 \[Nov 2012\]
----------------------------------------------------

  *  persist the voice settings (name, pitch, rate, ...)
  *  update the timebar after changing voice settings to reflect the new estimated read time
  *  selecting table of content entries now takes you to the text instead of the blank line
     before the text
  *  map the new Cainteoir Engine styles structure to text view formatting (GtkTextTag objects)

Version 0.7: Monday October 1st 2012 \[Sep 2012\]
-------------------------------------------------

  *  switch to glib-gettextize/intltool for localization
  *  update the translations and support more languages
  *  use a GtkBuilder UI file for most of the UI layout (requires glade >= 3.13.1 to edit)
  *  make the top-bar styling work for the Ambiance theme in Ubuntu 12.10
  *  use C++11 range-based for loops

Version 0.6: Tuesday July 31st 2012 \[Jul 2012\]
------------------------------------------------

  *  display the document text in the Document tab
  *  support drag and drop to change the open document
  *  convert the NEWS and README files to the markdown format

### bug fixes:

  *  fix a crash when opening a document with mismatched html tags

Version 0.5: Friday June 1st 2012 \[Jun 2012\]
----------------------------------------------

  *  moved the recent documents into a document library pane.
  *  improve the styling of the top and bottom bars.
  *  improve the linked button styling on the Ambiance and Radiance themes.

### bug fixes:

  *  the metadata pane is now scrollable to handle large description text.

Version 0.4: Saturday March 31st 2012 \[Mar 2012\]
--------------------------------------------------

  *  initial translations from OSTD [http://littlesvr.ca/ostd].
  *  hide the titlebar when maximized on GNOME 3.4 and later.
  *  report 'Document type is not supported' when loading fails instead of
     clearing the currently open document.
  *  support filtering voices by the current document's language.

### bug fixes:

  *  fix reading/recording section ranges (it would miss the last section).

Version 0.3: Sunday January 29th 2012 \[Nov 2012\]
--------------------------------------------------

  *  redesigned the UI -- make it consistent with the GNOME 3 application designs.
  *  don't show the "unable to open document" dialog when launching the program.
  *  keep the previous document if opening a document fails.
  *  require gtk+ >= 3.0.

Version 0.2: Tuesday November 22nd 2011 \[Nov 2011\]
----------------------------------------------------

### new functionality:

  *  display the voices in a voice list and support selecting the voice.
  *  switch the voice to match the document specified language if different.
  *  show the voice and engine details in the information/metadata view.
  *  display an error dialog if an error occurs while speaking.
  *  make the recent documents dialog wider (for Gtk+ 3).

### bug fixes:

  *  fix progress indicators when using the Radiance and Ambiance themes.
  *  don't crash if no file is selected from the recent documents UI.
  *  fix selecting the recorded file format based on file extension (was always Ogg/Vorbis).
  *  fix opening documents containing spaces.

Version 0.1 -- Sunday August 7th 2011 \[Aug 2011\]
--------------------------------------------------

### cainteoir-gtk:

  *  read and record text documents, reporting progress;
  *  view document metadata and table of contents;
  *  select the start and end chapter from the table of contents;
  *  view and edit voice speed, pitch and volume.
