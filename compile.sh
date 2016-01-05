#!/bin/sh
DEBUG_OPTIONS="-g -fsanitize=undefined -DDEBUG -D_GLIBCXX_DEBUG"
RELEASE_OPTIONS="-O3 -DNDEBUG"
g++ -std=c++11 -Wall   $DEBUG_OPTIONS a.cpp samurai.cpp || \
g++ -std=c++11 -Wall $RELEASE_OPTIONS a.cpp samurai.cpp
