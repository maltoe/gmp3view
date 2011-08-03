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

//{{{ Includes
using namespace std;
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "db.h"
#include "interface.h"
#include "config.h"
#include "base64.h"
#include "jpeg.h"
#include "extern.h"
#include "searchwin.h"
//}}}

//{{{ Prototypes
GtkWidget *prepare_window(void);
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
static void destroy(GtkWidget *widget, gpointer data);
GtkWidget *prepare_treeview(void);
GtkTreeStore *create_tstore();
void fill_tstore(GtkTreeStore *tstore);
gboolean tv_button_pressed(GtkWidget *tv, GdkEventButton *event, gpointer data);
GtkWidget *prepare_iconview(void);
GtkListStore *create_istore();
gboolean iv_key_pressed(GtkWidget *iv, GdkEventKey *event, gpointer data);
static gboolean iv_button_pressed(GtkWidget *iv, GdkEventButton *event, gpointer data);
gboolean iv_tooltip_queried(GtkWidget *iv, gint x, gint y, gboolean km, GtkTooltip *tooltip, gpointer data);
bool compare_treeiters(GtkTreeIter *a, GtkTreeIter *b);
void add_em_all(GtkIconView *iv, GtkTreePath *path, gpointer data);
gboolean free_pixbufs(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data);
GtkWidget *prepare_search(void);
void search_clicked(GtkButton *button, gpointer data);
GtkWidget *prepare_typeahead(void);
gboolean typeahead_key_released(GtkWidget *typeahead, GdkEventKey *event, gpointer data);
string ToUpper(string str);
int tstore_tags_cb(void *data, int argc, char **argv, char **azColName);
int albums_execute_query_cb(void *data, int argc, char **argv, char **azColName);
int tooltip_execute_query_cb(void *data, int argc, char **argv, char **azColName);

enum {
	COL_PATH,			//path to the album like S/shakira
	COL_ID,				//ID: sql primary int
	COL_DISPLAY,		//string to display _below_ the icon (Artist\nTitle\nYear)
	COL_PIXBUF,			//icon
	NUM_ICOLS
};

enum {
	COL_TEXT,
	COL_QUERY,
	NUM_TCOLS
};
//}}}

//{{{ Globals
static GtkWidget *iv;
static GtkWidget *tv;
static GtkWidget *scrolled_window;
static GtkWidget *typeahead;
static bool block_clicks = false;
static bool shutdown = false;
static GtkTreeModel *prev_istore = NULL;
static GtkAdjustment *prev_vadjust;
static GtkTreeIter last_album_hovered;
static string tooltip_markup;
//}}}

//{{{ Main
int run_interface(void)
{
	prepare_window();	
	
	gtk_main();		
	
	return 0;
}
//}}}

//{{{ Window
GtkWidget *prepare_window(void)
{	
	/* Create a new window */
	GtkWidget *window;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 700, 500);
	gtk_window_set_title(GTK_WINDOW(window), "gmp3view " VERSION);
	g_signal_connect(G_OBJECT(window), "delete_event",
			 G_CALLBACK(delete_event), NULL);
	g_signal_connect(G_OBJECT(window), "destroy",
			 G_CALLBACK(destroy), NULL);

	/* set css style */
	string css = getenv("HOME");
	css += CFG_PREFIX;
	css += STYLESHEET;
	GtkCssProvider *cssp = gtk_css_provider_new();
	GError *err = NULL;
	if(gtk_css_provider_load_from_path(cssp, css.c_str(), &err) != TRUE) {
		if(err->domain == G_FILE_ERROR && err->code == G_FILE_ERROR_NOENT)
			cout << "No stylesheet found." << endl;
		else
			cerr << err->message << endl;
		g_error_free(err);
	} else {
		gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);
	}

	/* create widgets */

	GtkWidget *tv = prepare_treeview();		
	GtkWidget *search = prepare_search();
	GtkWidget *iv = prepare_iconview();
	GtkWidget *typeahead = prepare_typeahead();

	GtkWidget *vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), tv, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), search, FALSE, FALSE, 0);
	gtk_widget_show(vbox1);
	
	GtkWidget *vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), iv, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), typeahead, FALSE, FALSE, 0);
	gtk_widget_show(vbox2);
	
	GtkWidget *hbox = gtk_hbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);
	gtk_widget_show(hbox);

	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(window);

	return window;
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	shutdown = true;
	gtk_main_quit();
	return FALSE;
}

