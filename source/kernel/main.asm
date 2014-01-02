include 'loonyvm.inc'

org 0x10000

entryPoint:
	invoke_va printf, msgHelloWorld
	jmp $

msgHelloWorld: db 'hello world from kernel!!!!', 0

include 'lib/string.asm'
include 'lib/term.asm'
include 'lib/printf.asm'
