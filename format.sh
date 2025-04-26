#!/bin/bash

# clang-format批量格式化脚本
# 用法: ./format.sh [目录] [clang-format配置文件]
# 示例: ./format.sh ./src ~/my_project/.clang-format

# 检查参数
if [ $# -lt 1 ]; then
    echo "用法: $0 [目录] [clang-format配置文件(可选)]"
    exit 1
fi

TARGET_DIR=$1
CLANG_FORMAT_FILE=${2:-".clang-format"}  # 如果未指定第二个参数，则使用当前目录的.clang-format

# 检查clang-format是否安装
if ! command -v clang-format &> /dev/null; then
    echo "错误: clang-format 未安装。请先安装 clang-format。"
    exit 1
fi

# 检查目标目录是否存在
if [ ! -d "$TARGET_DIR" ]; then
    echo "错误: 目录 $TARGET_DIR 不存在"
    exit 1
fi

# 检查.clang-format文件是否存在
if [ ! -f "$CLANG_FORMAT_FILE" ]; then
    echo "警告: clang-format配置文件 $CLANG_FORMAT_FILE 不存在，将使用clang-format默认样式"
    STYLE_ARG=""
else
    STYLE_ARG="--style=file:$CLANG_FORMAT_FILE"
    echo "使用配置文件: $CLANG_FORMAT_FILE"
fi

# 支持的代码文件扩展名
EXTENSIONS=("cpp" "hpp" "h" "c" "cc" "cxx" "hxx")

# 构建find命令的-name参数
NAME_ARGS=()
for ext in "${EXTENSIONS[@]}"; do
    NAME_ARGS+=(-name "*.$ext")
    if [ "$ext" != "${EXTENSIONS[-1]}" ]; then
        NAME_ARGS+=(-o)
    fi
done

# 查找并格式化文件
echo "开始在目录 $TARGET_DIR 中格式化代码..."
find "$TARGET_DIR" -type f \( "${NAME_ARGS[@]}" \) -print0 | while IFS= read -r -d '' file; do
    echo "格式化: $file"
    clang-format -i $STYLE_ARG "$file"
done

echo "格式化完成!"