/*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef GOPHERBROWSER_NETWORK_INTERFACE_H
#define GOPHERBROWSER_NETWORK_INTERFACE_H

#include "buffer-utils.h"

#define GOPHER_PORT 70
#define HTTP_PORT 80
#define HTTPS_PORT 443
#define DEFAULT_DNS_CACHE_SIZE 32

enum GOPHER_SUPPORT
{
    GOPHER_SUPPORT_BASIC = 1,
    GOPHER_SUPPORT_PLUS = 2,
    GOPHER_SUPPORT_GOPHERS = 4,
    GOPHER_SUPPORT_ALL = GOPHER_SUPPORT_BASIC | GOPHER_SUPPORT_PLUS | GOPHER_SUPPORT_GOPHERS
};

void init_network_interface();
void cleanup_network_interface();
void get_network_capabilities();
resizableBuffer download_file_contents(const char *const host, const char *const uri);
resizableBuffer get_gopher_page(const char *const host, const char *const selector);
resizableBuffer get_gopher_page_ex(const char *const host, const char *const selector, int port);
void download_file(const char *const host, const char *const selector, int port);

#endif //GOPHERBROWSER_NETWORK_INTERFACE_H