static void destroy(GtkWidget *widget, gpointer data)
{
	shutdown = true;
	gtk_main_quit();
}
//}}}

//{{{ Treeview


GtkWidget *prepare_treeview(void) {
	GtkTreeStore *tstore = create_tstore();
	
  tv = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tstore));
	
	GtkTreeViewColumn *col = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", COL_TEXT);
	
	fill_tstore(tstore);
	
    g_signal_connect(tv, "button-press-event", G_CALLBACK(tv_button_pressed), NULL);
	g_object_set(G_OBJECT(tv), "can-focus", FALSE, NULL);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv), FALSE);
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tv), FALSE);
	gtk_widget_show(tv);

	/* Make the Treeview scrollable */
	GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll), tv);
	gtk_widget_show(scroll);
	
	return scroll;
}

GtkTreeStore *create_tstore()
{
	GtkTreeStore *tstore;

	tstore = gtk_tree_store_new(NUM_TCOLS, G_TYPE_STRING, G_TYPE_STRING);

	return tstore;	
}

void fill_tstore(GtkTreeStore *tstore)
{
	/* Add the alphabet per default*/
	
	GtkTreeIter parent;
	gtk_tree_store_insert_with_values(tstore, &parent, NULL, 10000, COL_TEXT, "By Letter", -1);
	
	string cmd = "SELECT * FROM albums WHERE artist NOT GLOB '[A-Z]*' ORDER BY artist, year ASC";
	const char *buf = "123";
	gtk_tree_store_insert_with_values(tstore, NULL, &parent, 10000, COL_TEXT, buf, COL_QUERY, g_strdup(cmd.c_str()), -1);	
	
	for (char i = 65; i < 91; i++) {
		string base = "SELECT * FROM albums WHERE artist LIKE '";
		string cmd = base;
		cmd += i;
		cmd += "%' ORDER BY artist, year ASC";
		char buf[2] = {i, '\0'};
		gtk_tree_store_insert_with_values(tstore, NULL, &parent, 10000, COL_TEXT, buf, COL_QUERY, g_strdup(cmd.c_str()), -1);
	}
	
	/* Add the top tags per default */
	GtkTreeIter *tagparent = new GtkTreeIter;
	gtk_tree_store_insert_with_values(tstore, tagparent, NULL, 10000, COL_TEXT, "Top tags", -1);
	string tagcmd = "SELECT tag FROM tags GROUP BY tag HAVING SUM(number) > 1000";
	execute_query(tagcmd, tstore_tags_cb, (void*) tagparent);	
	
	// HOME/.gmp3view/query.lst
	GtkTreeIter *parent_p = NULL;
	string path = getenv("HOME");
	path += CFG_PREFIX;
	path += QUERIES;
	
	fstream fin;
	fin.open(path.c_str());
	if(!fin.good())
		return;
	
	string line;
	while (!fin.eof() && !fin.fail()) {
		getline(fin, line);
		
		// Ignore lines starting with a sharp
		if(line[0] == '#')
			continue;	
		
		// New category
		if(line[0] == '[') {
			size_t pos = line.find_last_of(']');
			if(pos == string::npos) {
				cerr << QUERIES << ": Syntax error." << endl;
				continue;
			}
			
			string category = line.substr(1, (pos - 1));
			gtk_tree_store_insert_with_values(tstore, &parent, NULL, 10000, COL_TEXT, g_strdup(category.c_str()), -1);
			parent_p = &parent;		
			continue;
		}
		
		// New query
		size_t pos = line.find_first_of('=');
		if(pos == string::npos) {
			cerr << QUERIES << ": Syntax error." << endl;
			continue;
		}		
		
		string name, query;
		name = line.substr(0, pos);
		query = line.substr(pos + 1);
		gtk_tree_store_insert_with_values(tstore, NULL, parent_p, 10000, 
									COL_TEXT, g_strdup(name.c_str()), 
									COL_QUERY, g_strdup(query.c_str()), -1);		
	}
	
	fin.close();
}	

