#!/bin/sh
DEBUG_OPTIONS="-g -fsanitize=undefined -DDEBUG -D_GLIBCXX_DEBUG"
exec g++ -std=c++11 -Wall $DEBUG_OPTIONS a.cpp samurai.cpp
