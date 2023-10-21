# Refactoring the Rower Gopher Client
Rower has been largely successful in implementing the core of the Gopher protocol, and large portions of it are a solid base to build upon. However, there is one area in which it is lacking: the UI.

While the UI is modern and represents Gopher content in an easily-approachable way, it is not well-engineered for performance and stability.

The refactor aims to solve these issues with some key improvements, namely:
* All GTK rendering code should be fully separated from the network and parsing code. This will reduce occurrences of memory-related issues and eliminate bugs related to passing GTK handles across threads.
* All network requests required to get the information about and display a resource should be done before the UI starts rendering. This may be revisited later, but it allows the UI to remain strictly single-threaded, as GTK is not thread-safe.
* In order to achieve this, the existing menu data structure will need to be revised. An element may contain an arbitrarily-sized resource buffer, containing data downloaded as the menu was parsed.

After further review, it was determined that these refactors are not actually necessary-- the real issue was incorrect function signatures being used with `g_idle_add`. A function that properly returns `false` when it has completed does not trigger this issue.