#!/bin/bash

g++ ./src/cpp/ministo.cpp ./src/cpp/hybrid_ministo.cpp ./src/cpp/cpu/sha3.c ./src/cpp/cpu/solver.cpp -std=c++11 -O3 -o ./bin/ministo
