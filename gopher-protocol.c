/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdlib.h>
#include <assert.h>
#include "gopher-protocol.h"
#include "network-interface.h"

#define PREV_CHAR_IS(c) (*currentPosition > 0 && source[*currentPosition - 1] == (c))

stringBuilder get_gopher_token(const char *source, size_t *currentPosition)
{
    if (source == nullptr) return SB_EMPTY;
    if (source[*currentPosition] == '\0') return SB_EMPTY;
    //Things that are individual tokens:
    //The first character on every line(always)
    //Anything separated by tabs-- including nothing
    //NEWLINES-- we don't parse these, but they're syntactically significant
    stringBuilder output = sb_new(32);
    if (*currentPosition == 0 || source[*currentPosition - 1] == '\n')
    {
        sb_append_char(&output, source[*currentPosition]);
        (*currentPosition)++;
        return output;
    }
    else
    {
        char c;
        for (; (c = source[*currentPosition]) != '\0'; (*currentPosition)++)
        {
            if (isspace(c))
            {
                if (c == '\t')
                {
                    (*currentPosition)++;
                    return output;
                }
                if (c == '\r' || c == '\n')
                {
                    if (sb_len(output) != 0 && !isspace(get_last_char(output.contents)))
                        return output;
                    else
                    {
                        sb_append_char(&output, c);
                        if (c == '\n')
                        {
                            (*currentPosition)++;
                            return output;
                        }
                    }
                }
                else //Otherwise, we add it to the current token. Note that space is not a token-breaking character here.
                {
                    sb_append_char(&output, c);
                }
            }
            else sb_append_char(&output, c); //If it's not whitespace, add it on
        }
        return output; //If we get to the end of the buffer, return what we have
    }
}
#undef PREV_CHAR_IS

#define PREV_CHAR_IS(c) (i > 0 && buf[i - 1] == (c))
#define NEXT_CHAR_IS(c) (i + 1 < bufSize && buf[i + 1] == (c))

stringBuilder parse_gopher_textfile(const char *buf, size_t bufSize)
{
    stringBuilder output = sb_new(bufSize + 1);
    for (size_t i = 0; i < bufSize; i++)
    {
        if (!(buf[i] == '.' && PREV_CHAR_IS('\n')))
        {
            output.contents[i] = buf[i]; //This is effectively equivalent to appending the character,
                                         //but it doesn't require calling the sb_append_char function
                                         //hundreds or thousands of times.
                                         //We already allocated as much memory as we're going to use up front.
        }
        else if (NEXT_CHAR_IS('\r')) break;
    }
    return output;
}
#undef NEXT_CHAR_IS
#undef PREV_CHAR_IS

gopherEntity gopher_entity_new(gopherEntityType type, const char *displayName, const char *selector, const char *host, int port)
{
    return (gopherEntity)
    {
        .type = type,
        .displayName = sv_new(displayName),
        .selector = sv_new(selector),
        .host = sv_new(host),
        .port = port
    };
}

void gopher_entity_free(gopherEntity *entity)
{
    sv_free(&entity->displayName);
    sv_free(&entity->selector);
    sv_free(&entity->host);
    rb_free(&entity->prefetchedData);
}

#define IS_EMPTY_OR_CRLF(sb) ((sb).capacity == 0 || strcmp((sb).contents, "\r\n") == 0)

gopherEntity parse_gopher_entity(const char * source, size_t *currentPosition, bool *reachedEnd)
{
    //Each entity is made up of 5 tokens: type, display, selector, host, port
    //There should also be a CRLF at the end, but we don't have to store that
    stringBuilder tokens[5] = { SB_EMPTY, SB_EMPTY, SB_EMPTY, SB_EMPTY, SB_EMPTY };
    int i = 0;
    for (; i < 5; i++)
    {
        tokens[i] = get_gopher_token(source, currentPosition);
        if (tokens[i].capacity == 0) *reachedEnd = true;
        if (IS_EMPTY_OR_CRLF(tokens[i])) break; //Handle early end by breaking out
    }
    //if (i == 4) //Handle line too long by ignoring all extra tokens until CRLF or EOF
    {
        stringBuilder currentToken = get_gopher_token(source, currentPosition);
        while (!IS_EMPTY_OR_CRLF(currentToken)) currentToken = get_gopher_token(source, currentPosition);
        if (currentToken.capacity == 0) *reachedEnd = true;
    }
    for (; i > 0 && i < 5; i++) //Fill in any empty tokens we might have
    {
        switch (i)
        {
            case 1:
                tokens[i] = sb_new_with_contents("Undefined Name");
                break;
            case 2:
                tokens[i] = sb_new_with_contents("/");
                break;
            case 3:
                tokens[i] = sb_new_with_contents("error.host");
                break;
            case 4:
                tokens[i] = sb_new_with_contents("70");
                break;
            default:
                assert(false && "Unreachable");
        }
    }
    char type = tokens[0].contents[0];
    int port = (int)strtol(tokens[4].contents, nullptr, 10);
    gopherEntity output = {
                .type = type,
                .displayName = sv_new_from_sb(tokens[1]),
                .selector = sv_new_from_sb(tokens[2]),
                .host = sv_new_from_sb(tokens[3]),
                .port = port
            };
    switch (output.type)
    {
        case GOPHER_ENTITY_IMAGE:
        case GOPHER_NS_ENTITY_IMAGE:
        case GOPHER_ENTITY_GIF:
        case GOPHER_P_ENTITY_BMP:
            output.prefetchedData = get_gopher_page_ex(output.host.contents, output.selector.contents, output.port);
            break;
        default:
            output.prefetchedData = RB_EMPTY;
    }
    for (i = 0; i < 5; i++)
    {
        sb_free(&tokens[i]);
    }
    return output;
}

