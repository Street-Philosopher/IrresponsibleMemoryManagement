cls
@REM clang++ -I dependencies\SFML-2.5.1\include src\*.cpp
msvc /O2 /Fe: IMM.exe /I C:\c-libs\SFML-2.5.1\include /I "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include" /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt" src\*.cpp
