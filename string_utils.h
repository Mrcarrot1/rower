/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef CASH_STRING_UTILS_H
#define CASH_STRING_UTILS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SB_DEFAULT_SIZE 32

//Get the number of bytes required to store the given string.
#define STR_REQUIRED_BYTES(string) ((strlen(string) + 1) * sizeof(char))

//Get the number of bytes required to store the result of concatenating two strings.
#define STR_CONCAT_REQUIRED_BYTES(str1, str2) ((strlen(str1) + strlen(str2) + 1) * sizeof(char))

/*
 Structure that stores a string and the capacity of the buffer.
 Should be created and modified only with the sb_* functions.
 NOTE: must be freed with the sb_free function when no longer needed.
 */
typedef struct stringBuilder
{
    size_t capacity;
    char *contents;
} stringBuilder;

#define SB_EMPTY ((stringBuilder) { .capacity = 0, .contents = "" })

/*
 Sets the contents of the specified stringBuilder to the given string.
 */
void sb_set_contents(stringBuilder *sb, const char *contents);

/*
 Appends the contents of the given string to the stringBuilder.
 */
void sb_append_contents(stringBuilder *sb, const char *contents);

/*
 Appends a single character to the end of the stringBuilder's contents.
 */
void sb_append_char(stringBuilder *sb, char c);

/*
 Allocates a new stringBuilder with the specified capacity.
 */
stringBuilder sb_new(size_t initialCapacity);

#define sb_new_with_default_size() sb_new(SB_DEFAULT_SIZE)

/*
 Allocates a new stringBuilder with the specified contents.
 */
stringBuilder sb_new_with_contents(const char *contents);

/*
 Allocates a new stringBuilder using memory from the specified allocator.
 */

/*
 Frees the memory associated with the provided stringBuilder.

 NOTE: This function is only guaranteed to work if the stringBuilder was created and modified
 exclusively with the sb_* family of functions provided by this header. If the contents have
 been set manually to a pointer that was not heap-allocated, it will fail.
 */
void sb_free(stringBuilder *sb);

/*
 Gets the length of the string in the stringBuilder.
 (Wrapper around strlen for stringBuilder)
 */
#define sb_len(_sb) strlen((_sb).contents)
//size_t sb_len(stringBuilder sb);

/*
 Immutable heap-allocated string that stores its length.
 */
typedef struct stringView
{
    size_t length;
    const char *contents;
} stringView;

/*
 Creates a new stringView with the specified contents.
 */
stringView sv_new(const char *contents);

/*
 Frees the memory associated with the specified stringView.
 */
void sv_free(stringView *sv);

/*
 Creates a new stringView from the specified stringBuilder's contents.
 */
#define sv_new_from_sb(_sb) sv_new((_sb).contents)

/*
 Copies len characters from the string into the output stringBuilder.
 */
stringBuilder substr(const char *str, size_t start, size_t len);

/*
 Gets the last char out of the provided buffer.
 */
char get_last_char(const char *s);

/*
 Reads a line from stdin to a stringBuilder, which is created at the provided output address.

 Returns: 0 if EOF was reached, 1 otherwise.
 */
int safe_readline(stringBuilder *output);

/*
 Reads a line from the provided stream to a stringBuilder, which is created at the provided output address.

 Returns: 0 if EOF was reached, 1 otherwise.
 */
int fsafe_readline(FILE *stream, stringBuilder *output);

/*
 Determines whether the provided string is empty or composed entirely of whitespace characters.
 */
bool is_space_or_empty(const char *s);

/*
 Removes any whitespace characters from the beginning of the string.
 */
void trim_start(char *s);

/*
 Removes any whitespace characters from the end of the string.
 */
void trim_end(char *s);

/*
 Removes any whitespace characters from the beginning and end of the string.
 */
void trim(char *s);

//atomicStrList, like the rest of this file, was borrowed from cash, which has a use for it and can provide it in most environments.
//This project has less need for it.
#if FALSE

/*
 Thread- and memory-safe dynamic string list implementation(use with atm_str_list_* functions)
 */
typedef struct atomicStrList
{
    mtx_t mutex;
    size_t count;
    size_t capacity;
    char **contents;
} atomicStrList;

/*
 Creates a new atomicStringList with the specified initial capacity and no contents.
 */
atomicStrList atm_str_list_new(size_t capacity);

/*
 Copies the string from the specified index in the list into the buffer, ending with a null terminator and not overflowing the specified size.
 */
void atm_str_list_get(atomicStrList *list, size_t index, char *buffer, size_t bufferSize);

/*
 Copies the string from the specified index in the list into a stringBuilder and returns it.
 Note that the stringBuilder will still need to be freed(sb_free) when done.
 */
stringBuilder atm_str_list_get_sb(atomicStrList *list, size_t index);

/*
 Adds the specified string to the list.
 */
void atm_str_list_add(atomicStrList *list, const char *str);

/*
 Sets the entry at the specified list index to the given string.
 */
void atm_str_list_set(atomicStrList *list, size_t index, const char *str);

/*
 Removes an entry at the specified index from the list.
 */
void atm_str_list_remove(atomicStrList *list, size_t index);

/*
 Destroys the list and frees all heap-allocated memory associated with it.
 */
void atm_str_list_free(atomicStrList *list);

/*
 Prints all items in the list.
 */
void atm_str_list_print(atomicStrList *list);

/*
 Checks if the list is a valid atomicStrList. Returns false if the list has been freed.
 */
bool atm_str_list_valid(atomicStrList *list);

#endif //FALSE

#endif //CASH_STRING_UTILS_H