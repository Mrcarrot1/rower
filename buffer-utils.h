/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef GOPHERBROWSER_BUFFER_UTILS_H
#define GOPHERBROWSER_BUFFER_UTILS_H

#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <stdatomic.h>

#define DEFAULT_BUFFER_SIZE 2048

typedef struct resizableBuffer
{
    size_t count;
    size_t capacity;
    void *contents;
} resizableBuffer;

resizableBuffer rb_new(size_t initialCapacity);
#define rb_new_with_default_size() rb_new(DEFAULT_BUFFER_SIZE)
void rb_free(resizableBuffer *buffer);
void rb_append(resizableBuffer *buffer, size_t numBytes, void *input);
void rb_resize(resizableBuffer *buffer, size_t newSize);
#define RB_EMPTY ((resizableBuffer) { 0 })


typedef struct tMemNode
{
    void *buffer;
    struct tMemNode *next;
} tMemNode;

typedef struct temporaryMem
{
    atomic_bool freed;
    struct tMemNode *next;
    struct tMemNode *last;
} temporaryMem;

temporaryMem tm_new();
void tm_free(temporaryMem *mem);

void *tm_add(temporaryMem *mem, void *contents, size_t contentSize);

void printBuffer(void *buf, size_t n, int bytesPerRow);

#endif //GOPHERBROWSER_BUFFER_UTILS_H