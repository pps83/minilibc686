;
; based on .nasm source file generated by soptcc.pl from c_stdio_medium_fputc_rp2.c
; Compile to i386 ELF .o object: nasm -O999999999 -w+orphan-labels -f elf -o c_stdio_medium_fputc_rp2.o c_stdio_medium_fputc_rp2.nasm

bits 32
cpu 386

global mini___M_fputc_RP2
%ifdef CONFIG_SECTIONS_DEFINED
%elifidn __OUTPUT_FORMAT__, bin
section .text align=1
section .rodata align=1
section .data align=1
section .bss align=1
mini_write equ +0x12345678
mini_fflush equ +0x12345679
%else
extern mini_write
extern mini_fflush
section .text align=1
section .rodata align=1
section .data align=4
section .bss align=4
%endif

section .text

mini___M_fputc_RP2:
		push ebx
		mov ebx, edx
		push ecx
		mov [esp+0x3], al  ; Local variable uc.
		mov eax, [edx+0x4]
		cmp [edx], eax
		jne .16
		push edx
		call mini_fflush
		pop edx
		test eax, eax
		je .17
.20:		or eax, byte -0x1
		jmp short .14
.17:		mov eax, [ebx+0x4]
		cmp [ebx], eax
		jne .16
		push byte 1
		lea eax, [esp+0x7]  ; Local variable uc.
		push eax
		push dword [ebx+0x10]
		call mini_write
		add esp, byte 0xc
		dec eax
		jne .20
		inc dword [ebx+0x20]
.16:		mov eax, [ebx]
		lea edx, [eax+0x1]
		mov [ebx], edx
		mov dl, [esp+0x3]  ; Local variable uc.
		mov [eax], dl
		cmp byte [esp+0x3], 0xa  ; Local variable uc.
		jne .21
		cmp byte [ebx+0x14], 0x6
		jne .21
		push ebx
		call mini_fflush
		pop ecx
.21:		movzx eax, byte [esp+0x3]  ; Local variable uc.
.14:		pop edx
		pop ebx
		ret

%ifdef CONFIG_PIC  ; Already position-independent code.
%endif

; __END__