gboolean tv_button_pressed(GtkWidget *tv, GdkEventButton *event, gpointer data)
{
	if(block_clicks)
		return TRUE;
	
	if(!(event->type == GDK_BUTTON_PRESS  &&  event->button == 1))
		return FALSE;
	
	GtkTreeStore *tstore = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tv)));
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *buf;

	/* Get tree path for row that was clicked */
	if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tv),
                (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
	{
		gtk_tree_model_get_iter(GTK_TREE_MODEL(tstore), &iter, path);
		gtk_tree_model_get(GTK_TREE_MODEL(tstore), &iter, COL_QUERY, &buf, -1);	
		gtk_tree_path_free(path);	
		if(buf) {
			/* Clear pixbufs */
			GtkTreeModel *istore = gtk_icon_view_get_model(GTK_ICON_VIEW(iv));
			gtk_tree_model_foreach(istore, free_pixbufs, NULL);
			gtk_list_store_clear(GTK_LIST_STORE(istore));
			
			/* Clear previous view */
			if(prev_istore) {
				gtk_tree_model_foreach(prev_istore, free_pixbufs, NULL);
				gtk_list_store_clear(GTK_LIST_STORE(prev_istore));
				prev_istore = NULL;
			}
			if(prev_vadjust) {
				prev_vadjust = 0;
			}
			
			gtk_entry_set_text(GTK_ENTRY(typeahead), "");
			gtk_widget_grab_focus(typeahead);
			
			g_object_set(G_OBJECT(iv), "has-tooltip", TRUE, NULL);
#ifdef _DEBUG_
			GTimer *tim;
			tim = g_timer_new();
			g_timer_start(tim);
#endif
			execute_query(buf, albums_execute_query_cb, NULL);
#ifdef _DEBUG_
			cout << "Time elapsed executing SQL command: " << g_timer_elapsed(tim, NULL) << endl;
			g_timer_stop(tim);
#endif
			g_free(buf);
		}
	}
	return FALSE;
}
//}}}

//{{{ Iconview
GtkWidget *prepare_iconview(void) 
{
	/* Prepare ListStore */
	GtkListStore *istore = create_istore();
	
	/* Prepare icon view */
	iv = gtk_icon_view_new_with_model(GTK_TREE_MODEL(istore));
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(iv), COL_DISPLAY);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(iv), COL_PIXBUF);
	g_signal_connect(iv, "button_press_event", G_CALLBACK(iv_button_pressed), NULL);
	g_signal_connect(iv, "key_press_event", G_CALLBACK(iv_key_pressed), NULL);
	g_signal_connect(iv, "query_tooltip", G_CALLBACK(iv_tooltip_queried), NULL);
	gtk_widget_show(iv);
	
	/* Adjust global tooltip timeout property */
	gtk_settings_set_long_property(gtk_settings_get_default(), "gtk-tooltip-timeout", 900, NULL);
	gtk_settings_set_long_property(gtk_settings_get_default(), "gtk-tooltip-browse-mode-timeout", 0, NULL);
	
	/* Make the Iconview scrollable */
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrolled_window), iv);
	gtk_widget_show(scrolled_window);
	
	return scrolled_window;
}

GtkListStore *create_istore()
{
	GtkListStore *istore;

	istore = gtk_list_store_new(NUM_ICOLS, G_TYPE_STRING, G_TYPE_STRING,
							G_TYPE_STRING, GDK_TYPE_PIXBUF);
			 
	return istore;
}

