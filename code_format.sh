#!/bin/bash
CLANG_FORMAT_PATH=$(which clang-format)

 if [ -z "$CLANG_FORMAT_PATH" ]; then
    echo "clang-format not found."
    exit 1
fi

PROJECT_ROOT=$(dirname "$0")

cpp_files=$(find $PROJECT_ROOT -name '*.cpp' -or -name '*.h' )

for file in $cpp_files; do
    echo "Formatting $file"
    $CLANG_FORMAT_PATH -i $file
done

echo "All .cpp or .h files have been formatted."
