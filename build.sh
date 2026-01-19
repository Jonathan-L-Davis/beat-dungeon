#!/usr/bin/bash

# If I ever need to make multiple exe's for a project, it should be easy enough to make this a function. Simply make arg1 the exe name, and arg2 the array name.
# I'd probably need to add parameters for including and linking libraries though. Not a big deal, but a complication nonetheless.

Executable="beat-dungeon"
Source_Files=(
    "main.cpp"
    
    "engine/draw.cpp" "engine/init.cpp" "engine/input.cpp" "engine/update.cpp"
    "engine/io/keyboard.cpp" "engine/io/mouse.cpp"
    
    "graphics/graphics.cpp"
    "graphics/STL/STL.cpp"
    "graphics/imgui/imgui.cpp" "graphics/imgui/imgui_demo.cpp" "graphics/imgui/imgui_draw.cpp" "graphics/imgui/imgui_tables.cpp" "graphics/imgui/imgui_widgets.cpp"
    "graphics/imgui/imgui_impl_sdl3.cpp" "graphics/imgui/imgui_impl_vulkan.cpp"
    "graphics/vulkan/shader.cpp" "graphics/vulkan/vulkan.cpp"
    
    "util/xml/Markup.cpp" "util/file/file.cpp" "util/stb/stb_image.cpp"
    
    "game/beat-dungeon.cpp"
)

Object_Files=()

mkdir -p obj
for file in "${Source_Files[@]}" ; do
    
    if [[ $file =~ [[:space:]]+ ]] ; then
        echo "File path: \"$file\" Contains a space and should not. Build script does not support spaces in paths. Bash handles that poorly."
        exit 1
    fi
    
    object_file="obj/src/$(echo $file | sed -e 's/\.cpp/\.o/')"
    file="src/$file"
    
    path="$(dirname $file)"
    Object_Files+=( "${object_file}" )
    mkdir -p "obj/$path" # otherwise clang++/g++ complain about non-existing directory
    zig c++ "$file" -g -o "${object_file}" -std=c++23 -O0 -c -Isrc -I/usr/include
done

for object in "${Object_Files[@]}" ; do
    file_glob+=" $object"
done

mkdir -p gen
zig c++ $file_glob -o "gen/${Executable}" -Wl,-rpath="\$ORIGIN" -L/usr/local/lib -lSDL3 -lX11 -lvulkan -fsanitize=address

mkdir -p res/shaders
glslc src/shader/vertex/shader.vert -o res/shaders/vert.spv
glslc src/shader/fragment/shader.frag -o res/shaders/frag.spv

cp -r res gen
cp config.xml gen

cp -r levels gen/