gboolean iv_key_pressed(GtkWidget *iv, GdkEventKey *event, gpointer data)
{
	if(event->keyval == GDK_KEY_Return)
		gtk_icon_view_selected_foreach(GTK_ICON_VIEW(iv), (GtkIconViewForeachFunc) add_em_all, NULL);
	
	return FALSE;
}

static gboolean iv_button_pressed(GtkWidget *iv, GdkEventButton *event, gpointer data)
{
	if(block_clicks)
		return FALSE;
	
	if(event->button == 3 && event->type == GDK_2BUTTON_PRESS) {
		GtkTreeModel *istore;
		GtkTreePath *tp;
		GtkTreeIter iter;
		gchar *albumid;
		gchar *path;
		string cover;

		tp = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(iv), (gint)event->x, (gint)event->y);
		if(tp) {
			istore = gtk_icon_view_get_model(GTK_ICON_VIEW(iv));
			gtk_tree_model_get_iter(istore,	&iter, tp);
			gtk_tree_model_get(istore, &iter, COL_ID, &albumid, -1);
			
			if(albumid) {
				if(!(albumid[0] == '-')) {
					gtk_tree_model_get(GTK_TREE_MODEL(istore), &iter, COL_PATH, &path, -1);
					cover = path;
					cover += "/folder.jpg";
					viewer_show(cover);
					g_free(path);
				}
				g_free(albumid);
			}
		}	
	}
	
	if(event->button == 1 && event->type == GDK_2BUTTON_PRESS) {
		GtkTreeModel *istore;
		GtkTreePath *tp;
		GtkTreeIter iter;
		gchar *path;

		tp = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(iv), (gint)event->x, (gint)event->y);
		if(tp) {
			istore = gtk_icon_view_get_model(GTK_ICON_VIEW(iv));
			gtk_tree_model_get_iter(istore,	&iter, tp);
			gtk_tree_model_get(istore, &iter, COL_PATH, &path, -1);
				
			player_enqueue(path);

			g_free(path);
		}
	}
	return FALSE;
}

gboolean iv_tooltip_queried(GtkWidget *iv, gint x, gint y, gboolean km, GtkTooltip *tooltip, gpointer data)
{
	string cmd;
	
	GtkTreeModel *istore;
	GtkTreePath *tp;
	GtkTreeIter iter;
	gchar *albumid;
	
	gboolean res;    
	res = gtk_icon_view_get_tooltip_context(GTK_ICON_VIEW(iv), &x, &y, km, &istore, &tp, &iter);
	if(!res)
		return FALSE;
	if(compare_treeiters(&last_album_hovered, &iter)) {
		gtk_tooltip_set_markup(tooltip, tooltip_markup.c_str());		
		return TRUE;
	}
	
	last_album_hovered = iter;
	istore = gtk_icon_view_get_model(GTK_ICON_VIEW(iv));
	gtk_tree_model_get(istore, &iter, COL_ID, &albumid, -1);
	tooltip_markup.erase();
		
	cmd = "SELECT * FROM tags WHERE albumid = '";
	cmd += albumid;
	cmd += "'";
	g_free(albumid);
	execute_query(cmd, tooltip_execute_query_cb, (void*) &tooltip_markup);	

	gtk_tooltip_set_markup(tooltip, tooltip_markup.c_str());	
		
	return TRUE;
}

bool compare_treeiters(GtkTreeIter *a, GtkTreeIter *b)
{
	if(a->stamp != b->stamp)
		return false;
	if(a->user_data != b->user_data)
		return false;
	if(a->user_data2 != b->user_data2)
		return false;
	if(a->user_data3 != b->user_data3)
		return false;
	
	return true;
}

gboolean free_pixbufs(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	GdkPixbuf *pixbuf;
	gtk_tree_model_get(model, iter, COL_PIXBUF, &pixbuf, -1);
	if(pixbuf)
		g_free(gdk_pixbuf_get_pixels(pixbuf));
	return FALSE;
}

