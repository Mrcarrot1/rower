cmake_minimum_required(VERSION 3.26)
project(rower C)

set(CMAKE_C_STANDARD 23)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpmath=sse -msse -msse2 -pthread -lm")

if (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DROWER_NETWORK_DEBUG")
endif ()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=signed-integer-overflow -fsanitize=unsigned-integer-overflow")
add_executable(rower main.c
        graphics-utils.h
        graphics-utils.c
        gopher-protocol.c
        gopher-protocol.h
        network-interface.c
        network-interface.h
        buffer-utils.c
        buffer-utils.h
        network-interface-posix.c
        string_utils.c
        ui.c
        ui.h
        collections.c
        collections.h)

target_link_libraries(rower gtk-4 pangocairo-1.0 pango-1.0 harfbuzz gdk_pixbuf-2.0 cairo-gobject cairo graphene-1.0 gio-2.0 gobject-2.0 glib-2.0)
target_include_directories(rower PRIVATE /usr/include/gtk-4.0 /usr/include/pango-1.0 /usr/include/glib-2.0 /usr/lib/glib-2.0/include /usr/include/sysprof-4 /usr/include/harfbuzz /usr/include/freetype2 /usr/include/libpng16 /usr/include/libmount /usr/include/blkid /usr/include/fribidi /usr/include/cairo /usr/include/pixman-1 /usr/include/gdk-pixbuf-2.0 /usr/include/graphene-1.0 /usr/lib/graphene-1.0/include)