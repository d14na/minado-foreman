#!/bin/bash

g++ ./minadod/ministo.cpp ./minadod/hybrid_ministo.cpp ./minadod/cpu/sha3.c ./minadod/cpu/solver.cpp -std=c++11 -O3 -o ./bin/minadod
