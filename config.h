#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CFG_PREFIX "/.gmp3view/"								/* Will always be prefixed by "$HOME" */
#define DATABASE "medialib.db"									/* No reason to change anything here */
#define QUERIES "query.lst"										/* This file (inside CFG_PREFIX) keeps the listed sql queries */
#define STYLESHEET "style.css"									/* this file keeps additionals css styles to apply to the widgets */
#define MP3_IMAGE "/usr/local/share/gmp3view/mp3.png"			/* This file is shown as a mp3 file's icon */
#define NUM_TAGS 12												/* Adjust to read more tags (set to <=0 for all tags)*/
#define THUMB_SIZE 160											/* thumbnails are scaled to THUMBSIZE*THUMBSIZE */

#define LASTFM_KEY "f263e056c82addecb074209a1fa97977"

#define VERSION "0.4.2alpha1"

#endif /* __CONFIG_H__ */
