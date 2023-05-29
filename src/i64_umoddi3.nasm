;
; Compile to i386 ELF .o object: nasm -O999999999 -w+orphan-labels -f elf -o i64_umoddi3.o i64_umoddi3.nasm
;
; Uses: %ifdef CONFIG_PIC
;

bits 32
cpu 386

global __umoddi3
%ifdef CONFIG_SECTIONS_DEFINED
%elifidn __OUTPUT_FORMAT__, bin
section .text align=1
section .rodata align=4
section .data align=4
section .bss align=4
__U8D equ +0x12345678
%else
extern __U8D
section .text align=1
section .rodata align=1
section .data align=1
section .bss align=1
%endif

section .text
__umoddi3:  ; Used the __cdecl (GCC __regparm__(0)) calling convention.
		push ebx
		mov eax, [esp+0x8]
		mov edx, [esp+0xc]
		mov ebx, [esp+0x10]	; Low half of the divisor.
		mov ecx, [esp+0x14]	; High half of the divisor.
		call __U8D
		xchg eax, ebx		; EAX := low half of the modulo, EBX := junk.
		mov edx, ecx
		pop ebx
		ret

%ifdef CONFIG_PIC  ; Already position-independent code.
%endif

; __END__
