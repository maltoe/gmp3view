//:folding=explicit:

//{{{ License
/*
*   This file is part of gmp3view.
*   (c) 2006-2011 Malte Rohde
*
*   gmp3view is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   gmp3view is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with gmp3view; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//}}}

using namespace std;
#include <string>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "searchwin.h"

//{{{ Toggles
static GtkWidget *window;
static GtkWidget *albums_radio;
static GtkWidget *tracks_radio;
static GtkWidget *artist_toggle;
static GtkWidget *year_toggle;
static GtkWidget *title_toggle;
//}}}

//{{{ Callbacks
static gboolean delete_event(GtkWidget * widget, GdkEvent * event,
			     gpointer data)
{
	string *query = (string*) data;
	query->erase();
	
	gtk_main_quit();
	return FALSE;
}

static void destroy(GtkWidget * widget, gpointer data)
{
	gtk_main_quit();
}

gboolean entry_key_pressed(GtkWidget *entry, GdkEventKey *event, gpointer data)
{
	const gchar *query = gtk_entry_get_text(GTK_ENTRY(entry));
	string *result = (string*) data;
	
	if(event->keyval == GDK_KEY_Escape) {
		result->erase();
		gtk_widget_destroy(window);
		return FALSE;
	}
		
	if(event->keyval != GDK_KEY_Return)
		return FALSE;
	
	if(g_utf8_collate(query, "") == 0)
		return FALSE;

	/* Ignore case */
	gchar *lower = g_utf8_strdown(query, g_utf8_strlen(query, -1));

	string artist;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(artist_toggle))) {
		artist = "OR LOWER(artist) LIKE '%";
		artist += lower;
		artist += "%' ";
	}

	string title;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(title_toggle))) {
		title = "OR LOWER(title) LIKE '%";
		title += lower;
		title += "%' ";
	}	
	
	string year;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(year_toggle))
		&& !(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tracks_radio)))) {
		year = "OR LOWER(year) LIKE '%";
		year += lower;
		year += "%' ";
	}		
		
	g_free(lower);
	
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tracks_radio)))
		result->append("SELECT * FROM tracks WHERE title LIKE 'notfound' ");
	else
		result->append("SELECT * FROM albums WHERE title LIKE 'notfound' ");
	
	result->append(artist);
	result->append(title);
	result->append(year);
	
	if(artist.empty() && title.empty() && year.empty())
		result->erase();
	
	gtk_widget_destroy(window);
	
	return FALSE;
}

void albums_toggled(GtkToggleButton *tb, gpointer data)
{
	int *ptable = (int*)data;
	*ptable = 0;
	
	gtk_widget_set_sensitive(year_toggle, TRUE);
}

void tracks_toggled(GtkToggleButton *tb, gpointer data)
{
	int *ptable = (int*)data;
	*ptable = 1;
	
	gtk_widget_set_sensitive(year_toggle, FALSE);
}
//}}}

int search_popup(string *query)
{
	/* Temporary sql query holder and table toggle*/
	string result;
	int table = 0;
	
	/* Create window */	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 50);
	gtk_window_set_title(GTK_WINDOW(window), "Search");
	g_signal_connect(G_OBJECT(window), "delete_event",
			 G_CALLBACK(delete_event), &result);
	g_signal_connect(G_OBJECT(window), "destroy",
			 G_CALLBACK(destroy), NULL);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
	
	/* Create search field */
	GtkWidget *entry;
	entry = gtk_entry_new();
	g_signal_connect(entry, "key-press-event", G_CALLBACK(entry_key_pressed), &result);
	gtk_widget_show(entry);
	
	/* Create radio buttons for table selection */
	albums_radio = gtk_radio_button_new_with_label(NULL, "albums");
	g_signal_connect(G_OBJECT(albums_radio), "toggled",
			 G_CALLBACK(albums_toggled), &table);
	gtk_widget_show(albums_radio);
	tracks_radio = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(albums_radio), (const gchar*)"tracks");
	g_signal_connect(G_OBJECT(tracks_radio), "toggled",
			 G_CALLBACK(tracks_toggled), &table);
	gtk_widget_show(tracks_radio);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(albums_radio), TRUE);
	GtkWidget *hbox1 = gtk_hbox_new(0, TRUE);
	gtk_box_pack_start(GTK_BOX(hbox1), albums_radio, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox1), tracks_radio, TRUE, TRUE, 0);
	gtk_widget_show(hbox1);
	
	/* Create bottom toggles (where to search in) */
	artist_toggle = gtk_check_button_new_with_label("Artist");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(artist_toggle), TRUE);
	gtk_widget_show(artist_toggle);
	title_toggle = gtk_check_button_new_with_label("Title");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(title_toggle), TRUE);
	gtk_widget_show(title_toggle);
	year_toggle = gtk_check_button_new_with_label("Year");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(year_toggle), TRUE);
	gtk_widget_show(year_toggle);
	GtkWidget *hbox2 = gtk_hbox_new(0, TRUE);
	gtk_box_pack_start(GTK_BOX(hbox2), artist_toggle, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox2), title_toggle, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox2), year_toggle, FALSE, FALSE, 0);
	gtk_widget_show(hbox2);
	
	/* Create a ruler */
	GtkWidget *sep = gtk_hseparator_new();
	gtk_widget_show(sep);
	
	/* Pack it all into a vbox */
	GtkWidget *vbox;
	vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), entry, TRUE, TRUE, 0);	
	gtk_box_pack_start(GTK_BOX(vbox), hbox1, TRUE, TRUE, 0);	
	gtk_box_pack_start(GTK_BOX(vbox), sep, TRUE, TRUE, 0);	
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, TRUE, TRUE, 0);	
	gtk_widget_show(vbox);
	
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(window);
	
	gtk_main();
	
	*query = result;
	return table;
}

