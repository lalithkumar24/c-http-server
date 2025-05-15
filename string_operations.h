#pragma once
#include <stdbool.h>
#include <stddef.h>

#define STRING_FROM_CSTR(str)                                                  \
  (string) { .data = str, .len = sizeof(str) }

typedef struct {
  const char *data;
  size_t len;
} string;

typedef struct {
  string *splits;
  size_t count;
  size_t capacity;
} string_splits;

bool string_equal(string *a, string *b);

string convert_cstr_string(const char *str);

void string_trim_spaces(string *s);

string_splits split_string(const char *str, size_t len, const char *split_by);

void free_string_splits(string_splits *splits);
