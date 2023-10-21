/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#ifndef GOPHERBROWSER_GOPHER_PROTOCOL_H
#define GOPHERBROWSER_GOPHER_PROTOCOL_H

#include <ctype.h>
#include <string.h>
#include <stdatomic.h>
#include "buffer-utils.h"
#include "string_utils.h"

/*
 Defines the various types used when defining entities in a Gopher directory.
 */
typedef enum gopherEntityType : char
{
    //Standard Gopher entity types
    GOPHER_ENTITY_TEXTFILE = '0',
    GOPHER_ENTITY_MENU,
    GOPHER_ENTITY_CSO,
    GOPHER_ENTITY_ERROR,
    GOPHER_ENTITY_MAC_BINHEX,
    GOPHER_ENTITY_PC_DOS_FILE,
    GOPHER_ENTITY_UUENCODED_FILE,
    GOPHER_ENTITY_INDEX_SERVER,
    GOPHER_ENTITY_TELNET,
    GOPHER_ENTITY_BINARY_FILE,
    GOPHER_ENTITY_ALTERNATE_SERVER = '+',
    GOPHER_ENTITY_GIF = 'g',
    GOPHER_ENTITY_IMAGE = 'I',
    GOPHER_ENTITY_TELNET3270 = 'T',

    //Standard Gopher+ entity types
    GOPHER_P_ENTITY_BMP = ':',
    GOPHER_P_ENTITY_MOVIE = ';',
    GOPHER_P_ENTITY_AUDIO = '<',

    //Nonstandard but occasionally used entity types
    GOPHER_NS_ENTITY_DOC = 'd',
    GOPHER_NS_ENTITY_HTML = 'h',
    GOPHER_NS_ENTITY_INFO_MESSAGE = 'i',
    GOPHER_NS_ENTITY_IMAGE = 'p',
    GOPHER_NS_ENTITY_RTF = 'r',
    GOPHER_NS_ENTITY_SOUND = 's',
    GOPHER_NS_ENTITY_PDF = 'P',
    GOPHER_NS_ENTITY_XML = 'X'
} gopherEntityType;

const char *get_string_gopher_type(gopherEntityType type);

/*
 Defines a single Gopher directory entity.
 */
typedef struct gopherEntity
{
    gopherEntityType type;
    stringView displayName; //In RFC 1436, this is referred to as User_Name.
                            //I have elected to change this, as that term often refers to a very different concept.
    stringView selector;
    stringView host;
    int port;
    resizableBuffer prefetchedData;
} gopherEntity;

/*
 Defines a Gopher menu.
 The menu structure is used for the entirety of a Gopher page.
 */
typedef struct gopherMenu
{
    atomic_bool freed;
    size_t numEntities;
    gopherEntity *entities;
} gopherMenu;

/*
 Gets the first full Gopher token starting at (source + currentPosition).
 */
stringBuilder get_gopher_token(const char *source, size_t *currentPosition);

/*
 Creates a new Gopher menu entity.
 */
gopherEntity gopher_entity_new(gopherEntityType type, const char *displayName, const char *selector, const char *host, int port);

/*
 Frees the heap memory associated with a Gopher menu entity.
 */
void gopher_entity_free(gopherEntity *entity);

/*
 Creates a Gopher menu structure from the given source text.
 */
gopherMenu parse_gopher_menu(const char *source);

/*
 Frees the heap memory associated with a Gopher menu and its child entities.
 */
void gopher_menu_free(gopherMenu *menu);

/*
 Parses a text file in the gopher format into a stringBuilder, stripping out any periods on their own line.
 */
stringBuilder parse_gopher_textfile(const char *buf, size_t bufSize);

#endif //GOPHERBROWSER_GOPHER_PROTOCOL_H