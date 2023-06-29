;
; based on disassemby from libgcc.a of GCC 7.5.0
; Compile to i386 ELF .o object: nasm -O999999999 -w+orphan-labels -f elf -o float_chp.o float_chp.nasm
;
; The following libgcc.a versions were tried, and the shortest code was
; selected: 4.4, 4.6, 4.8, 7.5.0, 8, 9, 10, 11. C source cxmuldiv.c in
; pcc-libs-1.1.0.tgz was also tried with soptcc.pl.
;
; TODO(pts): Implement all 3 sizes in terms of `long double'. Just adapt the stack.
;
; Uses: %ifdef CONFIG_PIC
; Uses: %ifdef CONFIG_I386
;

bits 32
%ifdef CONFIG_I386
cpu 386
%else
cpu 686
%endif

global __divxc3
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

%ifdef CONFIG_I386  ; Emulate the missing i686 instructions using i386 instructions.
  %macro _fucomip 2
    fucomp %1, %2
    fnstsw ax
    sahf
  %endmacro
  %macro _fucomi 2
    fucom %1, %2
    fnstsw ax
    sahf
  %endmacro
  %macro _cmovne 2
    je %%skip
    mov %1, %2
    %%skip:
  %endmacro
%else
  %define _fucomip fucomip
  %define _fucomi fucomi
  %define _cmovne cmovne
%endif  ; CONFIG_I386

section .text
; For PCC and GCC >= 4.3.
__divxc3:  ; long double _Complex __divxc3(long double a, long double b, long double c, long double d);
; Returns: the quotient of (a + ib) / (c + id).
		push esi
		push ebx
		fld tword [esp+0x10]
		fld tword [esp+0x1c]
		fld tword [esp+0x28]
		fld tword [esp+0x34]
		fld st1
		fabs
		fld st1
		fabs
		_fucomip st0, st1
		fstp st0
		jbe .8
		fld st1
		fdiv st0, st1
		fld st2
		fmul st0, st1
		fadd st0, st2
		fld st5
		fmul st0, st2
		fadd st0, st5
		fdiv st0, st1
		fxch st2
		fmul st0, st5
		fsub st0, st6
		fdivrp st1, st0
.1:		_fucomi st0, st0
		jp .9
		fstp st5
		fstp st3
		jmp short .2b
.2:		fstp st4
		fstp st4
.2b:		fstp st0
		fstp st0
		jmp short .7
.3:		fstp st0
.4:		fstp st0
.5:		fstp st0
		fstp st3
		fstp st0
		fstp st0
		fxch st1
.7:		mov eax, [esp+0xc]  ; Struct return pointer. We return it in EAX according to the ABI.
		fstp tword [eax]
		fstp tword [eax+0xc]
		pop ebx
		pop esi
		ret 4
.8:		fld st0
		fdiv st0, st2
		fld st1
		fmul st0, st1
		fadd st0, st3
		fld st1
		fmul st0, st5
		fadd st0, st6
		fdiv st0, st1
		fxch st2
		fmul st0, st6
		fsubr st0, st5
		fdivrp st1, st0
		jmp short .1
.9:		fxch st1
		_fucomi st0, st0
		jpo .2
		fxch st5
		_fucomi st0, st0
		mov esi, 0
		fldz
		setpo bl
		_fucomi st0, st4
		setpo al
		_cmovne eax, esi
		test al, al
		je .13
		_fucomip st0, st3
		setpo al
		_cmovne eax, esi
		test al, al
		je .14
		fxch st4
		_fucomi st0, st0
		jp near .29
		fstp st5
		fstp st0
		fstp st0
		fxch st1
		fxch st2
		fxch st1
		jmp short .11
.10:		fstp st5
		fstp st0
		fstp st0
		fxch st1
		fxch st2
		fxch st1
.11:		fxam
		fnstsw ax
		fstp st0
		test ah, 2
		push dword 0x7f800000
		fld dword [esp]
		pop ecx
		je .12
		fstp st0
		push dword 0xff800000
		fld dword [esp]
		pop ecx
.12:		fmul st2, st0
		fmulp st1, st0
		fxch st1
		jmp near .7
.13:		fstp st0
.14:		fld st0
		fsub st0, st1
		_fucomi st0, st0
		setp al
		and bl, al
		jne near .30
.15:		fld st5
		fsub st0, st6
		fxch st6
		_fucomi st0, st0
		jp .16
		fxch st6
		_fucomip st0, st0
		jp near .30
		jmp short .17
.16:		fstp st6
.17:		fld st4
		fsub st0, st5
		fxch st5
		jmp short .19
.18:		fxch st5
.19:		_fucomi st0, st0
		fxch st5
		setpo al
		_fucomip st0, st0
		setp bl
		and al, bl
		jne .22
		fld st3
		fsub st0, st4
		fxch st4
		jmp short .21
.20:		fxch st4
.21:		_fucomi st0, st0
		jp .3
		fxch st4
		_fucomip st0, st0
		jpo .4
		xor eax, eax
.22:		_fucomip st0, st0
		jp .5
		fld st4
		fsub st0, st5
		_fucomip st0, st0
		jp .5
		fstp st5
		fstp st0
		fxch st1
		fxch st2
		fxch st3
		test al, al
		jne near .37
		fldz
		fxch st3
.23:		fxam
		fnstsw ax
		fstp st0
		fxch st2
		test ah, 2
		fabs
		je .24
		fchs
.24:		fld st1
		fsub st0, st2
		fxch st2
		_fucomi st0, st0
		jp .25
		fxch st2
		_fucomip st0, st0
		jp near .39
		jmp short .26
.25:		fstp st2
.26:		fldz
		fxch st2
.27:		fxam
		fnstsw ax
		fstp st0
		fxch st1
		test ah, 2
		fabs
		je .28
		fchs
.28:		fld st2
		fmul st0, st2
		fld st4
		fmul st0, st2
		faddp st1, st0
		fldz
		fmul st1, st0
		fxch st5
		fmulp st3, st0
		fxch st1
		fmulp st3, st0
		fxch st2
		fsubp st1, st0
		fmulp st2, st0
		jmp near .7
.29:		test bl, bl
		jne .10
		fld st4
		xor ebx, ebx
		fsub st0, st5
		fxch st1
		fxch st5
		fxch st1
		jmp near .15
.30:		fld st4
		fsub st0, st5
		_fucomi st0, st0
		jp .18
		fstp st0
		fld st3
		fsub st0, st4
		_fucomi st0, st0
		jp .20
		fstp st0
		fstp st0
		fstp st5
		fstp st0
		fxch st1
		fxch st2
		fxch st3
		test bl, bl
		jne .38
		fldz
		fxch st1
.31:		fxam
		fnstsw ax
		fstp st0
		test ah, 2
		fabs
		je .32
		fchs
.32:		fld st3
		fsub st0, st4
		fxch st4
		_fucomi st0, st0
		jp .33
		fxch st4
		_fucomip st0, st0
		jp .40
		jmp short .34
.33:		fstp st4
.34:		fldz
		fxch st4
.35:		fxam
		fnstsw ax
		fstp st0
		fxch st3
		test ah, 2
		fabs
		je .36
		fchs
.36:		fld st2
		fmul st0, st4
		fld st2
		fmul st0, st2
		faddp st1, st0
		push dword 0x7f800000
		fld dword [esp]
		pop ecx
		fmul st1, st0
		fxch st4
		fmulp st2, st0
		fxch st4
		fmulp st2, st0
		fsubrp st1, st0
		fmulp st1, st0
		fxch st1
		jmp near .7
.37:		fld1
		fxch st3
		jmp near .23
.38:		fld1
		fxch st1
		jmp short .31
.39:		fld1
		fxch st2
		jmp near .27
.40:		fld1
		fxch st4
		jmp short .35

%ifdef CONFIG_PIC  ; Already position-independent code.
%endif

; __END__
