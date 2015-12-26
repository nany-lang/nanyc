nany {c++/bootstrap}
====================

C++/libyuni nany compiler for bootstrapping the Nany Compiler



Steps for compiling on UNIXes (and MinGW)
-----------------------------------------

Prerequisites:

 * CMake (>= 2.8.10)
 * C++ Toolchain: make, c++14 compiler (gcc >= 4.8, clang...)

Steps [debud mode]:

 * `cmake .`
 * `make -j 8`

Steps [release mode]:
 * `cmake . -DCMAKE_BUILD_TYPE=release`
 * `make -j 8`



Steps for compiling on MS-WIndows
---------------------------------

Prerequisites:

 * CMake for Windows
 * Visual Studio (>=2015)

Steps [debug mode]:

 * `cmake . -G "Visual Studio 14 2015"`  # or "Visual Studio 14 2015 Win64"
 * `msbuild nany-bootstrap.sln`

Steps [release mode]:

 * `cmake . -G "Visual Studio 14 2015" -DCMAKE_BUILD_TYPE=release`
 * `msbuild nany-bootstrap.sln`



Static mode
-----------

Nany can be compiled in static:

 * `cmake . -DNANY_STATIC=1`
