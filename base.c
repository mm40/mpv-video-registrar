//TODO : make sure all the memory allocated by various functions is dealocated later

#include "base.h"
#include "global.h"
#include <stdarg.h>

int isverbose = 0;
sqlite3 *sqconn = NULL;
char* url = NULL;
int ifIdExists = 0;

void insertComment(int vidID, char* comment, double time ) {
  verprintf("Inserting comment ...\n");
    setUpConn();
    sqlite3_stmt *res;
    const char* finalCommand = "insert into Komentari(movID, comment, time) values (?1 , ?2, ?3);";

    int error = sqlite3_prepare_v2(sqconn,finalCommand, -1, &res, NULL);
    sqlite3_bind_int(res,  1, vidID);
    sqlite3_bind_text(res, 2, strlen(comment) > 0 ? comment : NULL, -1, SQLITE_STATIC);
    sqlite3_bind_double(res,  3, time);

    if (sqlite3_step(res) != SQLITE_DONE) {
      errprintf("ERROR inserting data: %s\n", sqlite3_errmsg(sqconn));
      clexit(-1);
    }
    sqlite3_finalize(res);
}

void setUpConn() {
  if (sqconn) {
    //verprintf("INFO : DB connection already established.\n"); /* TODO: too frequent for playlists */
    return;
  }
  verprintf("Opening database ...\n");
  int error = sqlite3_open(PATH_SQL_DATA, &sqconn);
  if (error) {
    errprintf("ERROR : Database opening error !\n");
    clexit(-1);
  }
  error = sqlite3_exec(sqconn, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);
  if (error) {
    errprintf("ERROR : Could not set foreign_keys = ON %d\n", error);
  }
}

int getThemeIdPathFromTitle(char* themeName, char** themeID, char** themePath) {
  setUpConn();
  sqlite3_stmt *res;
  const char *tail;
  char *query = "select id, path from Teme where name = '"; // + title + "'"
  char *finalQuery = malloc(sizeof(char) * (strlen(query) + strlen(themeName) + 3));
  strcpy(finalQuery, query);
  strcat(finalQuery, themeName);
  strcat(finalQuery, "';");

  verprintf("SQL select query = %s\n", finalQuery);
  
  int error = sqlite3_prepare_v2(sqconn, finalQuery, sizeof(char) * (strlen(finalQuery) + 1), &res, &tail);
  if (error != SQLITE_OK) {
    errprintf("Error compiling SQL statement : '%s'\n", finalQuery);
    clexit(-1);
  }

  int rowcount = 0;
  //this should only return one row
  if(sqlite3_step(res) == SQLITE_ROW) {
    const char* id = sqlite3_column_text(res, 0);
    *themeID = malloc(sizeof(char) * (strlen(id) + 1));
    strcpy(*themeID, id);

    const char* path = sqlite3_column_text(res, 1);
    *themePath = malloc(sizeof(char) * (strlen(path) + 1));
    strcpy(*themePath, path);
    rowcount++;
  }
  return rowcount;
}

char* queriedData(char* query, cJSON* root) {
  cJSON* temp = cJSON_GetObjectItemCaseSensitive(root, query);
  char* value = temp ? temp->valuestring : NULL;

  char* result = NULL;
      if (value && strlen(value) > 0  && strcmp("null", value)) {
	result = malloc(sizeof(char)*(strlen(value) + 1));
	strcpy(result, value);
      }
      return result;
}

void populate(vidInfo* vid, char* JSON) {
  cJSON *json = cJSON_Parse(JSON);
  
  vid->title = queriedData("title", json);
  vid->fulltitle = queriedData("fulltitle", json);
  vid->upload_date = queriedData("upload_date", json);
  vid->ytid = queriedData("id", json);
  vid->uploader_url = queriedData("uploader_url", json);
  vid->display_id = queriedData("display_id", json);
  vid->description = queriedData("description", json);
  vid->uploader = queriedData("uploader", json);
  vid->uploader_id = queriedData("uploader_id", json);
  vid->subtitles = queriedData("subtitles", json);
  vid->automatic_captions = queriedData("automatic_captions", json);
  vid->thumbnail = queriedData("thumbnail", json);
  vid->webpage_url = queriedData("webpage_url", json);
  vid->next = NULL;
}

