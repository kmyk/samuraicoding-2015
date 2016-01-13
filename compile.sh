#!/bin/sh
RELEASE_OPTIONS="-O3 -DNDEBUG"
exec g++ -std=c++11 -Wall $RELEASE_OPTIONS a.cpp samurai.cpp
