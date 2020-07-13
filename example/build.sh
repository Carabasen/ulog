#!/bin/bash
set -e

g++ -std=c++17 -lstdc++fs -Werror -Wpedantic -Wall -Wextra -O3 -pthread -o ulog_example ../ulog.cpp ulog_example.cpp

#./ulog_example

