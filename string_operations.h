#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Structure representing a string view
 */
typedef struct {
  const char *data;
  size_t len;
} string;

/**
 * Structure representing a view into a portion of a string
 */
typedef struct {
  const char *start;
  size_t len;
} string_view;

/**
 * Structure representing a collection of string splits
 */
typedef struct {
  string_view *splits;
  size_t count;
  size_t capacity;
} string_splits;

/**
 * Compare two strings for equality
 *
 * @param a First string
 * @param b Second string
 * @return true if strings are equal, false otherwise
 */
bool string_equal(string *a, string *b);

/**
 * Convert a C-style string to our string structure
 *
 * @param str C-style string
 * @return Equivalent string structure
 */
string convert_cstr_string(const char *str);

/**
 * Trim spaces from the beginning and end of a string
 *
 * @param s String to trim
 */
void string_trim_spaces(string *s);

/**
 * Split a string by a delimiter
 *
 * @param str String to split
 * @param len Length of string
 * @param split_by Delimiter to split by
 * @return Collection of string splits
 */
string_splits split_string(const char *str, size_t len, const char *split_by);

/**
 * Free memory allocated for string splits
 *
 * @param splits String splits to free
 */
void free_string_splits(string_splits *splits);

#endif /* STRING_OPERATIONS_H */
