@echo off
cls

cl src/*.c /O2 /D _CRT_SECURE_NO_WARNINGS /D WIN32_LEAN_AND_MEAN /W4 /WX /wd4701 /wd4703 /wd4706 /wd4201 /std:c17 /TC /diagnostics:caret /link /incremental:no /opt:ref /subsystem:console /out:elefont.exe
del "*.obj" /Q
