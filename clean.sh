#!/bin/bash

rm -rf build/*
rm -rf lib/*
rm -rf bin/p*
rm -r CMakeFiles
rm cmake_install.cmake
rm Makefile
rm CMakeCache.txt

rm -r src/CMakeFiles
rm

find . -name 'Makefile'  -print -exec rm -rfv {} \;
find . -name 'cmake_install.cmake'  -print -exec rm -rfv {} \;
find . -name 'RUST'  -print -exec rm -rfv {} \;
find . -name 'CMakeFiles'  -print -exec rm -rfv {} \;
find . -name '.DS_Store'  -print -exec rm -rfv {} \;
find . -name '*~'  -print -exec rm -rfv {} \;
find . -name '#*'  -print -exec rm -rfv {} \;
find . -name '*.moos++'  -print -exec rm -rfv {} \;

#find . -name 'MOOSLog*'  -print -exec rm -rfv {} \;

