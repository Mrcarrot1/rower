/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdio.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "graphics-utils.h"
#include "network-interface.h"
#include "string_utils.h"
#include "gopher-protocol.h"
#include "ui.h"

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;


    app = gtk_application_new("com.calebmharper.rower", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK(activate_ui), NULL);
    //g_main_loop_new()
    status = g_application_run(G_APPLICATION (app), argc, argv);
    g_object_unref(app);

    return status;

    char host[2048] = { 0 };
    char uri[2048] = { 0 };
    printf("Please enter the site you would like to visit: ");
    fgets(host, 2047, stdin);
    if (host[strlen(host) - 1] == '\n') host[strlen(host) - 1] = '\0';
    printf("Please enter the page on that site you would like to visit, or nothing for the homepage: ");
    fgets(uri, 2047, stdin);
    if (uri[strlen(uri) - 1] == '\n') uri[strlen(uri) - 1] = '\0';
    resizableBuffer testPage = get_gopher_page(host, uri);
    fprintf(stderr, "Received %zu bytes from server.\n", testPage.count);
    gopherMenu menu = parse_gopher_menu(testPage.contents);
    for (size_t i = 0; i < menu.numEntities; i++)
    {
        gopherEntity entity = menu.entities[i];
        printf("%c (%s)[%s:%d%s]\n", entity.type, entity.displayName.contents, entity.host.contents, entity.port, entity.selector.contents);
    }



    rb_free(&testPage);

    //cleanup_network_interface();

    return 0;
    /*int width = 1920;
    int height = 1080;

    if (SDL_Init(SDL_INIT_VIDEO != 0))
    {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Bad Gopher Browser",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                          SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        fprintf(stderr, "Could not create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        fprintf(stderr, "Could not create SDL renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    TTF_Init();
    const char *text = u8"This is a bad Gopher Browser\nいろはにほへと\nちりぬるを\nわかよたれそ\nつねならむ\nうゐのおくやま\nけふこえて\nあさきゆめみし\nゑひもせす";
    TTF_Font *cjkFont = TTF_OpenFont(CJK_FONT, 32);
    SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(cjkFont, text, (SDL_Color) { 255, 255, 255, 255 }, 800);
    SDL_Rect rect = { 50, 100, surface->w, surface->h };
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_Event event;
    bool done = false;

    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT)
            {
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_CLOSE:
                        return 0;
                    default:
                        break;
                }

            }
        }
        SDL_RenderCopy(renderer, tex, NULL, &rect);
        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer);
        surface = TTF_RenderUTF8_Blended_Wrapped(cjkFont, text, (SDL_Color) { 255, 255, 255, 255 }, 800);
        surface->w = width;
        surface->h = height;
        rect = (SDL_Rect) { 50, 100, surface->w, surface->h };

        tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);


    }

    return 0;*/
}