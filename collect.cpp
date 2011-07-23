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
#include <string>
#include <fstream>
#include <iostream>
#include <gtk/gtk.h>
#include "base64.h"
#include "jpeg.h"
#include "db.h"
#include "config.h"
//}}}

//{{{ Prototypes
int add_directory(string letter);
void add_album(string letter, string album);
int album_exists_cb(void *data, int argc, char **argv, char **azColName);
int add_album_real(string letter, string album);
int add_file(string albumid, string path);
int album_added_cb(void *data, int argc, char **argv, char **azColName);
int scan_dir(string path, string albumid);
int load_album_tags(string albumid, string artist, string title);
int load_artist_tags(string albumid, string artist);
int parse_filename(string name, string *artist, string *title, string *tracknumber);
string extract_artist(string name);
string extract_year(string name);
string extract_title(string name);
GdkPixbuf *load_image(string path);
void create_tag_dir(void);
int parse_toptags(string toptags, string albumid);
//}}}

//{{{ Globals
static string basedir;
static bool isUpdate;
//}}}

//{{{ Main
int collect(string _basedir, bool _isUpdate) {
	
	if(*(--_basedir.end()) == '/')
		basedir = _basedir;
	else
		basedir = _basedir + '/';
		
  isUpdate = _isUpdate;
	
	/* Check for existance of tags directory */
	create_tag_dir();
	
	/* This is a good position for adapting gmp3view for
	 * different library layouts (e.g. listed by genre or artist) 
	 */
	
	/* Numerical albums */
	add_directory("123");
	
	/* Add the alphabet */
	for (char i = 65; i < 91; i++) {
		char buf[2];
		buf[0] = i; buf[1] = '\0';
		add_directory(buf);
	}
	
	return 0;
}

int add_directory(string letter)
{
	GDir *dir;
	const gchar *name;
	string path;

	/* Now go through the directory and extract all the file
	 * information */
	path += basedir;
	path += letter;
	dir = g_dir_open(path.c_str(), 0, NULL);
	if (!dir)
		return 1;

	name = g_dir_read_name(dir);
	while (name != NULL) {

		/* We ignore files that start with a '.' */
		if (name[0] != '.') {
			add_album(letter, name);
		}
		name = g_dir_read_name(dir);
	}
	
	g_dir_close(dir);
	
	return 0;
}

struct album_basics {
  string path;
  string letter;
  string album;
};

void add_album(string letter, string album)
{
  if(!isUpdate)
    add_album_real(letter, album);
  else {
    album_basics* ab = new album_basics;
    ab->path = basedir + letter + '/' + album;
    ab->letter = letter;
    ab->album = album;
    
    string albumexists = "SELECT id FROM albums WHERE path = \"";
    albumexists += ab->path;
    albumexists += "\";";
    execute_query(albumexists, album_exists_cb, ab);
  }
}

int album_exists_cb(void *data, int argc, char **argv, char **azColName)
{
  album_basics* ab = (album_basics*) data;

  if(argc == 0)
    add_album_real(ab->letter, ab->album);
  
  delete ab;
  return 0;
}

int add_album_real(string letter, string album)
{
	string artist = extract_artist(album);
	string year = extract_year(album);
	string title = extract_title(album);
	string path = basedir + letter + '/' + album;
	
	GdkPixbuf *cover;
	string path_to_cover;
	path_to_cover += path;
	path_to_cover += "/folder.jpg";
	
	cover = load_image(path_to_cover);
	if(cover) {
		guchar *pixels = gdk_pixbuf_get_pixels(cover);
		unsigned int numbytes = gdk_pixbuf_get_rowstride(cover) * gdk_pixbuf_get_height(cover);
		
		guchar *buf = g_new(guchar, 50000);
		numbytes = jpeg_compress(pixels, buf, 50000);
		
		string buf64 = base64_encode(buf, numbytes);
		
		append_to_albums(artist, title, year, path, buf64);
		
		g_free(buf);
		g_object_unref(G_OBJECT(cover));
	} else	
		append_to_albums(artist, title, year, path);
		
	/* Recurse dir for files and download last.fm tags */
	string cmd = "SELECT id, path, artist, title FROM albums WHERE artist = \"";
	cmd += artist;
	cmd += "\" AND title = \"";
	cmd += title;
	cmd += "\"";
	execute_query(cmd, album_added_cb, NULL);
	
	return 0;
}

