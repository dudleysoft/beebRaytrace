@echo off
beebasm -i beebCode.asm
c:\cygwin64\bin\xxd -i beebCode.bin >beebCode.c
c:\cygwin64\bin\xxd -i bagiCode.bin >bagiCode.c
