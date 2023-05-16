;
; written by pts@fazekas.hu at Tue May 16 13:56:57 CEST 2023
; Compile to i386 ELF .o object: nasm -O999999999 -w+orphan-labels -f elf -o start_linux.o start_linux.nasm
;
; Uses: %ifdef CONFIG_PIC
;

bits 32
cpu 386

global mini__start
global mini_exit
%ifdef CONFIG_SECTIONS_DEFINED
%elifidn __OUTPUT_FORMAT__, bin
section .text align=1
section .rodata align=4
section .data align=4
section .bss align=4
main equ +0x12345678
%else
extern main
section .text align=1
section .rodata align=1
section .data align=1
section .bss align=1
%endif

section .text
mini__start:  ; Entry point (_start) of the Linux i386 executable.
		; Now the stack looks like (from top to bottom):
		;   dword [esp]: argc
		;   dword [esp+4]: argv[0] pointer
		;   esp+8...: argv[1..] pointers
		;   NULL that ends argv[]
		;   environment pointers
		;   NULL that ends envp[]
		;   ELF Auxiliary Table
		;   argv strings
		;   environment strings
		;   program name
		;   NULL		
		pop eax  ; argc.
		mov edx, esp  ; argv.
		lea ecx, [edx+eax*4+4]  ; envp.
		push ecx  ; Argument envp for main.
		push edx  ; Argument argv for main.
		push eax  ; Argument argc for main.
		call main  ; Return value (exit code) in EAX (AL).
		; times 2 pop edx  ; No need to clean up the stack.
		xor ebx, eax  ; EBX := Exit code; EAX := junk.
		; TODO(pts): Call fflush(stdout) etc.
		jmp strict short mini_exit.ebx
mini_exit:  ; void mini_exit(int exit_code);
		; !! envp.
		mov ebx, [esp+4]  ; EBX := Exit code.
.ebx:		xor eax, eax
		inc eax  ; EAX := 1 == __NR_exit.
		int 0x80  ; Linux i386 syscall, exit(2) doesn't return.
		; Not reached, the syscall above doesn't return.

%ifdef CONFIG_PIC  ; Already position-independent code.
%endif

; __END__