int add_file(string albumid, string path)
{
	if(path.find(".mp3") == string::npos)
		return 0;
	
	string artist, title, trackno;
	
	string::size_type pos = path.rfind('/');
	parse_filename(path.substr(pos + 1), &artist, &title, &trackno);
	
	append_to_titles(albumid, artist, title, trackno, path);
	
	return 1;
}
//}}}

//{{{ More information needed
int album_added_cb(void *data, int argc, char **argv, char **azColName)
{
	if (argc < 1)
		return 1;
	
	string albumid;
	string path;
	string artist;
	string title;
	
	for(int i = 0; i < argc; i++) {
		if(g_utf8_collate(azColName[i], "id") == 0) {
			albumid = argv[i];
		} else if(g_utf8_collate(azColName[i], "path") == 0) {
			path = argv[i];
		} else if(g_utf8_collate(azColName[i], "artist") == 0) {
			artist = argv[i];
		} else if(g_utf8_collate(azColName[i], "title") == 0) {
			title = argv[i];
		}
	}
	
	if(path.empty() || albumid.empty())
		return 1;
	
	/* Scan tracks */
	scan_dir(path, albumid);	
	
	/* Try to download tags */
	if(!artist.empty()) {
		if(load_album_tags(albumid, artist, title) < NUM_TAGS)
			load_artist_tags(albumid, artist);
	}
	return 0;	
}

int scan_dir(string path, string albumid)
{
	GDir *dir;
	const gchar *name;
	
	dir = g_dir_open(path.c_str(), 0, NULL);
	if (!dir)
		return 1;

	name = g_dir_read_name(dir);
	while (name != NULL) {
		
		if (name[0] != '.') {
			string fullpath = path + '/' + name;
			if(g_file_test(fullpath.c_str(), G_FILE_TEST_IS_DIR))
				scan_dir(fullpath, albumid);
			else
				add_file(albumid, fullpath);
		}
			
		name = g_dir_read_name(dir);
	}

	g_dir_close(dir);
	
	return 0;	
}

int load_album_tags(string albumid, string artist, string title)
{
	string toptags = getenv("HOME");
	toptags += CFG_PREFIX;
	toptags += "tags/";
	toptags += artist;
	toptags += "-";
	toptags += title;
	toptags += ".xml";	
	
	if(g_file_test(toptags.c_str(), G_FILE_TEST_EXISTS)) {
#ifdef _DEBUG_
		cout << "Reusing existing " << artist << "-" << title << ".xml" << endl;
#endif
		//do nothing
	} else {
#ifdef _DEBUG_
		cout << "Downloading tags for " << artist << "-" << title << endl;
#endif
		string wget = "wget -q -O \"";
		wget += toptags;
		wget += "\" \"http://ws.audioscrobbler.com/2.0/?method=album.gettoptags&artist=";
		wget += artist;
		wget += "&album=";
		wget += title;
		wget += "&autocorrect=1&api_key=";
		wget += LASTFM_KEY;
		wget += "\"";
		system(wget.c_str());	
		
		if(!g_file_test(toptags.c_str(), G_FILE_TEST_EXISTS)) {
			cout << "Could not load toptags.xml for " << title << endl;
			return 0;
		}
	}

  // TODO parse_toptags most likely won't parse the new album toptags correctly!
	return parse_toptags(toptags, albumid);	
}

int load_artist_tags(string albumid, string artist)
{
	string toptags = getenv("HOME");
	toptags += CFG_PREFIX;
	toptags += "tags/";
	toptags += artist;
	toptags += ".xml";

	if(g_file_test(toptags.c_str(), G_FILE_TEST_EXISTS)) {
#ifdef _DEBUG_
		cout << "Reusing existing " << artist << ".xml" << endl;
#endif
		//do nothing
	} else {
#ifdef _DEBUG_
		cout << "Downloading tags for " << artist << endl;
#endif /* _DEBUG_ */
	
		string wget = "wget -q -O \"";
		wget += toptags;
		wget += "\" \"http://ws.audioscrobbler.com/1.0/artist/";
		wget += artist;
		wget += "/toptags.xml\"";
		system(wget.c_str());
		
		if(!g_file_test(toptags.c_str(), G_FILE_TEST_EXISTS)) {
			cout << "Could not load toptags.xml for " << artist << endl;
			return 0;
		}
	}
	
	
	return parse_toptags(toptags, albumid);
	
}
//}}}

