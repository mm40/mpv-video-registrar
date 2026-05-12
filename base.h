#ifndef _BASE_H_
#define _BASE_H_

#include <sqlite3.h> // sudo apt-get install libsqlite3-dev , to compile in gcc, add -lsqlite3
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

extern sqlite3 *sqconn; // connection to local database
extern int isverbose; // whether the program outputs various information or not
extern char* themeID; // the id of the theme of the video
extern char* themePath; // the path of the selected theme of the video
extern int ifIdExists; 
extern char* url;

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

// just needs to be declared
FILE* popen(char*, char*);

// just needs to be declared
void pclose(FILE*);

// just needs to be declared
int waitpid(int, int*, int);

// only prints comments for the video, used woth -g flag
void printComments(int vidID);

// deletes video from list, along with its comments. Returns 0 for success,
// error code otherwise.
int deleteVideo(int vidID);

// runs SQL update command with variable parameters : first parameter is the
// command itself, second - number of the following parameters, and after
// that pairs of type-value, for instance :
// ("command",6, int, 5, string, "test", float, 1.5).
// Types are SQLITE_(INTEGER/FLOAT/TEXT).
// Arguments in command are from 1 onward : ?1, ?2, ... etc.
// Returns 0 if succeded, error code if failed
int sqlUpdateCommand(const char* command, int argn, ...);

// deletes comment for vid id, at the specified time.
// Returns 0 for success, error for otherwise
int deleteCommentAt(int vidID, double time); 

#endif
