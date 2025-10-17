#!/bin/bash
set -E
set -x
g++ -c -fPIC bin/Dynamic_Functions.cpp -o bin/Dynamic_Functions.o
g++ -shared bin/Dynamic_Functions.o -o bin/libDynamic_Functions.so
g++ FShell.cpp -o FShell -Lbin -lDynamic_Functions -ldl -Wl,-rpath='$ORIGIN/bin'
g++ -c -fPIC bin/MountFS.cpp -o bin/MountFS.o
g++ -shared bin/MountFS.o -Lbin -lDynamic_Functions -o bin/MountFS.so
g++ -c -fPIC bin/API_Functions.cpp -o bin/API_Functions.o
g++ -shared bin/API_Functions.o -Lbin -lDynamic_Functions -o bin/API_Functions.so
g++ -c -fPIC bin/CreateMetaData.cpp -o bin/CreateMetaData.o
g++ -shared bin/CreateMetaData.o -Lbin -lDynamic_Functions -o bin/CreateMetaData.so
g++ -c -fPIC bin/FreeSpaceManagement.cpp -o bin/FreeSpaceManagement.o
g++ -shared bin/FreeSpaceManagement.o -Lbin -lDynamic_Functions -o bin/FreeSpaceManagement.so
g++ -c -fPIC bin/GenerateFiles.cpp -o bin/GenerateFiles.o
g++ -shared bin/GenerateFiles.o -Lbin -lDynamic_Functions -o bin/GenerateFiles.so
g++ -c -fPIC bin/GenerateDirectories.cpp -o bin/GenerateDirectories.o
g++ -shared bin/GenerateDirectories.o -Lbin -lDynamic_Functions -o bin/GenerateDirectories.so
g++ -c -fPIC bin/LowLevelEncoding.cpp -o bin/LowLevelEncoding.o
g++ -shared bin/LowLevelEncoding.o -Lbin -lDynamic_Functions -o bin/LowLevelEncoding.so
g++ -c -fPIC bin/LowLevelDecoding.cpp -o bin/LowLevelDecoding.o
g++ -shared bin/LowLevelDecoding.o -Lbin -lDynamic_Functions -o bin/LowLevelDecoding.so
rm bin/MountFS.o
rm bin/API_Functions.o
rm bin/CreateMetaData.o
rm bin/FreeSpaceManagement.o
rm bin/GenerateFiles.o
rm bin/LowLevelDecoding.o
rm bin/LowLevelEncoding.o
rm bin/GenerateDirectories.o
rm bin/Dynamic_Functions.o