void add_em_all(GtkIconView *iv, GtkTreePath *path, gpointer data)
{
	GtkListStore *istore;
	GtkTreeIter iter;
	gchar *cpath;

	istore = GTK_LIST_STORE(gtk_icon_view_get_model(GTK_ICON_VIEW(iv)));

	gtk_tree_model_get_iter(GTK_TREE_MODEL(istore), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(istore), &iter, COL_PATH, &cpath, -1);
	player_enqueue(cpath);

	g_free(cpath);	
}
//}}}

//{{{ Search button
GtkWidget *prepare_search(void) {
	GtkWidget *button = gtk_button_new_with_label("Search");
	g_signal_connect(button, "clicked", G_CALLBACK(search_clicked), NULL);
	gtk_widget_show(button);
	
	return button;
}

void search_clicked(GtkButton *button, gpointer data)
{
	if(block_clicks)
		return;
	
	string result;
	int tracks;
	tracks = search_popup(&result);
	if(!result.empty()) {
		/* Clear pixbufs */
		GtkTreeModel *istore = gtk_icon_view_get_model(GTK_ICON_VIEW(iv));
		gtk_tree_model_foreach(istore, free_pixbufs, NULL);
		gtk_list_store_clear(GTK_LIST_STORE(istore));
			
		gtk_entry_set_text(GTK_ENTRY(typeahead), "");
		gtk_widget_grab_focus(typeahead);
			
#ifdef _DEBUG_
		GTimer *tim;
		tim = g_timer_new();
		g_timer_start(tim);
#endif
		if(tracks)
			cout << "No support for tracks atm." << endl;	
		else
			execute_query(result, albums_execute_query_cb, NULL);
#ifdef _DEBUG_
		cout << "Time elapsed while executing SQL command: " << g_timer_elapsed(tim, NULL) << endl;
		g_timer_stop(tim);
#endif
	}
}
//}}}

//{{{ Typeahead find
GtkWidget *prepare_typeahead(void) {
	typeahead = gtk_entry_new();
	g_signal_connect(typeahead, "key-release-event", G_CALLBACK(typeahead_key_released), NULL);
	gtk_widget_show(typeahead);
	
	return typeahead;
}

gboolean typeahead_key_released(GtkWidget *typeahead, GdkEventKey *event, gpointer data)
{
	switch(event->keyval) {
		case GDK_KEY_Return:
			gtk_icon_view_selected_foreach(GTK_ICON_VIEW(iv), (GtkIconViewForeachFunc) add_em_all, NULL);
			break;
		case GDK_KEY_Escape:
			gtk_entry_set_text(GTK_ENTRY(typeahead), "");
			break;
		default:
			/* Simple typeahead search */
			string pattern = gtk_entry_get_text(GTK_ENTRY(typeahead));
			if(pattern.empty())
				break;
			pattern = ToUpper(pattern);
			
			bool pattern_found = false;
			
			GtkTreeModel *istore = gtk_icon_view_get_model(GTK_ICON_VIEW(iv));
			GtkTreeIter iter;
			if(!gtk_tree_model_get_iter_first(istore, &iter))
				break;
			
			do {
				if(pattern_found)
					continue;
				
				gchar *display;
				gtk_tree_model_get(istore, &iter, COL_DISPLAY, &display, -1);
				string tmp = display;
				tmp = ToUpper(tmp);
				if(tmp.find(pattern) != string::npos) {
					
					/* TODO: This seems too complicated. */
					GtkTreePath *path;
					gchar *strpath;
					strpath = gtk_tree_model_get_string_from_iter(istore, &iter);
					path = gtk_tree_path_new_from_string(strpath);
					gtk_icon_view_select_path(GTK_ICON_VIEW(iv), path);
					gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(iv), path, FALSE, 0, 0);
					gtk_tree_path_free(path);
					g_free(strpath);
					
					pattern_found = true;
				}
				g_free(display);
			} while(gtk_tree_model_iter_next(istore, &iter) != FALSE);
			
			/* if pattern not found, select none. */
			if(!pattern_found)
				gtk_icon_view_unselect_all(GTK_ICON_VIEW(iv));
				
			break;
	}
	
	return FALSE;
}

