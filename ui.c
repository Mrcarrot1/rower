/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "ui.h"
#include "buffer-utils.h"
#include "gopher-protocol.h"
#include "network-interface.h"
#include "string_utils.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <assert.h>
#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#define SET_MARGINS(gtkWidget, startMargin, endMargin, bottomMargin, topMargin) \
                                            gtk_widget_set_margin_bottom(gtkWidget, bottomMargin); \
                                            gtk_widget_set_margin_top(gtkWidget, topMargin);    \
                                            gtk_widget_set_margin_end(gtkWidget, endMargin);   \
                                            gtk_widget_set_margin_start(gtkWidget, startMargin)

#define CLEAR_ENTRY(x) gtk_entry_set_buffer(GTK_ENTRY(x), gtk_entry_buffer_new("", 0))


static GtkWidget *window, *pageBox, *pageEntry, *scrollView;
static gopherMenu currentPage = { 0 };
static atomic_bool pageIsLoading = false;
static temporaryMem pageTempMem = { 0 };

bool update_ui(GtkBox *box)
{
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollView), GTK_WIDGET(box));
    g_object_ref_sink(pageBox);
    pageBox = GTK_WIDGET(box);
    return false;
}

bool set_page_entry_text(stringBuilder *text)
{
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(pageEntry));
    gtk_entry_buffer_delete_text(buffer, 0, (int)gtk_entry_buffer_get_length(buffer));
    gb_gtk_ext_entry_buffer_append_text(buffer, text->contents);
    //gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(pageEntry)), text->contents, (int)sb_len(*text));
    sb_free(text);
    return false;
}

void append_page_entry_text(const char *text)
{
    gb_gtk_ext_entry_buffer_append_text(gtk_entry_get_buffer(GTK_ENTRY(pageEntry)), text);
}

void *load_page(const char *page)
{
    if (strlen(page) == 0) return nullptr;
    char host[512] = { 0 };
    gopherEntityType type = GOPHER_ENTITY_MENU; //Default to menu
    char selector[512] = { 0 };
    selector[0] = '/';
    size_t i = 0;
    for (; i < 511 && page[i] != '/'; i++)
    {
        host[i] = page[i];
        if (page[i] == '\0') break;
    }
    if (page[i] != '\0' && page[i+1] != '\0' && page[i+2] == '/')
    {
        type = page[i+1];
        i+=2;
    }
    size_t selectorStart = i;
    if (page[i] == '/')
    {
        for (; i < 511 && page[i] != '\0'; i++)
        {
            selector[i - selectorStart] = page[i];
        }
    }

    return load_page_ex(host, selector, 70, type);
}

void display_gopher_page(resizableBuffer *buf, gopherEntityType type)
{

}

