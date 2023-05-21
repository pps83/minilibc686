;
; written by pts@fazekas.hu at Sun May 21 16:39:01 CEST 2023
; Compile to i386 ELF .o object: nasm -O999999999 -w+orphan-labels -f elf -o memcpy.o memcpy.nasm
;
; Uses: %ifdef CONFIG_PIC
;

bits 32
cpu 386

global mini_memcpy
%ifdef CONFIG_SECTIONS_DEFINED
%elifidn __OUTPUT_FORMAT__, bin
section .text align=1
section .rodata align=4
section .data align=4
section .bss align=4
%else
section .text align=1
section .rodata align=1
section .data align=1
section .bss align=1
%endif

section .text
mini_memcpy:  ; void *mini_memcpy(void *dest, const void *src, size_t n);
		push edi
		push esi
		mov ecx, [esp+0x14]
		mov esi, [esp+0x10]
		mov edi, [esp+0xc]
		push edi
		rep movsb
		pop eax  ; Result: pointer to dest.
		pop esi
		pop edi
		ret

%ifdef CONFIG_PIC  ; Already position-independent code.
%endif

; __END__
