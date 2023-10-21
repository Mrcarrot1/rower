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