string ToUpper(string str)
{
	for(unsigned int i = 0; i < str.length(); i++)
		str[i] = toupper(str[i]);
	
	return str;
}
//}}}

//{{{ SQL callbacks
int tstore_tags_cb(void *data, int argc, char **argv, char **azColName)
{
  if (argc != 1)
    return 1;

  GtkTreeStore *tstore = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tv)));
  GtkTreeIter *parent = (GtkTreeIter*) data;

  string query = "SELECT DISTINCT id,path,artist,title,year,cover FROM albums JOIN tags ON albums.id = tags.albumid WHERE tag = '";
  query += argv[0];
  query += "'AND number > 15 ORDER BY artist, year ASC";
  
	gtk_tree_store_insert_with_values(tstore, NULL, parent, 10000, 
							COL_TEXT, g_strdup(argv[0]), 
							COL_QUERY, g_strdup(query.c_str()), -1);		

  return 0;
}

int albums_execute_query_cb(void *data, int argc, char **argv, char **azColName)
{
	if (argc < 1)
		return 1;

	GtkListStore *istore = GTK_LIST_STORE(gtk_icon_view_get_model(GTK_ICON_VIEW(iv)));
	
	string artist;
	string year;
	string title;
	string path;
	string id;
	string display;
	
	GtkTreeIter iter;
	gtk_list_store_append(istore, &iter);
	for(int i = 0; i < argc; i++) {
		if(g_utf8_collate(azColName[i], "cover") == 0) {
			
			if(argv[i] == NULL)
				break;

			string buf64 = base64_decode(argv[i]);
			guchar *buf = g_new(guchar, buf64.length());
			for(unsigned int i = 0; i < buf64.length(); i++)
				buf[i] = buf64[i];
			
			guchar *pixels = jpeg_decompress(buf, buf64.length());
			g_free(buf);
			
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(pixels, GDK_COLORSPACE_RGB, 
					FALSE, 8, 200, 200, (200 * 3), 
					NULL, NULL); 
			gtk_list_store_set(istore, &iter, COL_PIXBUF, pixbuf, -1);
		} else if(g_utf8_collate(azColName[i], "id") == 0) {
			id = argv[i];
			char *buf = g_strdup(id.c_str());
			gtk_list_store_set(istore, &iter, COL_ID, buf, -1);
		} else if(g_utf8_collate(azColName[i], "path") == 0) {
			path = argv[i];
			char *buf = g_strdup(path.c_str());
			gtk_list_store_set(istore, &iter, COL_PATH, buf, -1);
		} else if(g_utf8_collate(azColName[i], "artist") == 0) {
			artist = argv[i];
		} else if(g_utf8_collate(azColName[i], "title") == 0) {
			title = argv[i];
		} else if(g_utf8_collate(azColName[i], "year") == 0) {
			year = argv[i];
		} else {
			// do nothing
		}
	}
		
	display = artist + "\n" + year + "\n" + title;
	gtk_list_store_set(istore, &iter, COL_DISPLAY, g_strdup(display.c_str()), -1);	
	
	/* Show immediately */
	block_clicks = true;
	while(gtk_events_pending())
		gtk_main_iteration_do(FALSE);
	block_clicks = false;

	if(shutdown)
		return 1;
	else
		return 0;
}

int tooltip_execute_query_cb(void *data, int argc, char **argv, char **azColName)
{
	if (argc < 1)
		return 1;

	string *markup = (string*) data;
	
	string tag;
	string num;
	for(int i = 0; i < argc; i++) {
		if(g_utf8_collate(azColName[i], "tag") == 0) {
			tag = argv[i];
		} else if(g_utf8_collate(azColName[i], "number") == 0) {
			num = argv[i];
		}
	}
		
	*markup += "<span size=\"medium\">";
	*markup += tag;
	*markup += " ";
	*markup += " </span>";
	
	return 0;
}
//}}}

