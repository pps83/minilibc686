;
; based on .nasm source file generated by soptcc.pl from c_stdio_medium_ftell.c
; Compile to i386 ELF .o object: nasm -O999999999 -w+orphan-labels -f elf -o c_stdio_medium_ftell.o c_stdio_medium_ftell.nasm
;

bits 32
cpu 386

global mini_ftell
%ifdef CONFIG_SECTIONS_DEFINED
%elifidn __OUTPUT_FORMAT__, bin
section .text align=1
section .rodata align=1
section .data align=1
section .bss align=1
%else
section .text align=1
section .rodata align=1
section .data align=1
section .bss align=1
%endif

section .text

mini_ftell:
		mov edx, [esp+0x4]
		mov cl, [edx+0x14]
		lea eax, [ecx-0x1]
		cmp al, 0x2
		ja .2
		mov eax, [edx+0x8]
		jmp short .3
.2:		or eax, byte -1
		cmp cl, 0x3
		jbe .1
		mov eax, [edx]
.3:		sub eax, [edx+0x18]
		add eax, [edx+0x20]
.1:		ret

%ifdef CONFIG_PIC  ; Already position-independent code.
%endif

; __END__
