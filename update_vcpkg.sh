#!/bin/bash

git -C vcpkg pull origin master
hash=$(git -C vcpkg rev-parse HEAD)
sed -i "s/\"builtin-baseline\": \".*\"/\"builtin-baseline\": \"$hash\"/" vcpkg.json
