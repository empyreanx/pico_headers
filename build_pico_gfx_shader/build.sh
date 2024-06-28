#!/bin/bash

shopt -s nullglob
for f in *.glsl; do
	echo "Compiling $f to ${f%%.*}_shader.h"
	./sokol-shdc --input "$f" --output "${f%%.*}_shader.h" --format=sokol_impl --slang glsl410:hlsl5:metal_macos:metal_ios:metal_sim:glsl300es --reflection --ifdef
done
