#!/bin/sh
./build/rx $1 $2 $3 | sh | ./build/tx $2 $1 $3