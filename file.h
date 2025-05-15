#pragma once

#include "string_operations.h"
#include <linux/limits.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct {
  bool exists;
} file_metadata;

file_metadata file_status(string filename);
