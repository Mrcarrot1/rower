/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


/*#include "graphics-utils.h"

void log_sdl_error(FILE *stream, const char * const msg)
{
    fprintf(stream, "%s: %s\n", msg, SDL_GetError());
}

SDL_Texture *load_texture(const char * const file, SDL_Renderer *renderer)
{
    //Initialize to nullptr to avoid dangling pointer issues
    SDL_Texture *texture = nullptr;
    //Load the image
    SDL_Surface *loadedImage = SDL_LoadBMP(file);
    //If the loading went ok, convert to texture and return the texture
    if (loadedImage != nullptr)
    {
        texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
        SDL_FreeSurface(loadedImage);
        //Make sure converting went ok too
        if (texture == nullptr)
        {
            log_sdl_error(stderr, "Could not create texture from surface");
        }
    }
    else
    {
        log_sdl_error(stderr, "Could not load image");
    }
    return texture;
}

void draw_rect(int x, int y, int w, int h)
{
    SDL_Rect rect = { .x = x, .y = y, .w = w, .h = h };

}*/