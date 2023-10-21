# Rower Gopher Browser
This project is an implementation of a Gopher protocol client for Unix-like systems using GTK. It aims to provide compatibility with both the core Gopher protocol and various extensions that have been used with it over the years.

### Why Gopher?
Excellent question. I am a student at the University of Minnesota, where the Gopher protocol was first developed and deployed, though the University has since shut down all of its Gopher servers. I learned of the existence of this protocol and its history relating to the school, and I decided I had to do something with it.

Additionally, the Gopher protocol, having received no substantial updates since the mid-1990s, is very simple to implement, especially when compared to the modern web.

### Why POSIX?
I wanted to learn on a fairly low level how to use these technologies. Additionally, this is the system I use and expect to use for the foreseeable future. The project is designed to be relatively easy to reimplement using another system API, so other platform support is not out of the question in the future.

## Building
Building this project requires a compiler that supports certain features of the upcoming C23/C2x standard.

Alternatively, you can attempt to replace these features with their C17 equivalents. The primary incompatibility is caused by the usage of the `nullptr` constant, which may be replaced with `NULL`.

## Protocol Support
Support for Gopher is incomplete at this time. Most common Gopher entities are supported in some form, but many are not.

Additionally, there is no Gopher+ support that is ready for use. However, since servers are required to support basic Gopher clients regardless(and many servers do not support Gopher+ either), it's still quite usable.

The full support list for standard and common but nonstandard Gopher elements is below.

### Gopher Elements Introduced in RFC 1431(1991)
[X] Text File(`0`)
[X] Menu(`1`)
[ ] CSO Phone Book Server(`2`)
[X] Error(`3`)
[X] Macintosh Binhex File(`4`)
[X] PC-DOS File(`5`)
[ ] Uuencoded File(`6`)
[X] Index Server(`7`)
[ ] Telnet(`8`)
[X] Binary File(`9`)
[ ] Alternate Server(`+`)
[ ] GIF(`g`)
[X] Image(`I`)
[ ] Telnet-3270(`T`)

### Gopher+ Elements(1993)
[X] Bitmap Image(`:`)
[ ] Movie(`;`)
[ ] Audio(`<`)

### Nonstandard Gopher Elements in Common Use
[ ] Microsoft Word Document(`d`)
[X] HTML Document(`h`) *Note: supports only displaying HTML as plaintext
[X] Info Message(`i`)
[X] Image(`p`)
[ ] RTF Document(`r`)
[ ] Sound(`s`)
[ ] PDF Document(`P`)
[ ] XML Document(`X`)