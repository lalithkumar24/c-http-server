#include "string_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool string_equal(string *a, string *b) {
  return a->len == b->len && memcmp(a->data, b->data, a->len) == 0;
}

string convert_cstr_string(const char *str) {
  string s;
  s.len = strlen(str);
  s.data = str;
  return s;
}

void string_trim_spaces(string *s) {
  while (*s->data == ' ') {
    s->data += 1;
  }
  while (s->len > 0 && s->data[s->len - 1] == ' ') {
    s->len -= 1;
  }
}

string_splits split_string(const char *str, size_t len, const char *split_by) {
  string_splits result;
  const char *start = str;
  size_t result_i = 0;
  size_t split_by_len = strlen(split_by);

  result.capacity = 8;
  result.splits =(string_view*) calloc(sizeof(string_view), result.capacity);
  result.count = 0;
  for (size_t i = 0; i < len; ++i) {
    if (i + split_by_len < len &&
        memcmp(&str[i], split_by, split_by_len) == 0) {
      result.splits[result_i].start = start;
      result.splits[result_i].len = &str[i] - start;
      result.count += 1;
      result_i += 1;
      start = &str[i + split_by_len];
      i += split_by_len;
      if (result.count == result.capacity) {
        result.capacity *= 2;
        string_view *temp = (string_view *)realloc(
            result.splits, sizeof(string_view) * result.capacity);
        if (temp) {
          result.splits = temp;
        } else {
          perror("realloc()");
          abort();
        }
      }
    }
  }
  size_t last_len = &str[len] - start;
  if (last_len > 0) {
    result.splits[result_i].start = start;
    result.splits[result_i].len = last_len;
    result.count += 1;
  }
  return result;
}

void free_string_splits(string_splits *splits) {
  if (splits) {
    free(splits->splits);
    splits->splits = NULL;
  }
  splits->capacity = 0;
  splits->count = 0;
}
