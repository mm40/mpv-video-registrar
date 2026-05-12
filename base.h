#ifndef _BASE_H_
#define _BASE_H_

#include <sqlite3.h> // sudo apt-get install libsqlite3-dev , to compile in gcc, add -lsqlite3
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

sqlite3 *sqconn; // connection to local database
int isverbose; // whether the program outputs various information or not
char* themeID; // the id of the theme of the video
char* themePath; // the path of the selected theme of the video
int ifIdExists; 
char* url;

typedef struct VIDINFO {
  // order of these fields should be exactly as order in table Videos
  // data actualy saved in tables
  char* title;
  char* fulltitle; //
  char* upload_date;
  char* ytid;
  char* uploader_url;
  char* display_id;
  char* description;
  char* uploader;
  char* uploader_id;
  char* subtitles;
  char* automatic_captions;
  char* thumbnail;
  char* webpage_url;

  struct VIDINFO* next; // pointer to the next video
} vidInfo;

 // inserts comment for video vidID, with the text of comment, at the time time
void insertComment(int vidID,char* comment,double time);

// opens the connection to sqconn declared above
void setUpConn();

// From given tableName, fills themeID and themePath, if the row(s) with
// themeName exists in table. Returns number of such rows, should be only 1
// if 0 = none found; if > 1, more than one found, which should never happen.
int getThemeIdPathFromTitle(char* themeName, char** themeID, char** themePath);

char* queriedData(char* , cJSON*);

void populate(vidInfo* , char*);

int insertVideoIntoDatabase(vidInfo* ,char*);

// prints fields of vid structure
void printVideoProperties(vidInfo* vid);

// if video at url exists in database, returns its database id.
// It performs inverse CONTAINS function : checks whether any of listed ID's
// in database are contained in passed link, which can be either ordinary
// youtube link, or tv link, or mobile link, or playlist link ...
int idFromReceivedLink(char* url);

// opens the URL, writes the JSON to temp file ID (because JSON can be really
// big, even in megabytes sometimes for large playlists), than from that file
// it populates vidInfo fields, creating linked list if there are more videos
vidInfo* openURL(char* URL, int tempFileID);


#endif
