#include "file.h"
#include "string_operations.h"
#include <linux/limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

file_metadata file_status(string filename) {
  char buff[PATH_MAX];
  struct stat st;
  file_metadata metadate = {false};
  if (filename.len + 1 > PATH_MAX) {
    return metadate;
  }
  memset(&buff, 0, sizeof(buff));
  memcpy(&buff, filename.data, filename.len);
  if (stat(buff, &st) < 0) {
    return metadate;
  }
  metadate.exists = true;
  return metadate;
}