void *load_page_ex(const char *host, const char *selector, int port, gopherEntityType type)
{
    if (pageIsLoading) return nullptr;
    pageIsLoading = true;
    if (!currentPage.freed) gopher_menu_free(&currentPage);
    temporaryMem prevTempMem = pageTempMem;
    pageTempMem = tm_new();
    GtkWidget *output = nullptr;
    resizableBuffer buf = get_gopher_page_ex(host, selector, port);
    stringBuilder sb = sb_new(STR_CONCAT_REQUIRED_BYTES(host, selector) + 3);
    stringBuilder *pageEntryText = tm_add(&pageTempMem, &sb, sizeof(stringBuilder));
    sb_append_contents(pageEntryText, host);
    char typeStr[3] = { '/', type, '\0'};
    sb_append_contents(pageEntryText, typeStr);
    sb_append_contents(pageEntryText, selector);
    g_idle_add(G_SOURCE_FUNC(set_page_entry_text), pageEntryText);
    /*GtkEntryBuffer *pageEntryBuffer = gtk_entry_get_buffer(GTK_ENTRY(pageEntry));
    gtk_entry_buffer_delete_text(pageEntryBuffer, 0, (int)gtk_entry_buffer_get_length(pageEntryBuffer));
    gb_gtk_ext_entry_buffer_append_text(pageEntryBuffer, host);

    gb_gtk_ext_entry_buffer_append_text(pageEntryBuffer, typeStr);
    gb_gtk_ext_entry_buffer_append_text(pageEntryBuffer, selector);*/
    if (type == GOPHER_ENTITY_INDEX_SERVER)
    {

    }
    switch (type)
    {
        case GOPHER_NS_ENTITY_HTML:
        case GOPHER_ENTITY_TEXTFILE:
        {
            output = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
            //gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollView), output);
            stringBuilder text = parse_gopher_textfile(buf.contents, buf.count);
            GtkWidget *label = gtk_label_new(text.contents);
            sb_free(&text);
            SET_DEFAULT_ALIGNMENT(label);
            PangoAttrList *fontAttrs = pango_attr_list_new();
            pango_attr_list_insert(fontAttrs, pango_attr_font_desc_new(pango_font_description_from_string("monospace 16")));
            gtk_label_set_attributes(GTK_LABEL(label), fontAttrs);
            gtk_label_set_selectable(GTK_LABEL(label), true);
            SET_MARGINS(label, 10, 10, 0, 10);
            rb_free(&buf);
            gtk_box_append(GTK_BOX(output), label);
            break;
        }
        case GOPHER_ENTITY_INDEX_SERVER:
        case GOPHER_ENTITY_MENU:
        {
            output = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
            gopherMenu menu = parse_gopher_menu(buf.contents);
            rb_free(&buf);

            pageBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
            //gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollView), output);
            for (size_t j = 0; j < menu.numEntities; j++)
            {
                render_gopher_entity_to_gtk(GTK_BOX(output), menu.entities + j);
            }
            currentPage = menu;
            break;
        }
        case GOPHER_ENTITY_CSO:
            break;
        case GOPHER_ENTITY_ERROR:
            break;
        case GOPHER_ENTITY_MAC_BINHEX:
            break;
        case GOPHER_ENTITY_PC_DOS_FILE:
            break;
        case GOPHER_ENTITY_UUENCODED_FILE:
            break;
        /*case GOPHER_ENTITY_INDEX_SERVER:
        {
            break;
        }*/
        case GOPHER_ENTITY_TELNET:
            break;
        case GOPHER_ENTITY_BINARY_FILE:
            break;
        case GOPHER_ENTITY_ALTERNATE_SERVER:
            break;
        case GOPHER_ENTITY_GIF:
            break;
        case GOPHER_ENTITY_IMAGE:
            break;
        case GOPHER_ENTITY_TELNET3270:
            break;
        case GOPHER_P_ENTITY_BMP:
            break;
        case GOPHER_P_ENTITY_MOVIE:
            break;
        case GOPHER_P_ENTITY_AUDIO:
            break;
        case GOPHER_NS_ENTITY_DOC:
            break;
        case GOPHER_NS_ENTITY_INFO_MESSAGE:
            break;
        case GOPHER_NS_ENTITY_IMAGE:
            break;
        case GOPHER_NS_ENTITY_RTF:
            break;
        case GOPHER_NS_ENTITY_SOUND:
            break;
        case GOPHER_NS_ENTITY_PDF:
            break;
        case GOPHER_NS_ENTITY_XML:
            break;
    }
    tm_free(&prevTempMem);
    pageIsLoading = false;
    //gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollView), output);
    g_idle_add(G_SOURCE_FUNC(update_ui), output);
    return nullptr;
}

struct gopherSearchData
{
    gopherEntity entity;
    GtkEntry *searchEntry;
};

