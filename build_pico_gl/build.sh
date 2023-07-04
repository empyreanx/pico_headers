#!/bin/bash

cd "${0%/*}"

mkdir -p tmp

sed -e '/#include "khrplatform.h"/ {' -e 'r ./src/pico_gl_types.h' -e 'd' -e '}' ./glad/glad.h > ./tmp/glad.h
sed -e '/#include "glad.h"/ {' -e 'r ./tmp/glad.h' -e 'd' -e '}' ./src/pico_gl.h > ./tmp/pico_gl1.h
sed -e '/#include "glad.c"/ {' -e 'r ./glad/glad.c' -e 'd' -e '}' ./tmp/pico_gl1.h > ./tmp/pico_gl2.h
sed 's/khronos/pgl/g' ./tmp/pico_gl2.h > ./tmp/pico_gl3.h
sed 's/#include "glad.h"//g' ./tmp/pico_gl3.h > ./tmp/pico_gl4.h


dt=$(date '+%d/%m/%Y %H:%M:%S');

header="///=============================================================================\n"
header="$header/// WARNING: This file was automatically generated on $dt.\n"
header="$header/// DO NOT EDIT!\n"
header="$header///============================================================================\n"

echo -e $header > ../pico_gl.h
cat ./tmp/pico_gl4.h >> ../pico_gl.h

rm -rf ./tmp
