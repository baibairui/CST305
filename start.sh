#!/bin/bash

CFLAGS="-DGL_SILENCE_DEPRECATION"
LDFLAGS="-framework OpenGL -framework GLUT"

SRC_DIR=$(pwd)/src
BIN_DIR="$(pwd)/bin"

if [ ! -d "$BIN_DIR" ]; then
    mkdir -p "$BIN_DIR"
fi

FINAL_EXE="out"


CPP_FILES=""
for file in "$SRC_DIR"/*.cpp; do
    if [ -f "$file" ]; then
        CPP_FILES="$CPP_FILES $file"
    fi
done

if [ -z "$CPP_FILES" ]; then
    echo "Error: No .cpp files found in $SRC_DIR."
    exit 1
fi

if [ -f "$BIN_DIR/$FINAL_EXE" ]; then
    rm -f "$BIN_DIR/$FINAL_EXE"
fi

echo "Compiling all .cpp files into one executable..."
g++ -std=c++11 $CPP_FILES -o "$BIN_DIR/$FINAL_EXE" $CFLAGS $LDFLAGS

if [ $? -ne 0 ]; then
    echo "Error: Compilation failed. Please check your code or dependencies."
    exit 1
fi

echo "Compilation successful. Executing the program..."
"$BIN_DIR/$FINAL_EXE"