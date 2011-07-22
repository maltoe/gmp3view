//:folding=explicit:
/* triple '{' commands are jEdit folding signals */

//{{{ License
/*
*   This file is part of gmp3view.
*   (c) 2006 Malte Rohde
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

//{{{ Includes and defines
using namespace std;
#include <cstdlib>
#include <iostream>
#include <string>
#include "extern.h"
//}}}

//{{{ Prototypes
void enqueue_to_xmms_playlist(string path);
void play_with_xmms(string path);
void enqueue_to_audacious_playlist(string path);
void play_with_audacious(string path);
void view_with_feh(string path);
//}}}

//{{{ Main
/* This is basic configuration */
void player_play(string path)
{
	play_with_audacious(path);
}

void player_enqueue(string path)
{
	enqueue_to_audacious_playlist(path);
}

void viewer_show(string file)
{
	view_with_feh(file);	
}
//}}}

//{{{ xmms
void enqueue_to_xmms_playlist(string path)
{
/* path:	e.g. /home/user/MP3/lp/B*/
	string cmd("xmms -e \"");
	cmd += path;
	cmd += "\" &";
	system(cmd.c_str());
}

void play_with_xmms(string path)
{
/* path:	e.g. /home/user/MP3/lp/B*/
	string cmd("xmms -ep \"");
	cmd += path;
	cmd += "\" &";
	system(cmd.c_str());
}
//}}}

//{{{ audacious
void enqueue_to_audacious_playlist(string path)
{
/* path:	e.g. /home/user/MP3/lp/B*/
	string cmd("audacious -e \"");
	cmd += path;
	cmd += "\" &";
	system(cmd.c_str());
}

void play_with_audacious(string path)
{
/* path:	e.g. /home/user/MP3/lp/B*/
	string cmd("audacious -ep \"");
	cmd += path;
	cmd += "\" &";
	system(cmd.c_str());
}
//}}}

//{{{ feh
void view_with_feh(string path)
{
	string cmd("feh \"");
	cmd += path;
	cmd += "\" &";
	system(cmd.c_str());	
}
//}}}


