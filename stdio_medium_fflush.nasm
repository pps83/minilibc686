;
; based on .nasm source file generated by soptcc.pl from c_stdio_medium_fflush.c
; Compile to i386 ELF .o object: nasm -O999999999 -w+orphan-labels -f elf -o c_stdio_medium_fflush.o c_stdio_medium_fflush.nasm
;

bits 32
cpu 386

global mini_fflush
%ifdef CONFIG_SECTIONS_DEFINED
%elifidn __OUTPUT_FORMAT__, bin
section .text align=1
section .rodata align=1
section .data align=1
section .bss align=1
mini_write equ +0x12345678
mini___M_discard_buf equ +0x12345679
%else
extern mini_write
extern mini___M_discard_buf
section .text align=1
section .rodata align=1
section .data align=1
section .bss align=1
%endif

section .text

mini_fflush:
		push esi
		or ecx, byte -0x1
		push ebx
		mov ebx, [esp+0xc]
		cmp byte [ebx+0x14], 0x3
		jbe .4
		mov esi, [ebx+0x18]
.6:		mov eax, [ebx]
		cmp eax, esi
		je .13
		sub eax, esi
		push eax
		push esi
		push dword [ebx+0x10]
		call mini_write
		add esp, byte 0xc
		lea edx, [eax+0x1]
		cmp edx, byte 0x1
		jbe .10
		add esi, eax
		jmp short .6
.13:		xor ecx, ecx
		jmp short .7
.10:		or ecx, byte -0x1
.7:		sub esi, [ebx+0x18]
		add [ebx+0x20], esi
		push ebx
		call mini___M_discard_buf
		pop eax
.4:		pop ebx
		mov eax, ecx
		pop esi
		ret

%ifdef CONFIG_PIC  ; Already position-independent code.
%endif

; __END__
