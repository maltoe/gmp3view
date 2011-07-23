//:folding=explicit:

//{{{ License
/*
*   This file is part of gmp3view.
*   (c) 2006-2008 Malte Rohde
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
#ifdef _DEBUG_
using namespace std;
#include <iostream>
#endif
#include <gtk/gtk.h>
#include "interface.h"
#include "config.h"
#include "db.h"
#include "collect.h"
//}}}

//{{{ Main
int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	if(initialise_database())
		return 1;
	
	/* first argument means update/create database */
	if(argc > 1) {

			string basedir = argv[1];
			if(!g_file_test(basedir.c_str(), G_FILE_TEST_IS_DIR)) {
				cerr << "Error. First argument has to be a directory." << endl;
				return 1;
			}
			
			// If no third parameter is given, re-create the db.
			if(argc == 2)
  			generate_database(basedir);

#ifdef _DEBUG_
		  GTimer *tim;
		  tim = g_timer_new();
		  g_timer_start(tim);
#endif				  
	    /* Scan library */
	    cout << "Collecting data..." << endl;
	    collect(basedir, (argc > 2));	
#ifdef _DEBUG_
			cout << "Time elapsed while updating the db: " << g_timer_elapsed(tim, NULL) << endl;
			g_timer_stop(tim);
#endif
	}
	
	run_interface();
	
	close_database();
	return 0;
}
//}}}
