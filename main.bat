
echo removing previous build...
rem main

echo compiling...
clang++ src/*.cpp -o main.exe

echo running...
cls
main.exe