void *load_page_ex_wrapper(gopherEntity *data)
{
    stringBuilder selector = sb_new_with_contents(data->selector.contents);
    if (data->type == GOPHER_ENTITY_INDEX_SERVER)
    {
        struct gopherSearchData *searchData = (struct gopherSearchData*)data;
        sb_append_char(&selector, '\t');
        sb_append_contents(&selector, gtk_entry_buffer_get_text(gtk_entry_get_buffer(searchData->searchEntry)));
    }
    const char *host = tm_add(&pageTempMem, (void*)data->host.contents, data->host.length + sizeof(char));
    void *result = load_page_ex(host, selector.contents, data->port, data->type);
    sb_free(&selector);
    return result;
}

void threaded_load_page_ex(gopherEntity *data)
{
    pthread_t thread;
    pthread_create(&thread, nullptr, (void *(*)(void *)) load_page_ex_wrapper, data);
}

void *load_page_wrapper(void*)
{
    load_page(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(pageEntry))));
    return nullptr;
}

void threaded_load_page()
{
    const char *page = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(pageEntry)));
    pthread_t thread;
    pthread_create(&thread, nullptr, load_page_wrapper, (void*)page);
}



void activate_ui(GtkApplication *app, gpointer user_data)
{
    GtkWidget *grid;
    GtkWidget *button;
    //GtkWidget *image;

    /* create a new window, and set its title */
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW (window), "Rower Gopher Browser");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    scrollView = gtk_scrolled_window_new();

    /* Here we construct the container that is going pack our buttons */
    grid = gtk_grid_new();

    /* Pack the container in the window */
    gtk_window_set_child(GTK_WINDOW (window), box);
    gtk_box_append(GTK_BOX(box), grid);
    gtk_box_append(GTK_BOX(box), scrollView);
    gtk_box_set_homogeneous(GTK_BOX(box), false);
    gtk_widget_set_vexpand(scrollView, true);
    //gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollView), grid);

    int currentRow = 0;
    int currentCol = 0;

    button = gtk_button_new_from_icon_name("back");
    SET_MARGINS(button, 10, 0, 10, 10);
    gtk_grid_attach(GTK_GRID(grid), button, currentCol, currentRow, 1, 1);
    currentCol++;

    button = gtk_button_new_from_icon_name("gtk-go-forward-ltr");
    SET_MARGINS(button, 10, 0, 10, 10);
    gtk_grid_attach(GTK_GRID(grid), button, currentCol, currentRow, 1, 1);
    currentCol++;

    GtkWidget *label = gtk_label_new("gopher://");
    PangoAttrList *attrList = pango_attr_list_new();
    pango_attr_list_insert(attrList, pango_attr_size_new(18000));
    gtk_label_set_attributes(GTK_LABEL(label), attrList);
    SET_MARGINS(label, 10, 0, 10, 10);
    gtk_grid_attach(GTK_GRID(grid), label, currentCol, currentRow, 2, 1);
    currentCol += 2;

    pageEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(pageEntry), "ex. gopher.calebmharper.com");
    gtk_entry_set_attributes(GTK_ENTRY(pageEntry), attrList);
    gtk_entry_set_max_length(GTK_ENTRY(pageEntry), 511);
    gtk_widget_set_hexpand(pageEntry, true);
    g_signal_connect(pageEntry, "activate", G_CALLBACK(threaded_load_page), nullptr);
    SET_MARGINS(pageEntry, 10, 0, 10, 10);
    gtk_grid_attach(GTK_GRID(grid), pageEntry, currentCol, currentRow, 4, 1);
    currentCol += 4;
    int pageEntryHeight = gtk_widget_get_height(pageEntry);

    gtk_widget_set_size_request (grid, -1, pageEntryHeight + 20);

    button = gtk_button_new_with_label("Go");
    SET_MARGINS(button, 10, 10, 10, 10);
    g_signal_connect(button, "clicked", G_CALLBACK(threaded_load_page), nullptr);
    gtk_grid_attach(GTK_GRID(grid), button, currentCol, currentRow++, 2, 1);
    currentCol += 2;

    //GtkWidget *scrollView = gtk_scrolled_window_new();
    //SET_MARGINS(scrollView, 10, 10, 0, 10);
    //gtk_grid_attach(GTK_GRID(grid), scrollView, 0, currentRow++, 8, 12);

    pageBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    //gtk_grid_attach(GTK_GRID(grid), pageGrid, 0, currentRow++, 8, 1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollView), pageBox);

    gtk_window_present(GTK_WINDOW(window));
}