//{{{ Helpers
int parse_filename(string name, string *artist, string *title, string *tracknumber)
{
	string::size_type pos = name.find(" - ", 0);
	if (pos == string::npos) {
		*title = name.substr(0, name.length() - 4);
		return 1;
	} else {
		if(pos < 3) {
			*tracknumber = name.substr(0, pos);
		} else {
			*artist = name.substr(0, pos);
		}
		
		string::size_type pos2 = name.find(" - ", pos + 3);
		if (pos == string::npos) {
			*title = name.substr(pos + 3, name.length() - (pos + 3) - 4);
			return 2;
		} else {
			if((pos2 - (pos + 3)) < 3) {
				*tracknumber = name.substr(pos + 3, pos2 - (pos + 3));
			} else {
				*artist = name.substr(pos + 3, pos2 - (pos + 3));
			}

			*title = name.substr(pos2 + 3, name.length() - (pos2 + 3) - 4);			
		}
	}
	
	return 3;
}


string extract_artist(string name)
{
	string::size_type pos = name.find(" - ", 0);
	if (pos != string::npos) {
		return name.substr(0, pos);
	} else {
		/* This is a fallback. If there's no " - " inside the album title 
		 * we return the full name as the artist ... */
		return name;
	}
}

string extract_year(string name)
{
	string::size_type pos = name.find(" - ", 0);
	if (pos != string::npos) {
		string temp = name.substr(pos + 3);
		string::size_type pos2 = temp.find(" - ", 0);
		if (pos2 != string::npos) {
			return temp.substr(0, pos2);
		} else {
			/* if this occurs, the directory name is like ARTIST - YEAR,
			 * so we can simply return our temporary string */
			return temp;
		}
	} else {
		/* ... and zero for everything else, so everything should at least
		 * be displayed. */
		return "";
	}
}

string extract_title(string name)
{
	string::size_type pos = name.find(" - ", 0);
	if (pos != string::npos) {
		string temp = name.substr(pos + 3);
		string::size_type pos2 = temp.find(" - ", 0);
		if (pos2 != string::npos) {
			return temp.substr(pos2 + 3);
		} else {
			/* if this occurs, the directory name is like ARTIST - YEAR,
			 * so we return zero */
			return "";
		}
	} else {
		/* ... and zero for everything else, so everything should at least
		 * be displayed. */
		return "";
	}
}

GdkPixbuf *load_image(string path)
{
	GdkPixbuf *ret;
	GError *error = NULL;
	ret = gdk_pixbuf_new_from_file_at_scale(path.c_str(), 200, 200, FALSE, &error);
	if (!ret) {
		if (error->code != 4)
			cerr << error->message << endl;
		g_error_free(error);
	}

	return ret;
}

void create_tag_dir(void)
{
	string tagdir = getenv("HOME");
	tagdir += CFG_PREFIX;
	tagdir += "tags/";
	
	if(!g_file_test(tagdir.c_str(), G_FILE_TEST_IS_DIR)) {
		string mkdir = "mkdir -p ";
		mkdir += tagdir;
		system(mkdir.c_str());
	}
}

int parse_toptags(string toptags, string albumid)
{
	string tag;
	string number;
	int count = 0;
	
	fstream fin;
	fin.open(toptags.c_str());
	if(!fin.good()) {
		cerr << "Error reading " << toptags << endl;
		fin.close();
		return 1;
	}

	string line;
	while (!fin.eof() && !fin.fail()) {
		getline(fin, line);	
		if(line.find("<tag>") != string::npos) {

			if(fin.eof() || fin.fail()) {
				cerr << "Parsing error in " << toptags << "!" << endl;
				fin.close();
				return 1;
			}
			getline(fin, line);
			tag = line.substr(10, line.length() - 18);
			
			if(fin.eof() || fin.fail()) {
				cerr << "Parsing error in " << toptags << "!" << endl;
				fin.close();
				return 1;
			}			
			getline(fin, line);
			number = line.substr(11, line.length() - 19);
			
			append_to_tags(albumid, tag, number);
			if((count++ >= NUM_TAGS) && (NUM_TAGS > 0))
				break;
			
		}		
	}
	fin.close();
	
	return count;
}
//}}}
