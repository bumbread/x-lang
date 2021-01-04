@echo off
cl /nologo /FC /Z7 src\main.c /link /incremental:no
del *.ilk *.obj