#!/bin/sh
exec g++ -std=c++14 -Wall -g -fsanitize=undefined -DDEBUG -D_GLIBCXX_DEBUG a.cpp samurai.cpp
