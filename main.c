#include <time.h>
#include <unistd.h> // for access() function

#include "global.h"
#include "base.h"
#include "cJSON.h"

char* themeID;
char* themePath;

int main(int argc, char*argv[]) {
  int i = 0;
  srand(time(NULL));
  int tempFileID = rand();
  char* theme = NULL;
  int existanceOnly = 0;
  int removeOnly = 0;
  int commentRemoveOnly = 0;
  int getCommentsOnly = 0;
  
  themeID = NULL;
  themePath = NULL;
  char* comment = NULL;
  double commTime = -1;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] != '-') { // something
      url = argv[i];
      verprintf("Video URL set to '%s'\n", url);
    }
    else if (argv[i][1] != '-') { // -something
      char* flag = argv[i];
      if (!strcmp(ARG_VERBOSE, flag)) {
	verprintf("Verbose set to ON ... \n");
	isverbose = 1;
      }
      else if (!strcmp(ARG_MORE_VIDEOS, flag)) {
	verprintf("Multiple files will be loaded ... \n");
	tempFileID = -tempFileID;
      }
      else if (!strcmp(ARG_GET_COMMENTS, flag)) {
	verprintf("Only getting comments ... \n");
	getCommentsOnly = 1;
      }
      else if (!strcmp(ARG_EXISTANCE, flag)) {
	if (tempFileID < 0) {
	    errprintf("ERROR : Cant check existance for multiple links ! Either remove %s or %s. Exiting ...", ARG_EXISTANCE, ARG_MORE_VIDEOS);
	    clexit(-1);
	  }
	verprintf("Only existance of file should be checked ... \n");
	existanceOnly = 1;
      }
      else if (!strcmp(ARG_REMOVE_FROM_EVIDENCE, flag)) {
	removeOnly = 1;
      }
      else { // unknown flag
	verprintf("Unknown flag : %s\n", flag); 
      }
    }
    else { // --something
      char* flag = argv[i++];
      char* parameter = argv[i];
      if (!strcmp(ARG_THEME, flag)) {
	theme = parameter;
	verprintf("Video theme set to '%s'\n", parameter);
      }
      else if (!strcmp(ARG_COMMENT, flag)) {
	comment = parameter;
	verprintf("Video comment set to '%s'\n", parameter);
      }
      else if (!strcmp(ARG_COMMTIME, flag)) {
	sscanf(parameter, "%lf", &commTime);
	verprintf("Video comment time set to '%d'\n", commTime);
      }
      else if (!strcmp(ARG_REMOVE_COMMENT, flag)) {
	verprintf("Comment removal enabled\n");
	commentRemoveOnly = 1;
      }
      else { // unknown flag
	verprintf("Unknown flag : %s\n", flag); 
      }
    }
  }

  if (comment) {
    if (commTime < 0) {
      errprintf("Error : --time not specified for a comment ! Exiting ... \n"); // this happens when a playlist or channel is passed, but all videos from channel/playlist are either already evidented, or are unavailable, or some DB error occoured ...
      clexit(-1);
    }
  }
  if (tempFileID > 0 || existanceOnly) // only if single link is given
    ifIdExists = idFromReceivedLink(url);
  if (ifIdExists) {
    if (existanceOnly) { // only checking existance, and it does exist
      printf("Existant");
      clexit(0);
    }
    if (getCommentsOnly) {
      printComments(ifIdExists);
      clexit(0);
    }
    if (removeOnly) {
      if (!deleteVideo(ifIdExists))
	printf("Success!\n");  
      clexit(0);
    }
    if (commentRemoveOnly) {
      if (commTime < 0) {
	errprintf("Comment removal enabled, but comment time not specified. Exiting ...\n");
	clexit(-1);
      }
      if (!deleteCommentAt(ifIdExists, commTime))
	printf("Success!\n");  
      clexit(0);
    }
    verprintf("Video already in database, ID : %s\n");//, ifIdExists); TODO : this int ifIDexists here needs to be converted to Integer, to be printed
    if (!comment) {
      errprintf("Video already exists, but no comment added. Exiting ...\n");
      clexit(-1);
    }
  }
  else {
    if (existanceOnly) { // only checking existance, and it does not exist
      printf("NonExistant");
      clexit(0);
    }
    if (getCommentsOnly) {
      verprintf("Selected comments only, but video does not exist. Exiting ...\n");
      clexit(0);
    }
    if (removeOnly || commentRemoveOnly) {
      verprintf("Selected for removal, but video is not in database anyway ...\n");
      clexit(0);
    }
    verprintf("Video not already in database, proceeding with adding it ...\n");
    if (theme) {
      int rows = getThemeIdPathFromTitle(theme, &themeID, &themePath);
      if (rows == 1)
	verprintf("Loaded themeID and Path : %s, %s\n", themeID, themePath);
      else {
	errprintf("ERROR : Found %d rows with theme %s, expected only one row. Exiting ...\n", rows, theme);
	clexit(-1);
      }
    }
    else {
      errprintf("ERROR : --theme parameter not specified !\n");
      clexit(-1);
    }
  
    // not already in database, downloading JSON :
    verprintf("Random number : %d\n", tempFileID);
    vidInfo* video = openURL(url, tempFileID);
    if (video == NULL) {
      errprintf("ERROR : Someehow no videos can be added, exiting ...\n");
      clexit(-1);
    }
    verprintf("Populated : \n");
    //clexit(-1);
    insertVideoIntoDatabase(video, themeID);

    ifIdExists = idFromReceivedLink(url);
    // After this line, video certainly exists, and can be commented on.
  }

  if (comment) insertComment(ifIdExists, comment, commTime);
  
  // if (video) free(video); // individual fields of video-> ... should be also freed

  printf("Success!\n");  
  
}

//json should be deallocated separately from this function
