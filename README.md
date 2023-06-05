# minilibc686: size-optimized libc for Linux i386 and i686, for static linking

minilibc686 is a minimalistic, size-optimized C runtime library (libc)
targeting Linux i386 and i686, for building statically linked ELF-32
executables. Most of the code of minilibc686 is written in NASM assembly
language, and it is manually optimized for size (rather than speed) in
assembly. Feature tradeoffs were made to achieve small code sizes, so
minilibc686 deviates from standard C in many aspects.

Typical code sizes (total size of the executable program file, including ELF
headers, program code and libc code):

* A hello-world program in NASM assembly language, using Linux syscalls
  (demo_hello_linux_nolibc.nasm): **118 bytes**. This is not the world
  record, because e.g. 88 bytes is achievable (see
  [hellofli3.nasm](https://github.com/pts/mininasm/blob/master/demo/hello/hellofli3.nasm)),
  but it is a good demonstration of what is conveniently achievable with
  Linux syscalls only.

* A hello-world program using printf (demo_hello_linux_printf.nasm):
  **1196 bytes**. Only the very short main(...) function was written in C,
  the rest of the code is part of the libc, written in NASM assembly.
  Comparison:
  * minilibc686 (`./minicc --gcc`) is
    1196 bytes, already stripped
  * [Baselibc](https://github.com/PetteriAimonen/Baselibc)
    [2018-11-06](https://github.com/PetteriAimonen/Baselibc/commit/245a5940483267ef501aa7cdbc1b6a422f6e9daf) is
    1388 bytes after stripping, but it doesn't do output buffering;
    it has a [tiny printf](https://github.com/PetteriAimonen/Baselibc/blob/master/src/tinyprintf.c) implementation
  * [picolibc](https://keithp.com/picolibc/) 1.8 is
    2180 bytes after stripping, but it doesn't do output buffering.
  * [klibc](https://en.wikipedia.org/wiki/Klibc) 1.5.25 is
    2647 bytes after stripping, but it doesn't do output buffering
  * [diet libc](https://www.fefe.de/dietlibc/) 0.34 (`./minicc --diet`) is
    5820 bytes after stripping
  * [neatlibc](https://github.com/aligrudi/neatlibc) is
    12684 bytes after stripping;
    [some functions in 386 assembly](https://github.com/aligrudi/neatlibc/tree/master/x86)
  * OpenWatcom 2023-02 (`owcc -blinux -Os -fno-stack-check`) is
    12934 bytes after stripping
  * uClibc 0.9.30.1 (`./minicc --uclibc`) is
    14440 bytes, already stripped
  * musl (`zig cc -target i386-linux-musl -Os`) is
    15548 bytes after stripping
  * [lunixbochs/lib43](https://github.com/lunixbochs/lib43) is
    20576 bytes after stripping
  * [Cosmopolitan libc](https://justine.lol/cosmopolitan/)
    [tinylinux-2.2](https://justine.lol/cosmopolitan/cosmopolitan-amalgamation-tinylinux-2.2.zip) is
    24576 bytes; it should be ~16000 bytes; please note that it targets
    amd64
  * eglibc 2.19 (`./minicc --eglibc`) is
    582714 bytes after stripping
  * glibc 2.27 (`gcc -m32 -s -Os -static`) is
    594716 bytes after stripping
  * glibc 2.19 (`gcc -m32 -s -Os -static`) is
    663424 bytes after stripping

How is this possible?

* Most of the code is optimized for size manually.
* The design is simplified, many standard C features are dropped (e.g. a
  FILE* can't be opened for both reading and writing, printf(3) doesn't
  support floating point numbers).
* C compilers and linkers are run with flags to generate shorter code.
* Most initialization (.init and .fini) is skipped, the corresponding
  infrastructure is not present.
* Dependencies between .o files is kept small and in check (e.g. by using
  stdin, you won't get stdout or fopen(3) or fclose(3)).
* The linker is instructed not to generate most of the (unnecessary) ELF
  headers.
* A custom stripping step is used to remove bloat from the executable (e.g.
  ELF section headers are removed, only program headers remain).
* Smart linking is used to avoid defining unused functions, and to avoid
  calling empty functions (e.g. `isatty(0)` at startup time if the program
  doesn't use *stdin*). (Smart linking is not implemented fully, but it
  already provides size savings for the libc functions for which it is
  implemented.)

Try it on Linux i386 or Linux amd64 (without the leading `$`):

```
$ ./minicc -o demo demo_c_hello.c
$ ./demo
Hello, World!
```

The first time you run it, it builds the libmini686.a using the bundled NASM
(`tools/nasm-0.98.39`).

The following components are included:

* elf0.inc.nasm: A NASM library for creating ELF-32 executables written in
  NASM assembly language, entirely using NASM. (A C compiler or linker is
  not necessary.) The library takes care of adding headers and alignment.
  See demo_hello_linux_nolibc.nasm and demo_hello_linux_printf.nasm
  as example users of elf0.inc.nasm.

* src/*.nasm: libc functions written in NASM assembly language. They can be
  indiviually copy-pasted to ther projects.

* include/*.h and include/sys/*.h: C #include files for the users of
  minilibc686.

* build.sh: Shell script to build the minilibc686 (files libmini386.a,
  libmini686.a etc.) from NASM sources.

* soptcc.pl: Perl script to run many C compilers with many settings on the
  same source file, choose the shortest output, and generate NASM source
  code. This is useful for adding a new function to the libc. The first
  implementation of the function can be created by a C compiler (using
  soptcc.pl), and then manually optimized later

* tools/pts-tcc: A combined C compiler, ELF-32 linker and libc in a single
  Linux i386 statically linked executable. The C compiler and linker is
  [TinyCC](https://bellard.org/tcc/) 0.9.26, and the libc is uClibc 0.9.30.1
  (released on 2009-03-02). The C #include files are not provided, but the
  minilibc686 #include files can be used (they have the proper #ifdef()s).
  To build a program, run `./minicc --utcc -o prog prog.c'.

* tools/wcc386: OpenWatcom C compiler (released on 2023-03-04). It's
  convenient to use it with *minicc* (see below). It is also the default C
  compiler for *minicc*: to build a program, run `./minicc -o prog prog.c`.

* tools/omf2elf (with C source included): A tool to convert an i386 OMF
  .obj object file to an i386 ELF relocatable .o object file. This is
  useful, because the OpenWatcom C compiler creates the former, and the
  linkers (TinyCC, GNU ld(1) and GNU gold(1)) understand the latter, not the
  former. The *minicc* compiler frontend automatically runs this tool when
  using the OpenWatcom C compiler.

* minicc: A C compiler frontend to build small, statically linked ELF-32
  executables. Running minicc is the recommended way to build your programs
  using minilibc686. By default, these executables link against minilibc686,
  but with the usual `-nostdlib` flag, you can add your own libc. It can use
  GCC and Clang compilers installed onto the system, and it runs the
  compiler and the linker with many size-optimization flags, and it removes
  most unnecessary stuff from the final executable. In addition to these
  compilers, it can use the bundled TinyCC compiler (`tools/pts-tcc`) (use the
  `--tcc` flag).

* libc/dietlibc-0.34.sfx.7z: Self-extracting archive containing diet libc
  0.34 (released on 2018-09-24) (.h and .a files) targeting Linux i386,
  compiled for `-march=i386` and `-march=i686`. Use it with `./minicc
  --diet`.
  diet libc provides more functionality and compatibility than minilibc686,
  but it has more overhead (i.e. the program becomes a few KiB larger).

* libc/uclibc-0.9.30.1.sfx.7z: Self-extracting archive containing uClibc
  0.9.30.1 (released on 2009-03-02) (.h and .a files) targeting Linux i386,
  compiled for `-march=i686`. Use it with `./minicc --uclibc`.
  uClibc provides more functionality and compatibility than minilibc686, but
  it has more overhead (i.e. the program becomes a few KiB larger, even
  larger than diet libc).

* libc/[eglibc-2.19.sfx.7z](https://github.com/pts/minilibc686/releases/download/eglibc-2.91-v1/eglibc-2.19.sfx.7z):
  Self-extracting archive containing EGLIBC
  2.19 (released on 2014-09-29) (.h and .a files) targeting Linux i386,
  compiled for `-march=i686`. Use it with `./minicc --eglibc`.
  EGLIBC is glibc for embedded systems. It has full functionality, but
  for a hello-world it's quite bloated (>550 KiB).

* tools/nasm-0.98.39: NASM (Netwide Assembler) executables to build .o files
  from the .nasm files. It is invoked by build.sh.

* tools/ndisasm-0.98.39: Simple flat binary disassembler coming with NASM.
  It is invoked by build.sh to compare the output of various runs of
  tools/nasm-0.98.39.

* tools/tiny_libmaker: A tool to build static library (.a) files from ELF
  relocatable (object .o) files. Like GNU ar(1), but much smaller. C source
  is also included.

* tools/elfnostack, tools/elfofix, tools/elfxfix: Helper programs for
  manipulating ELF executable and relocatable files. build.sh and minicc
  invoke them automatically when needed. C source is also included.

* test_*.sh: Shell script to build parts of minilibc686 and run some unit
  tests. Please note that there is no full test coverage, and the testing
  infrastructure (i.e. bunch of hacky shell scripts) is primitive (so far).

* fyi/*.c: C reference implementations of some libc functions. Some of the
  algorithmically interesting ones are `fyi/qsort_fast.nasm` and
  `fyi/qsort.nasm`.

Please note that minilibc686 is more like an experimental technological demo
rather than a feature-complete and battle-tested libc, and it is not ready
for production use, it's especially not ready for easy porting of random
prewritten software. (If you need something like that, use musl with `zig
cc` with various targets.) It's also not well-documented. However, if you
start writing simple, new command-line tools for Linux, you may want to give
it a try.

Similar work:

* [aligrudi/neatlibc] has
  [some functions in 386 assembly]((https://github.com/aligrudi/neatlibc/tree/master/x86).
* [sebastiencs/asm_minilibc](https://github.com/sebastiencs/asm_minilibc/tree/master/x86)
  targets i386 and amd64. It implements a few mem* str* functions. The code
  doesn't look too much optimized for size.
* [tinyprintf](http://www.sparetimelabs.com/tinyprintf/tinyprintf.php) by
  Kustaa Nyholm contains a printf implementation in C, of size 0x25f bytes in
  i386 machine code, compared to the 0x215 bytes of minilibc686, in assembly.
* [vladcebo/TinyStdio](https://github.com/vladcebo/TinyStdio) floating point
  support and scanf (with floating point support) to
  [tinyprintf](http://www.sparetimelabs.com/tinyprintf/tinyprintf.php).

Other tiny libc projects targeting Windows:

* [LIBCTINY](http://www.wheaty.net/libctiny.zip) by Matt Pietrek ([2001](https://documentation.help/Far-Manager/msdnmag-issues-01-01-hood-default.aspx.html), originally in 1996), [tutorial](https://www.catch22.net/tuts/system/reducing-executable-size/)
* [Tiny C Runtime Library] (https://www.codeproject.com/Articles/15156/Tiny-C-Runtime-Library) by Mike V (2007)
* [minicrt of Google Omaha](https://chromium.googlesource.com/external/omaha/+/7274410f62ef28144a49ac54e315f037f5e01b96/third_party/minicrt), also [on GitHub](https://github.com/google/omaha/tree/5b1e98d4c6a1ff1f16249ee85fe49bff043f498a/third_party/minicrt) (2009)
* [leepa/libctiny](https://github.com/leepa/libctiny) by Lee Packham (2009)
* [Minicrt](https://www.benshoof.org/blog/minicrt) by Chris Benshoof (2010)
* [dreckard/minicrt](https://github.com/dreckard/minicrt) (2014)
* [liupengs/Mini-CRT](https://github.com/liupengs/Mini-CRT) (2016, also targets Linux)
* [MiniCRT](http://www.malsmith.net/minicrt/) by Malcolm Smith, also [on GitHub](https://github.com/dreckard/minicrt) (2017)
* [malxau/minicrt](https://github.com/malxau/minicrt) (2019)
* [nidud/asmc libc](https://github.com/nidud/asmc/tree/master/source/libc) (2023--, implemented in assembly, for amd64)

Features:

* It targets the i386 (first implemented by the Intel 80386 CPU introduced
  in 1985-10) or i686 (P6, first implemented by the Intel Pentium Pro CPU
  introduced in 1995-11). Both versions are built, and the user can select
  at program compile time.
* It uses the
  [cdecl](https://en.wikipedia.org/wiki/X86_calling_conventions#cdecl)
  calling convention (same as System V ABI), see details below.
* Some of the functions have a position-independent implementation,
  selectable at compile time (`-DCONFIG_PIC`). This is similar to
  `gcc -fpic`, but better. The precompiled code doesn't have any data
  dependencies (i.e. it's self-contained), and it can be copied to any
  address in memory and executed there. Thus it's modular on the binary level.
* Function implementations are also provided as NASM (0.98.39, released on
  2005-01-15) source code, and can be copy-pasted to other NASM projects.
  Thus it's modular on the source level.
* A C compiler is not necessary, see demo_hello_linux_nolibc.nasm and
  demo_hello_linux.nasm for building a Linux i386 32-bit ELF executable with
  NASM only.
* For this source code, NASM 0.98.39 generates bitwise identical output no
  matter the optimization level (`-O0` vs `-O999999999`).
* It assumes that an FPU (floating point unit) is available at runtime. (The
  original name of such a separate FPU was 80387 or 387.) This is always
  true for i686 (because having an FPU is mandatory since Pentium = P5 ==
  i586), but some i386 and i486 CPUs come without one. Linux, if the kernel
  is built with CONFIG_MATH_EMULATION, will transparently emulate an FPU.
  Currently minilibc686 is unable to target an i386 without an FPU or
  emulation. (If such support is ever added, then `-mno-80387
  -mno-fp-ret-in-387` will have to be used to compile C code, and soft-float
  functions such as `__muldf3` must be added to the libc. Please note that
  `-mno-fp-ret-in-387` breaks the cdecl ABI, because with that doubles are
  returned in EDX:EAX rather than ST(0), the latter requiring an FPU.)

Design limitations:

* No 64-bit support, it's 32-bit Intel (IA-32) only.
* Implementation is incomplete: only a few dozen functions are implemented,
  it's much less than the full C89 standard library.
* For the floating-point functions, an FPU is assumed to be available even
  for i386. (Linux does provide an emulation for systems which don't have
  it. i686 (P6, Pentium Pro) introduced in 1995-11 has it.)
* No multithreading support: the libc assumes that the program runs in a
  single thread.
* No debugging support.
* No profiling support.
* No errno. The return value indicate failure as usual, but more specific
  error code is not available.
* No locale support: isalpha(3), tolower(3), strncasecmp(3) etc. are
  ASCII-only (equivalent of LC_ALL=C).
* No wide character support, only the 8-bit string functions are provided.
* printf(3) is very limited: no long or *long long* support, no
  floating-point support, only a few format specifiers are supported.
* No address randomization.
* No stack protection.
* No speed optimizations, code is optimized for size.
* Building as a shared library (*.so, *.dylib, *.dll) is not supported.
* Only the cdecl calling convention is supported.
* No autodetection of needed features, the user has to pick the individual
  function from the alternatives provided.
* No minimalist, tiny executable linking built in. To build an executable,
  the user has to use external C compilers, linkers and build
  configurations.
* File offsets (off_t) are 32-bit. (This can be relaxed later.)

An end-to-end demo for building a printf-hello-world program in NASM
assembly with minilibc686, targeting Linux i386 32-bit ELF (actually i686
processors), is provided in test_demo_hello_linux.sh. For the build you need
a Linux host system and NASM. NASM also does the linking with `-f bin` and
the provided elf0.inc.nasm.

An end-to-end demo for building a printf-hello-world C program with
minilibc686, targeting Linux i386 32-bit ELF (actually i686 processors), is
provided in *test_demo_c_hello_linux.sh*. For the build you need a Linux i386
or amd64 host system. The build script uses the bundled NASM 0.98.39
assembler and the bundled TinyCC compiler, and it also autodetects GCC, and
if available, builds with GCC as well. Please note that using the *minicc*
compiler frontend is preferred (to the shell script
*test_demo_c_hello_linux.sh*) is recommended, see the command line above.

Please note that minilibc686 is currently not ready as a general libc
replacement for Linux, mostly because most of the functions haven't been
written yet, and there is no good linker provided yet. (The linkers of GCC,
Clang and TinyCC work, but they are not particularly impressive, because
they add some boilerplate which cannot be disabled.)

## minicc

The *minicc* compiler frontend is a batteries-included Linux command-line
tool to build small, statically linked Linux i386 programs from C source. It
is part of the minilibc686 distribution (just run it as `./minicc` from the
minilib686 directory). It is bundled with several C compilers, linkers and
libcs. Use it like this: `./minlibc -o prog prog.c`. Try it:

Try *minicc* on Linux i386 or Linux amd64 (without the leading `$`):

```
$ ./minicc -o demo demo_c_hello.c
$ ./demo
Hello, World!
```

*minicc* drop-in replacement for `gcc`, `clang`, `owcc` (OpenWatcom C
compiler) or `tcc` to build small, statically linked ELF-32 executables for
Linux i386. Running *minicc* is the recommended way to build your programs
using minilibc686. By default, these executables link against minilibc686,
but with command-line flags (e.g. the usual `-nostdlib` and `-nostdinc`),
you can specify any libc. It runs the compiler and the linker with many
size-optimization flags, and it removes most unnecessary stuff from the
final executable. *minicc* accepts the command-line flags in GCC syntax (and
from that it generates OpenWatcom *wcc386* syntax and others as needed).

Here is how to pick the compiler:

* These compilers are supported: GCC, Clang, OpenWatcom C compiler,
  TinyCC (TCC).
* By default, *minicc* uses the the bundled OpenWatcom C compiler (released
  on 2023-03-04). You can specify it explicitly as `--watcom`.
* To use the bundled TinyCC 0.9.26 compiler (`tools/pts-tcc`), specify
  `--tcc`.
* To use the system GCC, specify `--gcc` for *minicc*. Use `--gcc=...` to
  run it with a specific GCC command, e.g. `--gcc=gcc-4.8`.
* To use the system Clang, run it with `--gcc=clang`. You can also specify
  a specific Clang command, e.g. `--gcc=clang-6.0`.
* To use a specific version of the OpenWatcom C compiler, run it with
  `--gcc=wcc386`. You can also specify a pathname to the compiler. Only
  the *wcc386* executable file will be used from the OpenWatcom
  distribution.

Here is how to pick the linker:

* These linkers are supported: GNU ld(1), GNU gold(1), TinyCC linker.
* By default, *minicc* uses the bundled GNU ld(1) 2.22 linker (`tools/ld`)
  for non-TinyCC compiles, and TinyCC itself for TinyCC compiles.
* To use the linker coming with the GCC (or Clang) used, specify `--gccld`.
  This will likely be GNU ld(1) or GNU gold(1) within GNU Binutils installed
  along with GCC.
* To use the linker of the bundled TinyCC compiler (`tools/pts-tcc`),
  specify `--tccld`.
* To use the linker of another TinyCC compiler, run it with `--tccld=...`,
  specifying the TinyCC command.

Here is how to pick the libc:

* By default, *minicc* links against the bundled minilibc686 built for
  i686 (`-march=i686`).
* To use the bundled minilibc686 built for i386, specify `-march=i386`.
  This will also make the specified C source files be compiled for i386.
* To use the bundled uClibc built for i686, specify `--uclibc`.
* To use the bundled diet libc built for i686, specify `--diet`.
* To use the bundled diet libc built for i386, specify `--diet -march=i386`.
  This will also make the specified C source files be compiled for i386.
* To use the bundled eglibc built for i686, specify `--eglibc`.
  Please note that you have to download
  libc/[eglibc-2.19.sfx.7z](https://github.com/pts/minilibc686/releases/download/eglibc-2.91-v1/eglibc-2.19.sfx.7z)
  separately first, and move the file to the `libc/` directory next to
  `minicc`. (*minicc* will extract it automatically.)
* (Most users want `--uclibc` instead of this.)
  To use the bundled uClibc 0.9.30.1 (built for `-march=i686`) with the
  bundled TinyCC compiler, specify `--utcc`. Instead of the full uClibc .h
  files, the minilibc686 .h files will be used. By doing so, a subset of
  uClibc will be avaialble.
* (Most users want `--uclibc` instead of this.)
  To use the bundled uClibc 0.9.30.1 (built for `-march=i686`) with the GCC
  or Clang compiler and the linker of the bundled TinyCC compiler, specify
  `--utcc --gcc=... --tccld`.

Here is how to disable some default *minicc* functionality:

* Disable code size optimization: specify any `-O...` flag, e.g. `-O0` for
  the GCC default. Please note that by specifying `-Os`, you will get worse
  size optimization than the default *minicc*.
* Disable executable stripping: specify any `-g...` flag, e.g. `-g0` for the
  GCC default. If you specify `-g0 -s`, the executable will be stripped just
  like `gcc -s`, rather than full stripping done by *minicc*.
* Disable C compiler warnings enabled by *minicc*: specify any `-W...` flag,
  e.g. `-Wno-no` for the GCC default.
* Disable linking against minilibc686: specify `-nostdlib -nostdinc`, and
  specify any libc (with `-I...` for the #include directory and `....a` for
  the static library).

## Testing

minilibc686 has some unit tess. Run all of them by running `./test.sh`.

The tests are in the `*/*.test` files, which are shell scripts run by
`./test.sh` after creating some variables and functions.

Typically each test script file compiles the `*.nasm` file under test with
some test code written in C (`*.c`), runs it, and analyzes the result.

The test framework supports reproducible tests by automatically removing all
environment variables (including `$PATH`) and then setting some of them to
known good values; and also it runs each test file in a separte, empty
directory, to prevent accidental data sharing between tests.

Just like with building (`./build.sh`), the minilibc686 repository contains
all build tools (e.g. NASM assembler, TinyCC C compiler and linker,
tiny_libmaker static library creator) used by the tests, so running the
tests doesn't need tools (e.g. system GCC) installed to the system. As soon
as you have downloaded the repository to your Linux i386 or amd64 system,
you can run `./test.sh` without having to install anything.

## Calling convention

* minilibc686 uses the
  https://en.wikipedia.org/wiki/X86_calling_conventions#cdecl calling
  convention (same as System V ABI), see details below.
* cdecl in a nutshell: function arguments passed on the stack; the caller
  cleans up the arguments; 32-bit or shorter integer result is returned in
  EAX; 64-bit integer result is returned in EDX:EAX; pointer result is
  returned in EAX; only EAX, EDX and ECX aren't preserved by the calle.
* The cdecl calling convention (with some more specifics) is the default for
  GCC on Linux i386 and also the default for Clang, TinyCC and the Microsoft
  C compiler.
* For the OpenWatcom C compiler, specify `owcc -mabi=cdecl` or
  `wcc386 -ecc'.
* Since GCC 4.5 by default, stack must be aligned to 16-byte boundary at a
  function call. minilibc686 requires only the alignment to 4-byte boundary,
  which can be enabled with `gcc -mpreferred-stack-boundary=2` and `clang
  `-mstack-alignment=4`.
* Floating-point values are returned in ST(0), ST(1)..ST(7) must be free
  (popped or freed) on return.

## Build system

* The build system is implemented in a single Linux shell script, build.sh,
  which when run (as `./build.sh`), rebuilds the entire minilibc686 library
  (e.g. minilibc386.a and minilibc686.a) from *.nasm sources.
* There is no incremental build, it always rebuilds everything. Since the
  total code size is small, and NASM is fast, the full build takes a few
  seconds only.
* The build system is hermetic and fully reproducible. This is achieved by:
  * NASM generates the same output for the same input.
  * tools/tiny_libmaker generates the same output from the same input.
  * *.nasm files are built and added to the *.a files in alphabetical order.
  * All build tools and helper programs are included in the repository in
    the form of statically linked Linux i386 executables. They reside in the
    *tools* and *shlib* directories.
  * build.sh removes all environment variables, and sets `PATH` to *shlib*
    only (and directly invokes tools from *tools* when needed).
* The build system works on any Linux i386 and amd64 system, without the
  need to install any tools (like GCC, NASM, CMake, GNU make): as soon as
  the repository is checked out, it doesn't need anything from the system
  (apart from `/bin/sh` in the very beginning of the build), because all
  build tools are included in the repository.

## License

Assembly source files `src/*.nasm` are under the MIT license. Everything
else is under GPL v2 (GNU General Public License, Version 2).

## Linker problems

This section is mostly an FYI, it doesn't affter minilibc686 users directly.

* GNU gold(1) 2.22 is mostly correct.

  * It adds empty PT_LOAD phdr (for .rodata).
  * It creates PT_GNU_STACK only if `-z execstack` or `-z noexecstack` was
    specified, or if any of the .o files contain a declaration -- but .o
    files generated by GNU as(1) compiled from .s generated by GCC do
    contain it.
  * It doesn't add unnecessary alignment.
  * With `-N`, it can merge all sections (rwx).
  * It supports linking weak ELF symbols properly.
  * Binary size is huge: 2 082 968 bytes.
  * !! TODO(pts): For mininasm, why does --tccld create a smaller program?
    ```
    $ qq ./minicc --tccld -Os -W -Wall -Werror -o mininasm mininasm.c && sstrip.static mininasm && ls -ld mininasm
    -rwxr-xr-x 1 pts pts 23200 May 21 17:12 mininasm
    qq ./minicc -Os -W -Wall -Werror -o mininasm mininasm.c && sstrip.static mininasm && ls -ld mininasm
    -rwxr-xr-x 1 pts pts 24636 May 21 17:12 mininasm
    ```
  Linking command is:
  `ld -z execstack -nostdlib -m elf_i386 -static -o prog f1.o f2.o lib.a`

  FYI mark stack as nonexecutable in GNU as:
  `.section .note.GNU-stack,"",@progbits`
* pts-tcc 0.9.26 (`pts-tcc -nostdlib`):
  * (incorrect?!) It puts string contants (.rodata.str1.1) to .data (writable)
  * It aligns sections to 0x20 bytes.
  * It doesn't have -Wl,-e,_start to specify the entry point symbol.
  * It doesn't look for _start in the .a file.
  * It also cannot merge ASCIIZ string literals (?).
  * It starts .text at 0x80 (rather than 0x74).
  * It aligns .data to 0x20*x, and it also aligns end of .data to 0x20*x.
  * It doesn't align .rodata though.
  * It also aligns .bss (but not the end of .bss) to 0x20*x.
  * It adds 2 PT_LOADs, and no PT_GNU_STACK. (Can we patch it to add only
    1 PT_LOAD?)
  * It may be too slow for large projects.
  * It supports linking weak ELF symbols properly.
  * The linker handles common symbols, but the C compiler generates a global
    symbol instead. Workaround: add `common` to one of the .nasm files.
  * See ct2/ctabc.sh for harmless linker error of common symbols depending on
    .o file order.
  * See `tcc_common_lib_bug.sh` for TCC failing to link in symbol
    mini___M_init_isatty even though it's referenced as extern and common.
  * It displayes `... defined twice` error, but it doesn't fail.

# TODOs

* Break dependencies: by using sprintf(3), don't get fflush(3). We need
  smart linking or more weak symbols (e.g. mini___M_fputc_RP2 in
  stdio_medium_vfprintf.nasm).
* Make sure that the binary output is bitwise identical with NASM 2.13.02: `NASM=nasm build.sh`.
* ELF patch: .o change section alignments
* !! fflush(stdout) before getchar() or any read on stdin.
  https://man7.org/linux/man-pages/man3/stdio.3.html
  Should it also flush if reading from stdin buffer? No, no flush after
  ungetc(..., stdin) in eglibc. diet libc doesn't flush on fread(3),
  uClibc does.

__END__
