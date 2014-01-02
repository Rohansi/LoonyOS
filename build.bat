@echo off
if not defined include set include=include

tools\fasm source\bios.asm                  bin\bios.bin
tools\fasm source\bootloader\boot.asm       bin\boot.bin
tools\fasm source\kernel\main.asm           bin\kernel.bin

tools\dskimg install.lua
