#ifndef __DB_H__
#define __DB_H__

int initialise_database(void);
void close_database(void);
void execute_query(string query, int (*callback)(void*,int,char**,char**), void *data);
void append_to_albums(string artist, string title, string year, string path);
void append_to_albums(string artist, string title, string year, string path, string cover);
void append_to_titles(string albumid, string artist, string title, string tracknumber, string path);
void append_to_tags(string albumid, string tag, string number);
int generate_database(string basedir);

/***********************
 *** Database layout ***
 ***********************

TABLE albums (
	id INTEGER PRIMARY KEY AUTOINCREMENT, 
	artist TEXT, 
	title TEXT, 
	year INTEGER, 
	path TEXT, 
	cover TEXT
)

TABLE tracks (
	albumid INTEGER, 
	artist TEXT, 
	title TEXT, 
	tracknumber TEXT, 
	path TEXT
)

Soon:
TABLE tags (
	albumid INTEGER,
	tag TEXT,
	number INTEGER
)


*/

#endif /* __DB_H__ */
