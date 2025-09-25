#!/bin/bash

build_path="./build"
build_type="Debug"

if [ "$1" == "release" ]; then
    build_path="./build-release"
    build_type="Release"
fi

cmake -S . -B $build_path -DCMAKE_BUILD_TYPE=$build_type | cat && cmake --build $build_path -j | cat