GtkWidget *gb_gtk_ext_icon_label_button(const char *iconName, const char *label)
{
    GtkWidget *button = gtk_button_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_append(GTK_BOX(box), gtk_image_new_from_icon_name(iconName));
    GtkWidget *gLabel = gtk_label_new(label);
    gtk_widget_set_hexpand(gLabel, false);
    gtk_box_append(GTK_BOX(box), gLabel);
    gtk_button_set_child(GTK_BUTTON(button), box);
    SET_MARGINS(button, 10, 10, 0, 10);
    gtk_widget_set_hexpand(button, false);
    return button;
}

guint gb_gtk_ext_entry_buffer_append_text(GtkEntryBuffer *buffer, const char *contents)
{
    return gtk_entry_buffer_insert_text(buffer, gtk_entry_buffer_get_length(buffer),
                                        contents, (int)strlen(contents));
}

void handle_gopher_textfile(void*, gpointer data)
{
    gopherEntity *entity = data;
    //assert(entity->type == GOPHER_ENTITY_TEXTFILE);
    threaded_load_page_ex(entity);
}

void *download_pthread_wrapper(gopherEntity *entity)
{
    download_file(entity->host.contents, entity->selector.contents, entity->port);
    return nullptr;
}

void handle_gopher_page(void*, gpointer data)
{
    gopherEntity *entity = data;
    /*stringBuilder *pageEntryText = calloc(1, sizeof(stringBuilder));
    *pageEntryText = sb_new(STR_CONCAT_REQUIRED_BYTES(entity->host.contents, entity->selector.contents) + 3);
    sb_append_contents(pageEntryText, entity->host.contents);
    char typeStr[3] = { '/', entity->type, '\0'};
    sb_append_contents(pageEntryText, typeStr);
    sb_append_contents(pageEntryText, entity->selector.contents);
    g_idle_add(G_SOURCE_FUNC(set_page_entry_text), pageEntryText);*/
    GtkEntryBuffer *pageEntryBuffer = gtk_entry_get_buffer(GTK_ENTRY(pageEntry));

    gtk_entry_buffer_delete_text(pageEntryBuffer, 0, (int)gtk_entry_buffer_get_length(pageEntryBuffer));
    gb_gtk_ext_entry_buffer_append_text(pageEntryBuffer, entity->host.contents);
    /*stringBuilder selector = sb_new_with_contents(entity->selector.contents);
    if (entity->type == GOPHER_ENTITY_INDEX_SERVER)
    {
        struct gopherSearchData *searchData = data;
        sb_append_char(&selector, '\t');
        sb_append_contents(&selector, gtk_entry_buffer_get_text(gtk_entry_get_buffer(searchData->searchEntry)));
    }*/
    gb_gtk_ext_entry_buffer_append_text(pageEntryBuffer, entity->selector.contents);
    /*pthread_t thread;
    pthread_create(&thread, nullptr, (void *(*)(void *)) load_page_ex_wrapper, data);*/
    /*GThread *thread = g_thread_new("pageLoad", (GThreadFunc) load_page_ex_wrapper, data);
    g_thread_join(thread);*/
    threaded_load_page_ex(entity);
    //sb_free(&selector);
}

void handle_gopher_bin(void*, gopherEntity *entity)
{
    pthread_t downloadThread;
    //gopherEntity *entity = data;
    pthread_create(&downloadThread, nullptr, (void *(*)(void *)) download_pthread_wrapper, entity);
}

