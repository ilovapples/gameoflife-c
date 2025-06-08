@echo off
del life.exe
gcc -Wall -Wextra -Werror -o life life.c -O1
life.exe %*
