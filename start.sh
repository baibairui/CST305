#!/bin/bash

# 设置编译选项
CFLAGS="-I/opt/X11/include -DGL_SILENCE_DEPRECATION"
LDFLAGS="-L/opt/X11/lib -lglut -framework OpenGL"

# 定义源文件目录和目标目录
SRC_DIR=$(pwd)"/src"
BIN_DIR=$(pwd)"/bin"

# 如果目标目录不存在，创建它
if [ ! -d "$BIN_DIR" ]; then
    mkdir -p "$BIN_DIR"
fi

# 定义最终可执行文件的名称
FINAL_EXE="out"

# 清空之前的编译命令（如果有）
rm -f "$BIN_DIR/$FINAL_EXE"

# 收集所有的 .cpp 文件
CPP_FILES=""
for file in "$SRC_DIR"/*.cpp; do
    CPP_FILES="$CPP_FILES $file"
done

# 编译所有的 .cpp 文件，生成一个最终的可执行文件
echo "Compiling all .cpp files into one executable..."
g++ $CPP_FILES -o "$BIN_DIR/$FINAL_EXE" $CFLAGS $LDFLAGS 
"$BIN_DIR/$FINAL_EXE"