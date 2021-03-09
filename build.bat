@echo off
cl /nologo /W3 /WX /FC /Z7 src\main.c /link /incremental:no
del *.ilk *.obj