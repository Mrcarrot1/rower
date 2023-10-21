/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "network-interface.h"
#include "string_utils.h"

//Only compile this part on Unix systems--
//We don't support anything else yet, but it should be relatively easy
//to write a Win32 interface for this code.
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <threads.h>

struct addrCacheEntry
{
    stringView hostname;
    struct sockaddr_in addr;
};

static struct
{
    atomic_bool isInitialized;
    mtx_t mutex;
    size_t entryCap;
    size_t numEntries;
    struct addrCacheEntry *entries;
} addrCache = { 0 };

inline void init_addr_cache()
{
    if (addrCache.isInitialized) return;
    addrCache.isInitialized = true;
    mtx_init(&addrCache.mutex, mtx_plain);
    addrCache.entryCap = DEFAULT_DNS_CACHE_SIZE;
    addrCache.numEntries = 0;
    addrCache.entries = calloc(addrCache.entryCap, sizeof(struct addrCacheEntry));
}
void init_addr_cache();

void addr_cache_add(const char *host, struct sockaddr_in entry)
{
    init_addr_cache();
    mtx_lock(&addrCache.mutex);
    addrCache.numEntries++;
    if (addrCache.numEntries > addrCache.entryCap)
    {
        addrCache.entryCap *= 2;
        void *newBuf = calloc(addrCache.entryCap, sizeof(struct addrCacheEntry));
        memcpy(newBuf, addrCache.entries, (addrCache.numEntries - 1) * sizeof(struct addrCacheEntry));
        free(addrCache.entries);
        addrCache.entries = newBuf;
    }
    struct addrCacheEntry cacheEntry = { .addr = entry, .hostname = sv_new(host) };
    addrCache.entries[addrCache.numEntries - 1] = cacheEntry;
    mtx_unlock(&addrCache.mutex);
}

struct sockaddr_in addr_cache_get(const char *host)
{
    init_addr_cache();
    mtx_lock(&addrCache.mutex);
    struct sockaddr_in output = { 0 };
    for (size_t i = 0; i < addrCache.numEntries; i++)
    {
        if (strcmp(addrCache.entries[i].hostname.contents, host) == 0)
            output = addrCache.entries[i].addr;
    }
    mtx_unlock(&addrCache.mutex);
    return output;
}

const char *get_ip_addr(const char * const host)
{
    struct hostent *h = gethostbyname(host);
    for (size_t i = 0; i < h->h_length; i++)
    {
        puts(h->h_addr_list[i]);
    }
    return h->h_addr_list[0];
}

resizableBuffer get_gopher_page(const char *const host, const char * const selector)
{
    return get_gopher_page_ex(host, selector, 70);
}

