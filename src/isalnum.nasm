;
; written by pts@fazekas.hu at Sun May 21 16:39:01 CEST 2023
; Compile to i386 ELF .o object: nasm -O999999999 -w+orphan-labels -f elf -o isalnum.o isalnum.nasm
;
; Uses: %ifdef CONFIG_PIC
;

bits 32
cpu 386

global mini_isalnum
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
mini_isalnum:  ; int mini_isalnum(int c);
		mov al, [esp+4]
		sub al, '0'
		cmp al, '9'-'0'+1
		jc .found
		add al, '0'
		or al, 0x20
		sub al, 'a'
		cmp al, 'z'-'a'+1
.found		sbb eax, eax
		neg eax
		ret

%ifdef CONFIG_PIC  ; Already position-independent code.
%endif

; __END__
