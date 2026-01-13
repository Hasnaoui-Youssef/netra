#!/bin/bash

TARGET_DIR="${1:-.}"

if [[ ! -d "$TARGET_DIR" ]] then
    exit 1
fi

find "$TARGET_DIR" -name "*.hpp" -type f | while read -r file; do
    filename=$(basename "$file" .hpp)
    guard_name=$(echo "$filename" | tr '[:lower:]' '[:upper:]')
    full_guard="__${guard_name}_H__"
    if [[ ! -s "$file" ]] || [[ -z "$(tr -d '[:space:]' < "$file")" ]]; then
        cat > "$file" << EOF
#ifndef $full_guard
#define $full_guard

#endif // $full_guard
EOF
    fi
done
