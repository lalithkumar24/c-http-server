#pragma once

#include "string_operations.h"
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>

typedef struct {
  bool exists;
  size_t size;
} file_metadata;

file_metadata file_status(string filename);