gopherMenu parse_gopher_menu(const char *source)
{
    size_t entityCount = 0;
    size_t entityCapacity = 32;
    size_t currentPosition = 0;
    bool reachedEnd = false;
    gopherEntity *entities = calloc(entityCapacity, sizeof(gopherEntity));
    gopherEntity currentEntity = parse_gopher_entity(source, &currentPosition, &reachedEnd);
    do
    {
        if (entityCount == entityCapacity)
        {
            gopherEntity *newBuffer = calloc(entityCapacity += 32, sizeof(gopherEntity));
            memcpy(newBuffer, entities, entityCount * sizeof(gopherEntity));
            free(entities);
            entities = newBuffer;
        }
        entities[entityCount] = currentEntity;
        entityCount++;
        currentEntity = parse_gopher_entity(source, &currentPosition, &reachedEnd);
    } while (!reachedEnd);

    return (gopherMenu) { .entities = entities, .numEntities = entityCount, .freed = false };
}

void gopher_menu_free(gopherMenu *menu)
{
    if (menu->freed) return;
    menu->freed = true;
    for (size_t i = 0; i < menu->numEntities; i++)
    {
        gopher_entity_free(&menu->entities[i]);
    }
    if (menu->numEntities > 0) free(menu->entities);
}

const char *get_string_gopher_type(gopherEntityType type)
{
    switch (type)
    {

        case GOPHER_ENTITY_TEXTFILE:
            return "text file";
        case GOPHER_ENTITY_MENU:
            return "menu";
        case GOPHER_ENTITY_CSO:
            return "CSO phone book entity";
        case GOPHER_ENTITY_ERROR:
            return "error condition";
        case GOPHER_ENTITY_MAC_BINHEX:
            return "Macintosh BINHEX file";
        case GOPHER_ENTITY_PC_DOS_FILE:
            return "PC-DOS binary file";
        case GOPHER_ENTITY_UUENCODED_FILE:
            return "uuencoded file";
        case GOPHER_ENTITY_INDEX_SERVER:
            return "index server";
        case GOPHER_ENTITY_TELNET:
            return "Telnet session";
        case GOPHER_ENTITY_BINARY_FILE:
            return "binary file";
        case GOPHER_ENTITY_ALTERNATE_SERVER:
            return "alternate server";
        case GOPHER_ENTITY_GIF:
            return "GIF image";
        case GOPHER_ENTITY_IMAGE:
            return "image";
        case GOPHER_ENTITY_TELNET3270:
            return "tn3270-based Telnet session";
        case GOPHER_P_ENTITY_BMP:
            return "(+)BMP image";
        case GOPHER_P_ENTITY_MOVIE:
            return "(+)movie";
        case GOPHER_P_ENTITY_AUDIO:
            return "(+)audio";
        case GOPHER_NS_ENTITY_DOC:
            return "(nonstandard) MS Word document";
        case GOPHER_NS_ENTITY_HTML:
            return "(nonstandard) HTML document";
        case GOPHER_NS_ENTITY_INFO_MESSAGE:
            return "(nonstandard) info message";
        case GOPHER_NS_ENTITY_IMAGE:
            return "(nonstandard) image";
        case GOPHER_NS_ENTITY_RTF:
            return "(nonstandard) RTF text document";
        case GOPHER_NS_ENTITY_SOUND:
            return "(nonstandard) sound";
        case GOPHER_NS_ENTITY_PDF:
            return "(nonstandard) PDF document";
        case GOPHER_NS_ENTITY_XML:
            return "(nonstandard) XML document";
        default:
            return "unknown type";
    }
}