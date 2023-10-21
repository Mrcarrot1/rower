/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef GOPHERBROWSER_UI_H
#define GOPHERBROWSER_UI_H

#include <stdatomic.h>
#define GTK_DISABLE_DEPRECATED
#define GDK_DISABLE_DEPRECATED
#include <gtk/gtk.h>
#include "gopher-protocol.h"

#define DEFAULT_ALIGNMENT GTK_ALIGN_START
#define SET_DEFAULT_ALIGNMENT(_w) gtk_widget_set_halign(_w, DEFAULT_ALIGNMENT)

void *load_page(const char *page);
void *load_page_ex(const char *host, const char *selector, int port, gopherEntityType type);
void activate_ui(GtkApplication *app, gpointer user_data);
void render_gopher_entity_to_gtk(GtkBox *box, gopherEntity *entity);
guint gb_gtk_ext_entry_buffer_append_text(GtkEntryBuffer *buffer, const char *contents);
GtkWidget *gb_gtk_ext_icon_label_button(const char *iconName, const char *label);

#endif //GOPHERBROWSER_UI_H