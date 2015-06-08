/* Header C compatibility tests (compile only).
 *
 * Copyright (C) 2015 Reece H. Dunn
 *
 * This file is part of cainteoir-engine.
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

#include <gtk/gtk.h>

// Check that all the cainteoir-gtk headers compile with a C compiler ...

#include <cainteoir-gtk/cainteoir_audio_data_s16.h>
#include <cainteoir-gtk/cainteoir_document.h>
#include <cainteoir-gtk/cainteoir_document_index.h>
#include <cainteoir-gtk/cainteoir_document_view.h>
#include <cainteoir-gtk/cainteoir_metadata.h>
#include <cainteoir-gtk/cainteoir_settings.h>
#include <cainteoir-gtk/cainteoir_speech_parameter.h>
#include <cainteoir-gtk/cainteoir_speech_synthesizers.h>
#include <cainteoir-gtk/cainteoir_speech_voice_view.h>
#include <cainteoir-gtk/cainteoir_supported_formats.h>
#include <cainteoir-gtk/cainteoir_timebar.h>
#include <cainteoir-gtk/cainteoir_waveform_view.h>

int main(int argc, char **argv)
{
	return 0;
}
