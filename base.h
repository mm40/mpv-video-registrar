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

#endif