int insertVideoIntoDatabase(vidInfo* vid, char* themeID) {
  setUpConn();
  sqlite3_stmt *res;
  const char* finalCommand = "insert into Snimci(title, fulltitle, upload_date, ytid, uploader_url, display_id, description, uploader, uploader_id, subtitles, automatic_captions, thumbnail, webpage_url, odgledan, temaID) values (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15);";

  verprintf("Inserting video into database ..\n");
  printVideoProperties(vid);
  int error = sqlite3_prepare_v2(sqconn,finalCommand, -1, &res, NULL);

  sqlite3_bind_text(res, 1, vid->title, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 2, vid->fulltitle, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 3, vid->upload_date, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 4, vid->ytid, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 5, vid->uploader_url, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 6, vid->display_id, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 7, vid->description, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 8, vid->uploader, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 9, vid->uploader_id, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 10, vid->subtitles, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 11, vid->automatic_captions, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 12, vid->thumbnail, -1, SQLITE_STATIC);
  sqlite3_bind_text(res, 13, vid->webpage_url, -1, SQLITE_STATIC);
  sqlite3_bind_int(res,  14, 0);
  sqlite3_bind_text(res, 15, themeID, -1, SQLITE_STATIC);
  
  if (sqlite3_step(res) != SQLITE_DONE) {
    errprintf("ERROR inserting data: %s\n", sqlite3_errmsg(sqconn));
    //will output "UNIQUE constraint failed" if there already is row with same fields in the table, preventing adding duplicate. This is useful for checking fields which cannot be easily checked (without downloading JSON file), only by looking at the input link (like idFromReceivedLink function does).
    /* TODO:  allow comments to be added for videos that are discovered this way*/
    if (vid->next)
      errprintf("Alhough error occoured, continuing with other videos ...\n");
    else
      clexit(-1);
  }
  sqlite3_finalize(res);

  if (vid->next)
    insertVideoIntoDatabase(vid->next, themeID);
}

void printVideoProperties(vidInfo* vid) { 
  verprintf("\nPrinting video information : \n=========================\n");
  verprintf("title : %s\n", vid->title);
  verprintf("fulltitle : %s\n", vid->fulltitle);
  verprintf("upload_date : %s\n", vid->upload_date);
  verprintf("id : %s\n", vid->ytid);
  verprintf("uploader_url : %s\n", vid->uploader_url);
  verprintf("display_id : %s\n", vid->display_id);
  verprintf("description : %s\n", vid->description);
  verprintf("uploader : %s\n", vid->uploader);
  verprintf("uploader_id : %s\n", vid->uploader_id);
  verprintf("subtitles : %s\n", vid->subtitles);
  verprintf("automatic_captions : %s\n", vid->automatic_captions);
  verprintf("thumbnail : %s\n", vid->thumbnail);
  verprintf("webpage_url : %s\n", vid->webpage_url);
  verprintf("====================\n");
  //if (vid->next) stampaj(vid->next);
}

int idFromReceivedLink(char* url) { 
  setUpConn();
  
  //command for searching (61 characters)
  //select id, title from snimci where 'URL' like '%' || ytid || '%';
  
  sqlite3_stmt *res;
  /* char* finalCommand = malloc(sizeof(char)*(66 + strlen(url))); // 66 : length of command (65) + terminator */

  char* finalCommand = "select id from snimci where ?1 like '%' || ytid || '%';";

  /* verprintf("Selection command : %s\n", finalCommand); */
  
  int error = sqlite3_prepare_v2(sqconn,finalCommand, -1, &res, NULL);
  sqlite3_bind_text(res, 1, url, -1, SQLITE_STATIC);

  char *resID = NULL;
  
  if(sqlite3_step(res) == SQLITE_ROW) {
    const char* id = sqlite3_column_text(res, 0);
    resID = malloc(sizeof(char) * (strlen(id) + 1));
    strcpy(resID, id);
    // second field not needed
  }

  return resID ? atoi(resID) : 0;

  //https://stackoverflow.com/questions/338739/inverse-of-sql-like-value
  //https://stackoverflow.com/questions/472063/mysql-what-is-a-reverse-version-of-like
  //google : sql inverse of contains
}

