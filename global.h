#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define ARG_THEME "--theme"
#define ARG_COMMENT "--comment"
#define ARG_GET_COMMENTS "-g"
#define ARG_COMMTIME "--time"
#define ARG_SOMETHING "-j"
#define ARG_VERBOSE "-v"
#define ARG_EXISTANCE "-e"

// for entire channels and entire playlists
#define ARG_MORE_VIDEOS "-m"

// remove video from evidence if exists, along with its comments
#define ARG_REMOVE_FROM_EVIDENCE "-r"

#define ARG_REMOVE_COMMENT "--remove-comment"

/* TODO: BUG : what if the video is the 3rd on the playlist, and not the first? This command will still return the first.
/* TODO:  add --sub-lang en  --write-auto-sub */
//#define COMM_YT "yt-dlp  --skip-download --write-info-json " //command to be called, with vide URL at the end of it.
#define COMM_YT "yt-dlp -i  --dump-single-json  --skip-download "

#define COMM_YT_NO_PLAYLIST_ADDITION " --no-playlist "

//#define COMM_YT_OUTPUT_ADDITION " -o "

#define COMM_YT_QUIET_ADDITION " -q "

#define COMM_JQ "jq -r -c "

// yt-dlp adds this to the file name supplied with -o, at the end
#define PATH_YT_JSON_ADDITION ".info.json" 

#define PATH_SQL_DATA "./video-registry.db"

// maximum length of all JSON fields for single video
#define MAX_JSON_SINGLE 15000

#define PATH_LOG "./mpv-video-registrar-log.txt"

/* TODO:  make it so that not for every field jq is called, but only once, and have this progrum cut necessary fields. */
#define MAX_COMMAND_LENGTH 5000

// for channel of ! 1700 videos, 51MB of JSON was needed !
#define MAX_JSON 10*1024*1024

/* TODO:  this is 10MB of text, however, if playlist is 300 videos for instance, this could be small, since 1 video is ~ 35kb.
   if LISTCHAN_STRIP is used, than ~1700 videos is around ~1.2MB. TODO : Anyway, Wherever this constant is used, there should be dynamic reallocation of size */
// This JSON addition has to be used when entire playlist or channel is being loaded. JSON for playlist can be
// As big as 500kb (for playlist of ~50 items), and this big JSON can not be (for some reason) piped in the queriedData function from "echo" comand to "jq" command.
// popen function in openURL can handle piping of such large data, so very large JSON is piped right when received from yt-dlp with COMM_JSON_LISTCHAN_STRIP to
// remove all unnecessary fields, and only leave the fields that are needed for each video. Sample of this JSON is on bottom of the file
//#define COMM_JSON_LISTCHAN_STRIP " | jq  '.|{extractor: .extractor, _type: ._type, title: .title, extractor_key: .extractor_key, webpage_url: .webpage_url, entries: [.entries[]| {n_entries: .n_entries, title: .title, fulltitle: .fulltitle, upload_date: .upload_date, id: .id, uploader_url: .uploader_url, display_id: .display_id, description: .description, uploader: .uploader, uploader_id: .uploader_id, subtitles: .subtitles, automatic_captions: .automatic_captions, thumbnail: .thumbnail, webpage_url: .webpage_url}]}'"
#define COMM_JSON_LISTCHAN_STRIP "| jq  -c '.entries[]| {title: .title, fulltitle: .fulltitle, upload_date: .upload_date, id: .id, uploader_url: .uploader_url, display_id: .display_id, description: .description, uploader: .uploader, uploader_id: .uploader_id, subtitles: .subtitles, automatic_captions: .automatic_captions, thumbnail: .thumbnail, webpage_url: .webpage_url, separator: \"__++__++__\"}'"
/* TODO:  add JSON strip for single videos aswell, because queryData will work faster when there is less JSON*/

// Separator explained : instead of passing the result of piping trough COMM_JSON_LISTCHAN_STRIP to further JSON querrying trough jq,
// i put the separator into the result. I will use this separator to iterate trough JSON query here with C, and pass each video,
// one at time, to process trough jq. I do this because JSON querry can be as big as 1.2MB (for ~1700 video playlist),
// and maximum JSOM size that command queriedData can receive is 128kb (on my system).
// That is why i will use separator to querry one video at a time with queryData.

#define verprintf(...) do { if (isverbose) printf( __VA_ARGS__); } while(0)
#define errprintf(...) printf(__VA_ARGS__)

#define clexit(...) do { \
  if (sqconn) sqlite3_close(sqconn); \
  if (themeID) free(themeID); \
  if (themePath) free(themePath); \
  logging("Could not process link");     \
exit(__VA_ARGS__); } while(0) 

#include <stdio.h>

void logging(char*);


#endif
