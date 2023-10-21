/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>
#include "string_utils.h"

void sb_set_contents(stringBuilder *sb, const char *contents)
{
    if (STR_REQUIRED_BYTES(contents) > sb->capacity)
    {
        free(sb->contents);
        sb->contents = calloc(STR_REQUIRED_BYTES(contents), 1);
    }
    strcpy(sb->contents, contents);
}

void sb_append_contents(stringBuilder *sb, const char *contents)
{
    if (STR_CONCAT_REQUIRED_BYTES(sb->contents, contents) > sb->capacity)
    {
        char *newBuffer = calloc(STR_CONCAT_REQUIRED_BYTES(sb->contents, contents), 1);
        sb->capacity = STR_CONCAT_REQUIRED_BYTES(sb->contents, contents);
        strcpy(newBuffer, sb->contents);
        free(sb->contents);
        sb->contents = newBuffer;
    }
    strcpy(sb->contents + strlen(sb->contents), contents);
}

void sb_append_char(stringBuilder *sb, char c)
{
    if (STR_REQUIRED_BYTES(sb->contents) + sizeof(char) > sb->capacity)
    {
        char *newBuffer = calloc(STR_REQUIRED_BYTES(sb->contents) + sizeof(char), 1);
        sb->capacity = STR_REQUIRED_BYTES(sb->contents) + sizeof(char);
        strcpy(newBuffer, sb->contents);
        free(sb->contents);
        sb->contents = newBuffer;
    }
    size_t position = sb_len(*sb);
    sb->contents[position] = c;
    sb->contents[position + 1] = '\0';
}

stringBuilder sb_new(size_t initialCapacity)
{
    stringBuilder sb;
    sb.capacity = initialCapacity;
    sb.contents = calloc(initialCapacity, sizeof(char));
    return sb;
}

stringBuilder sb_new_with_contents(const char *contents)
{
    stringBuilder sb;
    sb.capacity = STR_REQUIRED_BYTES(contents);
    sb.contents = calloc(sb.capacity, 1);
    strcpy(sb.contents, contents);
    return sb;
}

void sb_free(stringBuilder *sb)
{
    if (sb->capacity == 0) return; //The capacity should equal zero if and only if it has already been freed
    sb->capacity = 0;
    free(sb->contents);
    sb->contents = ""; //This should prevent a segfault if for some reason we try to read the contents later-- it will just show up as nothing instead
}

stringBuilder substr(const char *str, size_t start, size_t len)
{
    stringBuilder output = sb_new(len + 1);

#ifdef alloca   //alloca is not a standard C feature, so we fall back to one that is if we don't have it
                //If we do, though, it's preferable to use that because it's faster and is freed as soon as the function returns
    char *sub = alloca((len + 1) * sizeof(char));
#else
    char *sub = calloc(len + 1, sizeof(char));
#endif

    strncpy(sub, str + start, len);
    sb_set_contents(&output, sub);

#ifndef alloca
    free(sub);
#endif

    return output;
}

//Old sb_len implementation from before I made it a macro
/*size_t sb_len(stringBuilder sb)
{
    return strlen(sb.contents);
}*/

char get_last_char(const char *s)
{
    size_t i;
    for (i = 0; s[i] != '\0'; i++);
    return s[i > 0 ? i - 1 : i];
}

int safe_readline(stringBuilder *output)
{
    return fsafe_readline(stdin, output);
}

int fsafe_readline(FILE *stream, stringBuilder *output)
{
    *output = sb_new(32);
    char buf[32];
    fgets(buf, 32, stream);
    sb_append_contents(output, buf);
    while (get_last_char(buf) != '\n' && get_last_char(buf) != '\0')
    {
        if (!fgets(buf, 32, stream)) return 0;
        sb_append_contents(output, buf);
    }
    return 1;
}

bool is_space_or_empty(const char *s)
{
    if (s == nullptr) return true;
    size_t len = strlen(s);
    if (len == 0) return true;
    bool result = true;
    for (size_t i = 0; i < len; i++)
    {
        if (!isspace(s[i])) result = false;
    }
    return result;
}

void trim_start(char *s)
{
    if (s == nullptr) return;
    size_t whitespaceChars;
    for (whitespaceChars = 0; isspace(s[whitespaceChars]) /*|| s[whitespaceChars] == 3 || *((unsigned char *)s + whitespaceChars) == 0xe8*/; whitespaceChars++);
    memmove(s, s + whitespaceChars, strlen(s) - whitespaceChars + 1);
}

void trim_end(char *s)
{
    if (s == nullptr) return;
    for (size_t i = strlen(s) - 1; isspace(s[i]) || s[i] == 3 || *((unsigned char *)s + i) == 0xe8; i--)
    {
        s[i] = '\0';
    }
}

