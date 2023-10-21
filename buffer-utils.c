/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "buffer-utils.h"

resizableBuffer rb_new(size_t initialCapacity)
{
    void *buffer = calloc(initialCapacity, 1);
    if (buffer == nullptr)
    {
        return (resizableBuffer) { 0 };
    }
    return (resizableBuffer) { .count = 0, .capacity = initialCapacity, .contents = buffer };
}

void rb_free(resizableBuffer *buffer)
{
    buffer->count = 0;
    buffer->capacity = 0;
    free(buffer->contents);
    buffer->contents = nullptr;
}

void rb_append(resizableBuffer *buffer, size_t numBytes, void *input)
{
    size_t requiredCapacity = buffer->count + numBytes;
    if (requiredCapacity > buffer->capacity) rb_resize(buffer, requiredCapacity);
    memcpy(buffer->contents + buffer->count, input, numBytes);
    buffer->count += numBytes;
}

/*
 Resizes the provided buffer to the specified size.
 If newSize is less than the current capacity of the buffer, the buffer will be truncated to that length.
 Otherwise, it will be extended, with all new bytes set to zero.
 */
void rb_resize(resizableBuffer *buffer, size_t newSize)
{
    if (buffer->capacity == newSize) return;
    size_t newCount = newSize >= buffer->count ? buffer->count : newSize;
    void *newBuf = calloc(newSize, 1);
    memcpy(newBuf, buffer->contents, newCount);
    free(buffer->contents);
    buffer->contents = newBuf;
    buffer->capacity = newSize;
}

void printBuffer(void *buf, size_t n, int bytesPerRow)
{
    size_t numberRows = (n/2) % bytesPerRow == 0 ? (n/2) / bytesPerRow : (n/2) / bytesPerRow + 1;
    for (size_t i = 0; i < numberRows; i++)
    {
        for (int j = 0; j < bytesPerRow; j++)
        {
            printf("%02x ", ((unsigned char *)buf)[i * bytesPerRow + j]);
        }
        printf("\n");
    }
}

temporaryMem tm_new()
{
    return (temporaryMem) { .freed = false, .next = nullptr, .last = nullptr };
}

void *tm_add(temporaryMem *mem, void *contents, size_t contentSize)
{
    tMemNode *node = calloc(1, sizeof(tMemNode));
    void *buffer = calloc(1, contentSize);
    memcpy(buffer, contents, contentSize);
    if (mem->next == nullptr) mem->next = node;
    else mem->last->next = node;
    mem->last = node;
    return buffer;
}

void tm_node_free(tMemNode *node)
{
    if (node->next != nullptr) tm_node_free(node->next);
    free(node->buffer);
    free(node);
}

void tm_free(temporaryMem *mem)
{
    if (mem->freed) return;
    mem->freed = true;
    if (mem->next != nullptr)
        tm_node_free(mem->next);
    mem->next = nullptr;
    mem->last = nullptr;
}