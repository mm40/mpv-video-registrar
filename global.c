#include "global.h"

void logging(char* logMessage) {
  FILE* f = fopen(PATH_LOG, "a");
  if (!f) return;
  //fprintf(f, "[URL] : %s [MSG] : %s\n", url, logMessage);
  fprintf(f, "%s", logMessage);
  fclose(f);
}
