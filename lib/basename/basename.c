#include "basename.h"

//basname implementation copied shamelessly from https://github.com/libsdl-org/SDL_ttf/pull/289
//will become unnessecary when https://github.com/libsdl-org/SDL/issues/7915 is closed

char* SDL_basename(const char *path) {
  static char buffer[1024];
  const char *pos, *sep, *prev_sep;
  int prev_was_sep = 0;

  prev_sep = sep = NULL;
  for (pos = path; *pos; pos++) {
    if (*pos == '/' || *pos == '\\') {
      if (!prev_was_sep) {
          prev_sep = sep;
      }
      sep = pos;
      prev_was_sep = 1;
    } else {
      prev_was_sep = 0;
    }
  }
  if (!sep) {
    if (path[0] == '\0') {
      buffer[0] = '.';
      buffer[1] = '\0';
    } else {
      SDL_strlcpy(buffer, path, sizeof(buffer));
    }
  } else {
    if (sep[1] == '\0') {
      if (prev_sep) {
        char *s;
        SDL_strlcpy(buffer, prev_sep + 1, sizeof(buffer));
        SDL_strtok_r(buffer, "/\\", &s);
      } else {
        buffer[0] = *sep;
        buffer[1] = '\0';
      }
    } else {
      SDL_strlcpy(buffer, sep + 1, sizeof(buffer));
    }
  }
  return buffer;
}