void trim(char *s)
{
    if (s == nullptr) return;
    trim_start(s);
    trim_end(s);
}

stringView sv_new(const char *contents)
{
    if (strlen(contents) == 0) return (stringView) { .length = 0, .contents = "" };
    size_t bufferSize = STR_REQUIRED_BYTES(contents);
    char *contentBuf = calloc(bufferSize + 256, 1);
    memcpy(contentBuf, contents, bufferSize);
    return (stringView) { .length = bufferSize - 1, .contents = contentBuf };
}

void sv_free(stringView *sv)
{
    if (sv->length == 0) return;
    free((void*)sv->contents);
    sv->length = 0;
    sv->contents = "";
}


//atomicStrList(see string_utils.h for more information)
#if FALSE

atomicStrList atm_str_list_new(size_t capacity)
{
    atomicStrList output;
    mtx_init(&output.mutex, mtx_plain);
    output.capacity = capacity;
    output.count = 0;
    output.contents = calloc(capacity, sizeof(char *));
    return output;
}

void atm_str_list_get(atomicStrList *list, size_t index, char *buffer, size_t bufferSize)
{
    assert(list->capacity != SIZE_MAX && "atm_str_list_get called after list was freed");
    mtx_lock(&list->mutex);
    const char *output = list->contents[index];
    strncpy(buffer, output, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
    mtx_unlock(&list->mutex);
}

stringBuilder atm_str_list_get_sb(atomicStrList *list, size_t index)
{
    assert(list->capacity != SIZE_MAX && "atm_str_list_get_sb called after list was freed");
    mtx_lock(&list->mutex);
    stringBuilder output = sb_new(strlen(list->contents[index]));
    sb_set_contents(&output, list->contents[index]);
    mtx_unlock(&list->mutex);
    return output;
}

#define resize_buf(buf, oldSize, newSize) \
    void *newBuf = calloc(newSize, 1);    \
    memcpy(newBuf, buf, oldSize);         \
    free(buf);                            \
    (buf) = (typeof(buf))(newBuf)

void atm_str_list_add(atomicStrList *list, const char *str)
{
    assert(list->capacity != SIZE_MAX && "atm_str_list_add called after list was freed");
    mtx_lock(&list->mutex);
    list->count++;
    if (list->capacity == list->count)
    {
        assert(SIZE_MAX - list->capacity > 16 && "Exceeded capacity of list");
        resize_buf(list->contents, list->capacity, list->capacity + 16);
    }
    list->contents[list->count - 1] = calloc(STR_REQUIRED_BYTES(str), 1);
    strcpy(list->contents[list->count - 1], str);
    mtx_unlock(&list->mutex);
}

void atm_str_list_set(atomicStrList *list, size_t index, const char *str)
{
    assert(list->capacity != SIZE_MAX && "atm_str_list_set called after list was freed");
    mtx_lock(&list->mutex);
    if (index >= list->capacity)
    {
        resize_buf(list->contents, list->capacity * sizeof(char *), (index + 1) * sizeof(char *));
    }
    list->contents[index] = calloc(STR_REQUIRED_BYTES(str), 1);
    strcpy(list->contents[index], str);
    mtx_unlock(&list->mutex);
}

void atm_str_list_remove(atomicStrList *list, size_t index)
{
    assert(list->capacity != SIZE_MAX && "atm_str_list_remove called after list was freed");
    mtx_lock(&list->mutex);
    if (index >= list->count)
    {
        mtx_unlock(&list->mutex);
        return; //If it doesn't exist, do nothing
    }
    free(list->contents[index]);
    if (index < list->count - 1)
    {
        memmove(list->contents + index, list->contents + index + 1, list->count - 1 - index);
        memset(list->contents + list->count - 1, 0, sizeof(char *));
    }
    list->count--;
    mtx_unlock(&list->mutex);
}

void atm_str_list_free(atomicStrList *list)
{
    mtx_lock(&list->mutex);
    for (size_t i = 0; i < list->count; i++)
    {
        free(list->contents[i]);
    }
    free(list->contents);
    list->contents = nullptr;
    list->capacity = SIZE_MAX;
    mtx_destroy(&list->mutex);
}

void atm_str_list_print(atomicStrList *list)
{
    mtx_lock(&list->mutex);
    for (size_t i = 0; i < list->count; i++)
    {
        puts(list->contents[i]);
        printBuffer(list->contents[i], strlen(list->contents[i]), 16);
    }
    mtx_unlock(&list->mutex);
}

bool atm_str_list_valid(atomicStrList *list)
{
    return list->capacity != SIZE_MAX;
}

#endif