vidInfo* openURL(char* url, int tempFileID) {
  verprintf("Opening url ", url, " ...\n");
  char command[MAX_COMMAND_LENGTH] = {'\0'};
  strcpy(command, COMM_YT);
  
  if (tempFileID > 0) {
    strcat(command, COMM_YT_NO_PLAYLIST_ADDITION);
  }

  if (!isverbose)
    strcat(command, COMM_YT_QUIET_ADDITION);

  strcat(command, " \"");
  strcat(command, url);
  strcat(command, "\"");

  if (tempFileID < 0) {
    strcat(command, COMM_JSON_LISTCHAN_STRIP);
  }
  verprintf("Command to be called : %s\n", command);

  FILE *fp;
  char path[1000];
  //char resJ[MAX_JSON] = {'\0'};
  char *resJ = malloc(sizeof(char) * MAX_JSON);
  resJ[0] = '\0';
  

  fp = popen(command, "r"); /* TODO:  enable reading of warnings and errors that youtube-downloader outputs*/
  if (fp == NULL) { /* TODO:  does this cover the situation when there are 10 clips downloaded in a playlist, and it's stopped at 5ht? If not, fix it !!*/
    errprintf("ERROR : Could not run command :\n=====\n'%s\n=====', exiting...", command);
    clexit(1);
  }
  while (fgets(path, sizeof(path), fp) != NULL) {
    strcat(resJ, path);
    //verprintf("%s", path);
  }

  pclose(fp);
  verprintf("Done with youtube-dl\n");

  //char* JSON = malloc(sizeof(char)*strlen(resJ) + 1);
  //strcpy(JSON, resJ);

  //  clexit(1);
  vidInfo* res = NULL;

  if (tempFileID > 0) { // only one video
     res = malloc(sizeof(vidInfo));     
     populate(res, resJ);
  }
  else { // more than one clip
    //need to search ,"separator":"__++__++__"} and replace with }
    //actually replace ," with }\0 and pass pointer to beggining. Next pointer will start at the end of }\0separator":"__++__++__"}, and so on ...
    const char* search = ",\"separator\":\"__++__++__\"}";
    char *occourence = strstr(resJ, search);
    char *ithJSON = resJ;
    vidInfo *last, *curr;
    int i = 0;
    while(occourence) {
      i++;
      occourence[0] = '}';
      occourence[1] = '\0';
      occourence += strlen(search) + 1;
      
      curr = malloc(sizeof(vidInfo));
      populate(curr, ithJSON);

      ithJSON = occourence;
      occourence = strstr(ithJSON, search);
      
      if (curr->ytid == NULL) { // all is null, video disabled or deleted...
	errprintf("ERROR : Video number %d cannot be evidented !\n", i+1);
	continue; /* TODO:  should also free(ithJSON) like a few lines below ...*/
      }
      else if(idFromReceivedLink(curr->webpage_url)) {
	verprintf("EXISTS Already, skipping it : Video url : %s , title : %s\n", curr->webpage_url, curr->title);
	continue;
      }
      
      if (!res)
	res = curr;
      else
	last->next = curr;

      
      last = curr;
      // free(ithJSON); /* TODO:  this causes error. WHY ? Fix it  */
    }
  }
    
  return res;
}

void printComments(int videoID) {
  //select comment, time from komentari where movID = videoID;
  setUpConn();
  sqlite3_stmt *res;
  const char *tail;
  char *query = "select comment, time from komentari where movID = ?1 order by time;"; // + title + "'"
  int error = sqlite3_prepare_v2(sqconn, query, -1, &res, NULL);
  sqlite3_bind_int(res, 1, videoID);
  verprintf("SQL select query = %s\n", query);

  if (error != SQLITE_OK) {
    errprintf("Error compiling SQL statement : '%s'\n", query);
    clexit(-1);
  }

  //this should only return one row
  while (sqlite3_step(res) == SQLITE_ROW) {
    //const char* id = sqlite3_column_text(res, 0);
    printf("%s\n%s\n", sqlite3_column_text(res, 0), sqlite3_column_text(res, 1));
    //const char* path = sqlite3_column_text(res, 1);
  }
}

int sqlUpdateCommand(const char* command, int argn, ...) { // TODO : where ever database is updated (not query), elegantly call this function like it's called from deleteVideo
  setUpConn();
  
  va_list list;
  int i;
  
  va_start(list,argn);  
  sqlite3_stmt *res;
  int error = sqlite3_prepare_v2(sqconn,command, -1, &res, NULL);
  if (error) return error;
  
  for (i = 0; i < argn/2; i++) {
    int type = va_arg(list, int);
    
    switch (type) {
    case SQLITE_INTEGER: sqlite3_bind_int(res, i+1, va_arg(list, int)); break;
    case SQLITE_FLOAT: sqlite3_bind_double(res, i+1, va_arg(list, double)); break;
    case SQLITE_TEXT: sqlite3_bind_text(res, i+1, va_arg(list, char*), -1, SQLITE_STATIC); break;
 
    default: errprintf("Unsupported data type for sqlUpdateCommand !"); return -1;
    }         
  }

  if (sqlite3_step(res) != SQLITE_DONE) {
    errprintf("ERROR deleting data: %s\n", sqlite3_errmsg(sqconn));
    clexit(-1);
  }
  error = sqlite3_finalize(res);
  va_end(list);
  return error;
}

int deleteVideo(int vidID) {
  verprintf("Deleting video from the database ...\n");
  int error;
  if (error = sqlUpdateCommand("delete from Komentari where movID = ?1;",2, SQLITE_INTEGER, vidID)) return error;
  if (error = sqlUpdateCommand("delete from Snimci where id = ?1;",2, SQLITE_INTEGER, vidID)) return error;
  verprintf("Videos deleted sucessfully !\n");
  return 0;
}

int deleteCommentAt(int vidID, double time) {
  verprintf("Deleting comment from the database ...\n");
  int error;
  if (error = sqlUpdateCommand("delete from Komentari where movID = ?1 and time = ?2", 4, SQLITE_INTEGER, vidID, SQLITE_FLOAT, time)) return error;
  verprintf("Comments deleted successfully !\n");
  return 0;
}