resizableBuffer get_gopher_page_ex(const char *const host, const char *const selector, int port)
{
#ifdef ROWER_NETWORK_DEBUG
    fprintf(stderr, "Downloading %s:%d%s\n", host, port, selector);
#endif
    char buffer[DEFAULT_BUFFER_SIZE] = {0}; //Buffer for temporarily storing the data we receive
    int32_t sock;


    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int error = 0;
    struct
    {
        struct sockaddr_in addr;
        in_port_t port;
        int socktype;
    } hostInfo = { .socktype = AF_INET, .port = htons(port) };
    struct sockaddr_in addr = addr_cache_get(host);
    if (addr.sin_port == 0)
    {
        fprintf(stderr, "DNS Cache Miss: %s\n", host);
        struct addrinfo *servinfo = nullptr;
        error = getaddrinfo(host, "gopher", &hints, &servinfo);
        hostInfo.addr = *(struct sockaddr_in*)servinfo->ai_addr;
        hostInfo.socktype = servinfo->ai_socktype;
        addr_cache_add(host, hostInfo.addr);
        freeaddrinfo(servinfo);
    }
    else
    {
        fprintf(stderr, "DNS Cache Hit: %s\n", host);
        hostInfo.addr = addr;
        hostInfo.socktype = SOCK_STREAM;
    }

    //((struct sockaddr_in*)servinfo->ai_addr)->sin_port = htons((uint16_t)port);

    if (error != 0)
    {
        fprintf(stderr, "Could not load page %s%s\n", host, selector);
        return RB_EMPTY;
    }

    sock = socket(AF_INET, hostInfo.socktype, IPPROTO_IP);

    error = connect(sock, (struct sockaddr *) &hostInfo.addr, sizeof(hostInfo.addr));

    if (error != 0)
    {
        fprintf(stderr, "Could not establish connection to %s port %d: %s\n", host, port, strerror(errno));
        return RB_EMPTY;
    }

    stringBuilder initialMessage = sb_new_with_contents(selector);
#ifdef EXPERIMENTAL_GOPHER_PLUS
    sb_append_contents(&initialMessage, "\t+");
#endif
    sb_append_contents(&initialMessage, "\r\n");

    send(sock, initialMessage.contents, sb_len(initialMessage), 0);
    sb_free(&initialMessage);

    ssize_t len = recv(sock, buffer, DEFAULT_BUFFER_SIZE - 1, 0);
    if (len == -1)
    {
        fprintf(stderr, "Could not receive data: %s", strerror(errno));
        return RB_EMPTY;
    }
    resizableBuffer output = rb_new(len);
    rb_append(&output, len, buffer);
    while (len != 0)
    {
        len = recv(sock, buffer, DEFAULT_BUFFER_SIZE - 1, 0);
        if (len == -1)
        {
            fprintf(stderr, "Could not receive data: %s", strerror(errno));
            return RB_EMPTY;
        }
        rb_append(&output, len, buffer);
    }
    //len = recv(sock, buffer, DEFAULT_BUFFER_SIZE - 1, 0);


    buffer[len] = '\0';

    //printf("Received %s (%d bytes).\n", buffer, len);

    close(sock);

#ifdef ROWER_NETWORK_DEBUG
    if (output.count == 0)
        fprintf(stderr, "Warning: resource %s:%d%s downloaded 0 bytes\n", host, port, selector);
#endif

    return output;
}

void download_file(const char *const host, const char *const selector, int port)
{
    fprintf(stderr, "Downloading %s:%d%s\n", host, port, selector);
    char buffer[DEFAULT_BUFFER_SIZE] = {0}; //Buffer for temporarily storing the data we receive
    int32_t sock;

    struct addrinfo *servinfo = nullptr;
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int error = getaddrinfo(host, "gopher", &hints, &servinfo);
    ((struct sockaddr_in*)servinfo->ai_addr)->sin_port = htons((uint16_t)port);

    if (error != 0)
    {
        fprintf(stderr, "Could not load page %s%s\n", host, selector);
        return;
    }

    sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    error = connect(sock, servinfo->ai_addr, servinfo->ai_addrlen);

    if (error != 0)
    {
        fprintf(stderr, "Could not establish connection to %s port %d: %s\n", host, port, strerror(errno));
        return;
    }

    const char *fileName = strchr(selector, '/');
    while (true)
    {
        const char *f = strchr(fileName + 1, '/');
        if (f == nullptr) break;
        fileName = f;
    }
    stringBuilder filePath = sb_new(64);
    const char *homePath = getenv("HOME");
    if (homePath == nullptr) homePath = ".";
    sb_append_contents(&filePath, homePath);
    sb_append_contents(&filePath, "/Downloads");
    sb_append_contents(&filePath, fileName);
#warning TODO: Add better checking to make sure the downloads folder exists dumbass
    FILE *file = fopen(filePath.contents, "wb+");
    if (file == nullptr)
    {
        fprintf(stderr, "Could not open file %s: %s\n", filePath.contents, strerror(errno));
        return;
    }

    fprintf(stderr, "Saving downloaded file as %s\n", filePath.contents);
    sb_free(&filePath);

    stringBuilder initialMessage = sb_new_with_contents(selector);
    sb_append_contents(&initialMessage, "\r\n");

    send(sock, initialMessage.contents, sb_len(initialMessage), 0);
    sb_free(&initialMessage);

    ssize_t len = recv(sock, buffer, DEFAULT_BUFFER_SIZE - 1, 0);
    if (len == -1)
    {
        fprintf(stderr, "Could not receive data: %s", strerror(errno));
        return;
    }
    fwrite(buffer, 1, len, file);

    while (len != 0)
    {
        len = recv(sock, buffer, DEFAULT_BUFFER_SIZE - 1, 0);
        if (len == -1)
        {
            fprintf(stderr, "Could not receive data: %s", strerror(errno));
            return;
        }
        fwrite(buffer, 1, len, file);
    }
    fclose(file);
}

#endif