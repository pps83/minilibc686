;
; written by pts@fazekas.hu at Tue Apr  9 21:17:17 CEST 2024
; Compile to i386 ELF .o object: nasm -O999999999 -w+orphan-labels -f elf -o strncat.o strncat.nasm
;
; Code size: 0x26 bytes (optimized).
;
; Uses: %ifdef CONFIG_PIC
;

bits 32
cpu 386

global mini_strncat
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
mini_strncat:  ; char *mini_strncat(char *dest, const char *src, size_t n);
		mov ecx, [esp+0xc]  ; Argument n.
.in_ecx:  ; TODO(pts): Make mini_strcat(...) call mini_strncat(?, ?, -1) as mini_strncat.in_ecx from smart.nasm if both functions are present.
		push edi  ; Save.
		mov edi, [esp+8]  ; Argument dest.
		mov edx, [esp+0xc]  ; Argument src.
		push edi  ; Will use as  return value.
		xor eax, eax
.0:		scasb
		jne short .0
		dec edi
.1:		xor eax, eax
		test ecx, ecx
		jz short .3
		mov al, [edx]
		inc edx
.3:		stosb
		dec ecx
		test al, al
		jnz short .1
.2:		pop eax  ; Result: pointer to dest.
		pop edi  ; Restore.
		ret

%ifdef CONFIG_PIC  ; Already position-independent code.
%endif

; __END__