void render_gopher_entity_to_gtk(GtkBox *box, gopherEntity *entity)
{
    PangoAttrList *fontAttrs = pango_attr_list_new();
    //pango_attr_list_insert(fontAttrs, pango_attr_size_new(18000));
    pango_attr_list_insert(fontAttrs, pango_attr_font_desc_new(pango_font_description_from_string("monospace 16")));
    switch (entity->type)
    {
        case GOPHER_NS_ENTITY_INFO_MESSAGE:
        {
            GtkWidget *label = gtk_label_new(entity->displayName.contents);
            gtk_label_set_attributes(GTK_LABEL(label), fontAttrs);
            //gtk_label_set_selectable(GTK_LABEL(label), true);
            SET_DEFAULT_ALIGNMENT(label);
            SET_MARGINS(label, 10, 10, 0, 10);
            gtk_box_append(box, label);
            break;
        }
        case GOPHER_NS_ENTITY_HTML:
        case GOPHER_ENTITY_TEXTFILE:
        {
            GtkWidget *button = gb_gtk_ext_icon_label_button("text-x-generic", entity->displayName.contents);
            SET_DEFAULT_ALIGNMENT(button);
            gpointer data = (gpointer)entity;
            g_signal_connect(button, "clicked", G_CALLBACK(handle_gopher_textfile), data);
            gtk_box_append(box, button);
            break;
        }
        case GOPHER_ENTITY_MENU:
        {
            GtkWidget *button = gb_gtk_ext_icon_label_button("inode-directory", entity->displayName.contents);
            SET_DEFAULT_ALIGNMENT(button);
            g_signal_connect(button, "clicked", G_CALLBACK(handle_gopher_page), entity);
            gtk_box_append(box, button);
            break;
        }
        case GOPHER_ENTITY_CSO:
        {
            fprintf(stderr, "CSO phone book server unsupported. Use other client.\nServer info: %s:%d%s\n", entity->host.contents, entity->port, entity->selector.contents);
            break;
        }
        case GOPHER_ENTITY_ERROR:
        {
            stringBuilder sb = sb_new_with_contents("A server error has occurred: ");
            sb_append_contents(&sb, entity->displayName.contents);
            GtkWidget *label = gtk_label_new(sb.contents);
            SET_DEFAULT_ALIGNMENT(label);
            sb_free(&sb);
            fprintf(stderr, "%s\n", entity->displayName.contents);
            PangoAttrList *attrs = pango_attr_list_copy(fontAttrs);
            pango_attr_list_insert(attrs, pango_attr_foreground_new(255, 50, 50));
            gtk_label_set_attributes(GTK_LABEL(label), fontAttrs);
            gtk_box_append(box, label);
            break;
        }
        case GOPHER_ENTITY_BINARY_FILE:
        case GOPHER_ENTITY_MAC_BINHEX:
        case GOPHER_ENTITY_PC_DOS_FILE:
        {
            GtkWidget *button = gb_gtk_ext_icon_label_button("binary", entity->displayName.contents);
            SET_DEFAULT_ALIGNMENT(button);
            gtk_box_append(box, button);
            g_signal_connect(button, "clicked", G_CALLBACK(handle_gopher_bin), entity);
            break;
        }
        case GOPHER_ENTITY_INDEX_SERVER:
        {
            GtkWidget *localBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
            //GtkWidget *button = gb_gtk_ext_icon_label_button("search", entity->displayName.contents);
            GtkWidget *label = gtk_label_new(entity->displayName.contents);
            gtk_label_set_attributes(GTK_LABEL(label), fontAttrs);
            SET_DEFAULT_ALIGNMENT(label);
            SET_MARGINS(label, 10, 10, 0, 0);
            gtk_box_append(GTK_BOX(localBox), label);
            GtkWidget *entry = gtk_entry_new();
            gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter search text...");
            gtk_box_append(GTK_BOX(localBox), entry);
            SET_DEFAULT_ALIGNMENT(entry);
            gtk_editable_set_width_chars(GTK_EDITABLE(entry), 50);
            GtkWidget *button = gtk_button_new_from_icon_name("search");
            SET_DEFAULT_ALIGNMENT(button);
            //gtk_widget_set_sensitive(button, false);
            gtk_box_append(GTK_BOX(localBox), button);
            SET_DEFAULT_ALIGNMENT(localBox);
            struct gopherSearchData searchData;
            searchData.entity = *entity;
            searchData.searchEntry = GTK_ENTRY(entry);
            void *callbackData = tm_add(&pageTempMem, &searchData, sizeof(struct gopherSearchData));
            g_signal_connect(button, "clicked", G_CALLBACK(handle_gopher_page), callbackData);
            g_signal_connect(entry, "activate", G_CALLBACK(handle_gopher_page), callbackData);
            gtk_box_append(box, localBox);
            break;
        }
        case GOPHER_NS_ENTITY_IMAGE:
        case GOPHER_ENTITY_IMAGE:
        case GOPHER_P_ENTITY_BMP:
        case GOPHER_ENTITY_GIF:
        {
            fprintf(stderr, "Loading image %s\n", entity->selector.contents);
            resizableBuffer imageBuf = entity->prefetchedData;
            if (imageBuf.count == 0)
            {
                GtkWidget *label = gtk_label_new("Image failed to load.");
                gtk_label_set_attributes(GTK_LABEL(label), fontAttrs);
                gtk_box_append(box, label);
                break;
            }
            /*FILE *rawImg = fopen("rawimage.bin", "wb");
            fwrite(imageBuf.contents, 1, imageBuf.count, rawImg);
            fclose(rawImg);*/
            int x, y, imgChannels;
            stbi_uc *image = stbi_load_from_memory(imageBuf.contents, (int)imageBuf.count, &x, &y, &imgChannels, 4);

            //Write image as BMP for debugging
            /*FILE *img = fopen("image.bmp", "wb");
            fwrite(image, x * y * imgChannels, 1, img);
            fclose(img);*/

            //GdkTexture *texture = gdk_memory_texture_new(x, y, GDK_MEMORY_R8G8B8A8, (GBytes *) image, x * 4);
            GError *error = nullptr;
            GBytes *imgBytes = g_bytes_new(imageBuf.contents, imageBuf.count);
            GdkTexture *texture = gdk_texture_new_from_bytes(imgBytes, &error);
            if (error != nullptr)
            {
                fprintf(stderr, "A GDK error occurred when loading %s: %s", entity->selector.contents, error->message);
                GtkWidget *label = gtk_label_new("Image failed to load.");
                gtk_label_set_attributes(GTK_LABEL(label), fontAttrs);
                gtk_box_append(box, label);
                break;
            }
            GtkWidget *picture = gtk_picture_new();
            gtk_picture_set_paintable(GTK_PICTURE(picture), GDK_PAINTABLE(texture));
            gtk_picture_set_content_fit(GTK_PICTURE(picture), GTK_CONTENT_FIT_CONTAIN);
            SET_MARGINS(picture, 10, 10, 0, 10);
            gtk_widget_set_halign(picture, GTK_ALIGN_START);
            gtk_box_append(box, picture);
            break;
        }
        case GOPHER_ENTITY_UUENCODED_FILE:
        case GOPHER_ENTITY_TELNET:

        case GOPHER_ENTITY_ALTERNATE_SERVER:

        case GOPHER_ENTITY_TELNET3270:

        case GOPHER_P_ENTITY_MOVIE:
        case GOPHER_P_ENTITY_AUDIO:
        case GOPHER_NS_ENTITY_DOC:


        case GOPHER_NS_ENTITY_RTF:
        case GOPHER_NS_ENTITY_SOUND:
        case GOPHER_NS_ENTITY_PDF:
        case GOPHER_NS_ENTITY_XML:
            fprintf(stderr, "This client does not support Gopher entities of type %s(%c)\n", get_string_gopher_type(entity->type), entity->type);
    }
}