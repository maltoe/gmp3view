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
#include <cstdlib>
#include <string>
#include <iostream>
#include <sqlite3.h>
#include "db.h"
#include "config.h"

static sqlite3 *db;

int initialise_database(void)
{
	string dbpath = getenv("HOME");
	dbpath += CFG_PREFIX;
	dbpath += DATABASE;

	int rc = sqlite3_open(dbpath.c_str(), &db);
	if(rc) {
		cerr << "SQlite: " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
	}
	return rc;
}

void close_database(void)
{
	sqlite3_close(db);
}

void execute_query(string query, int (*callback)(void*,int,char**,char**), void *data)
{
	char *err;
	int rc = sqlite3_exec(db, query.c_str(), callback, data, &err);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}		
}

void append_to_albums(string artist, string title, string year, string path)
{
	string cmd = "INSERT INTO albums (artist, title, year, path) VALUES (\"";
	cmd += artist;
	cmd += "\", \"";
	cmd += title;
	cmd += "\", \"";
	cmd += year;
	cmd += "\", \"";
	cmd += path;
	cmd += "\")";
	
	char *err;
	int rc = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}		
}

void append_to_albums(string artist, string title, string year, string path, string cover)
{
	string cmd = "INSERT INTO albums (artist, title, year, path, cover) VALUES (\"";
	cmd += artist;
	cmd += "\", \"";
	cmd += title;
	cmd += "\", \"";
	cmd += year;
	cmd += "\", \"";
	cmd += path;
	cmd += "\", \"";
	cmd += cover;
	cmd += "\")";
	
	char *err;
	int rc = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}		
/*	
	sqlite3_blob *blob;
	int sqlite3_blob_open(db, NULL, "albums", "cover", , 120000, &blob);

	rc = sqlite3_blob_write(&blob, cover, 600, 0);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}		
	
	sqlite3_blob_close(&blob);*/
}

void append_to_titles(string albumid, string artist, string title, string tracknumber, string path)
{
	string cmd = "INSERT INTO tracks (albumid, artist, title, tracknumber, path) VALUES (\"";
	cmd += albumid;
	cmd += "\", \"";
	cmd += artist;
	cmd += "\", \"";
	cmd += title;
	cmd += "\", \"";
	cmd += tracknumber;
	cmd += "\", \"";
	cmd += path;
	cmd += "\")";
	
	char *err;
	int rc = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}		
}

void append_to_tags(string albumid, string tag, string number)
{
	string cmd = "INSERT INTO tags (albumid, tag, number) VALUES (\"";
	cmd += albumid;
	cmd += "\", \"";
	cmd += tag;
	cmd += "\", \"";
	cmd += number;
	cmd += "\")";
	
	char *err;
	int rc = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}	
}

int generate_database(string basedir)
{
	/* Clean up */
	cout << "Cleaning up..." << endl;
	string cmd = "DROP TABLE IF EXISTS albums; DROP TABLE IF EXISTS tracks; DROP TABLE IF EXISTS tags; VACUUM";
	
	char *err;
	int rc = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}			
		
	cmd.erase();
	cmd = "CREATE TABLE albums (id INTEGER PRIMARY KEY AUTOINCREMENT, artist TEXT, title TEXT, year INTEGER, path TEXT, cover TEXT)";
	
	rc = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}			

	cmd.erase();
	cmd = "CREATE TABLE tracks (albumid INTEGER, artist TEXT, title TEXT, tracknumber TEXT, path TEXT)";
	
	rc = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}
	
	cmd.erase();
	cmd = "CREATE TABLE tags (albumid INTEGER, tag TEXT, number INTEGER)";
	
	rc = sqlite3_exec(db, cmd.c_str(), NULL, NULL, &err);
	if(rc != SQLITE_OK) {
		cerr << "SQLite: " << err << endl;
		sqlite3_free(err);
	}

	return 0;
}
