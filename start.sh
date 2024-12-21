#!/bin/bash

# 设置编译选项（使用 macOS 原生 OpenGL 和 GLUT 框架）
CFLAGS="-DGL_SILENCE_DEPRECATION"
LDFLAGS="-framework OpenGL -framework GLUT"

# 定义源文件目录和目标目录
SRC_DIR=$(pwd)/src
BIN_DIR="${SRC_DIR}/bin"

# 如果目标目录不存在，创建它
if [ ! -d "$BIN_DIR" ]; then
    mkdir -p "$BIN_DIR"
fi

# 定义最终可执行文件的名称
FINAL_EXE="out"

# 收集所有的 .cpp 文件
CPP_FILES=""
for file in "$SRC_DIR"/*.cpp; do
    if [ -f "$file" ]; then
        CPP_FILES="$CPP_FILES $file"
    fi
done

# 如果没有找到任何 .cpp 文件，退出并提示
if [ -z "$CPP_FILES" ]; then
    echo "Error: No .cpp files found in $SRC_DIR."
    exit 1
fi

# 清除之前的目标文件（如果存在）
if [ -f "$BIN_DIR/$FINAL_EXE" ]; then
    rm -f "$BIN_DIR/$FINAL_EXE"
fi

# 编译所有的 .cpp 文件
echo "Compiling all .cpp files into one executable..."
g++ -std=c++11 $CPP_FILES -o "$BIN_DIR/$FINAL_EXE" $CFLAGS $LDFLAGS

# 检查编译是否成功
if [ $? -ne 0 ]; then
    echo "Error: Compilation failed. Please check your code or dependencies."
    exit 1
fi

# 执行最终生成的文件
echo "Compilation successful. Executing the program..."
"$BIN_DIR/$FINAL_EXE"