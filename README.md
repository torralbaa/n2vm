# n2vm
No Name Virtual Machine: A simple VM.

## Table of Contents
 + [Getting started](#getting-started)
   + [Dependencies](#dependencies)
   + [Obtaining the sources](#obtaining-the-sources)
   + [Building](#building)
   + [Installation](#installation)
 + [Design](#design)
   + [Instruction templates](#instruction-templates)
   + [Instruction set](#instruction-set)
   + [Conditional execution](#conditional-execution)
 + [Implementation](#implementation)
 + [Assembly](#assembly)
   + [Basic syntax](#basic-syntax)
   + [Storing data](#storing-data)
   + [Labels](#labels)
 + [N2 Language](#n2-language)
   + [Variables](#variables)
   + [Functions](#functions)
   + [Inline assembly and registers](#inline-assembly-and-registers)
 + [Command line options](#command-line-options)
   + [n2vm](#n2vm)
   + [n2as](#n2as)
   + [n2cc](#n2cc)
 + [API](#api)
   + [`typedef struct n2vm_t`](#typedef-struct-n2vm_t)
   + [`typedef int (*op_t)()`](#typedef-int-op_t)
   + [`int n2vm_init()`](#int-n2vm_init)
   + [`n2vm_t* n2vm_new(int mem_min, int mem_max, int stack_max, int sys_max)`](#n2vm_t-n2vm_newint-mem_min-int-mem_max-int-stack_max-int-sys_max)
   + [`int n2vm_run(n2vm_t* vm)`](#int-n2vm_runn2vm_t-vm)
   + [`int n2vm_clean(n2vm_t* vm)`](#int-n2vm_cleann2vm_t-vm)
 + [Extras](#extras)
 + [Licensing](#licensing)

## Getting started
### Dependencies
 + A C99+ compiler (gcc, clang, etc.).
 + A standard C library (glibc, uClibc, musl, etc.)
 + A GNU-compatible Makefile proccessor.
 + [`re2c`](http://re2c.org/) >= 1.1.1
 + Python >= 3.7.x

### Obtaining the sources
You will need to clone the repo, as following:
```sh
git clone https://github.com/Alvarito050506/n2vm.git
```

### Building
To build the VM and the compiler, do:
```sh
make
```
The assembler does not need to be compiled, since it's written in Python.

### Installation
To install the VM, the compiler and the assembler in your system, run:
```sh
make install
```
This will probably require root/elevated permissions.

## Design
The n2vm architecture and design is heavily inspired by ARMv7+.

### Instruction templates
All n2vm the instructions are 32-bit.

No operands (NP):
```
0x00 ----------
     | opcode |
0x08 ----------
     | cflags |
0x0c ----------
     | ignore |
0x20 ----------
```

Single-operand (SR):
```
0x00 -------------
     |  opcode   |
0x08 -------------
     |  cflags   |
0x0c -------------
     | reg/uint8 |
0x0f -------------
     |  ignore   |
0x20 -------------
```

Immediate to register (RI/RA):
```
0x00 ----------
     | opcode |
0x08 ----------
     | cflags |
0x0c ----------
     |  reg   |
0x0f ----------
     | uint16 |
0x20 ----------
```

Register to register (RR):
```
0x00 ----------
     | opcode |
0x08 ----------
     | cflags |
0x0c ----------
     |  dst   |
0x0f ----------
     |  src   |
0x13 ----------
     | ignore |
0x20 ----------
```

### Instruction set
`PC` is the program counter, `gpr` the registers, and `mem` the virtual memory, as defined [here](#typedef-struct-n2vm_t).

|  Mnemonic   |   Opcode   |  Template  |                  Description                  |
| :---------: | :--------: | :--------: | :-------------------------------------------: |
|    `nop`    |   `0x00`   |     NP     |                 No operation.                 |
|    `inc`    |   `0x01`   |     SR     |                  `gpr[reg]++`                 |
|    `dec`    |   `0x02`   |     SR     |                  `gpr[reg]--`                 |
|    `add`    |   `0x03`   |     RR     |             `gpr[dst] += gpr[src]`            |
|    `sub`    |   `0x04`   |     RR     |             `gpr[dst] -= gpr[src]`            |
|    `mul`    |   `0x05`   |     RR     |             `gpr[dst] *= gpr[src]`            |
|    `div`    |   `0x06`   |     RR     |             `gpr[dst] /= gpr[src]`            |
|    `lri`    |   `0x07`   |     RI     |              `gpr[reg] = uint16`              |
|    `lrt`    |   `0x08`   |     RI     | Same as `lri`, but loads the `uint16` at top. |
|    `lrr`    |   `0x09`   |     RR     |             `gpr[dst] = gpr[src]`             |
|    `lrc`    |   `0x0a`   |     RR     |           `gpr[dst] = mem[gpr[src]]`          |
|    `lrh`    |   `0x0b`   |     RR     |  Same as `lrc`, but loads a 16-bits `short`.  |
|    `lrm`    |   `0x0c`   |     RR     |   Same as `lrc`, but loads a 32-bits `int`.   |
|    `lmc`    |   `0x0d`   |     RR     |           `mem[gpr[dst]] = gpr[src]`          |
|    `lmh`    |   `0x0e`   |     RR     | Same as `lmc`, but stores a 16-bits `short`.  |
|    `lmr`    |   `0x0f`   |     RR     |  Same as `lmc`, but stores a 32-bits `int`.   |
|    `jmp`    |   `0x10`   |     SR     |                `PC = gpr[src]`                |
|    `cmp`    |   `0x11`   |     RR     | Compares `src` and `dst` and sets the flags.  |
|    `shl`    |   `0x12`   |     RR     |            `gpr[dst] <<= gpr[src]`            |
|    `shr`    |   `0x13`   |     RR     |            `gpr[dst] >>= gpr[src]`            |
|    `ior`    |   `0x14`   |     RR     |            `gpr[dst] \|= gpr[src]`            |
|    `xor`    |   `0x15`   |     RR     |            `gpr[dst] ^= gpr[src]`             |
|    `and`    |   `0x16`   |     RR     |            `gpr[dst] &= gpr[src]`             |
|    `not`    |   `0x17`   |     SR     |            `gpr[dst] ~= gpr[dst]`             |
|    `out`    |   `0x18`   |    RI/RR   |    Calls the I/O function `iop[gpr[src]]`.    |
|    `inp`    |   `0x19`   |     NP     |        Reserved - Not implemented yet.        |
|    `cll`    |   `0x1a`   |     SR     | Same as `jmp`, but stores `PC` in the stack.  |
|    `sys`    |   `0x1b`   |     SR     |   Makes a system call to `sys_tab[uint8]`.    |
|    `ret`    |   `0x1c`   |     NP     |               `PC = stk[stc--]`               |
|    `hlt`    |   `0x1d`   |     NP     |      Halts the VM, stops the execution.       |

### Conditional execution
_All_ the operations can be conditionally executed, but only the `cmp` operation can _set_ the flags. The following is a list of avaiable flags:
 + `al`: `0b0000`. Execute **al**ways.
 + `eq`: `0b1000`. Execute if **eq**ual.
 + `ne`: `0b0100`. Execute if **n**ot **e**qual.
 + `gt`: `0b0010`. Execute if **g**reater **t**han.
 + `lt`: `0b0001`. Execute if **l**ess **t**han.

## Implementation
n2vm is like any other simple VM: Loads the bytecode and executes it until a `hlt` instruction or an error. The implementation uses an array of function pointers ([`op_t`](#typedef-int-op_t)) to store the implementation of each opcode. Then it enters a loop that checks the flags and executes the next instruction:
```python
# Python pseudocode.
while running:
	if not is_valid_op(op):
		return -1;
	if not check_flags(flags):
		continue;
	if ops[op](vm, reg, val) != 0:
		return -1;
```

## Assembly
### Basic syntax
The n2vm assembly is based on a simplified version of the GNU Assembly, where the "destination" comes before the "source":
```asm
; NP example
;  Does nothing
nop

; SR example
;  Increments the register 0x00
inc 0x00

; RI example
;  Loads 0xcafe into the register 0x00
lri 0x00, 0xcafe

; RR example
;  Loads the value of the register 0x01 into the register 0x01
lrr 0x01, 0x01
```

The execution condition can be added at the end of the instruction:
```asm
; Conditional execution example
;  Jumps to the address loaded in the register 0x02 if the values of the registers 0x01 and 0x00 are equal.
cmp 0x00, 0x01
jmp 0x01 !eq
```

### Storing data
To store arbitrary data in memory, you can use the `.data` pseudo-instruction:
```asm
; Arbitrary data example
;  String
.data "Hello!\x00"
;  Integer
.data 0xbebe
```

### Labels
The syntax for labels is a bit particular:
```asm
; Labels example
;  Main function, entry point.
;  Loads the address of `abc` in the register 0x00, and then halts the VM.
.main:
	lri 0x00, @abc
	hlt
;  Arbitrary data.
.abc:
	.data 0xabc01
```

You check the [`test`](https://github.com/Alvarito050506/n2vm/tree/master/test) folder for more examples.

## N2 Language
The N2 Language (a.k.a. N2C), is a toy high-level language created for n2vm. It only supports a few basic operations, and more complex ones could be implemented in assembly. It is based on a mix of C and assembly, following the "everything is an pointer" philosophy.
 + Keywords: `func`, `var`, `asm`, `call`, `return`.
 + Data types: pointers.
 + Comments: inline (`//`), multiline (`/*...*/`)

### Variables
To declare variables (translated to labels), you can use the `var` keyword. Variables **must** be initialized in the declaration.
```c
/* Variables example. */
var my_int = 0x00;  // Integer.
var my_str = "This is a string!\x00";  // Bytes/string.
```

You can also reassign existing variables as following.
```c
my_int = 0x40000; // Reassigns `my_int` to a 32-bit integer.
```

### Functions
To define functions (translated to labels), you can use the `func` keyword. And to return a specific value from functions, you can use the `return` keyword. There **must** be a `main` function in each program.
```c
/* Functions example. */

func main
{
	return 0x00; // Returns 0x00.
}
```

To call a function, you can use the `call` keyword.
```c
/* Function call example. */

func main
{
	call dummy; // Calls the `dummy` function.
	return 0x00;
}

func dummy
{
	return 0x01; // Returns 0x00.
}
```

### Inline assembly and registers

The inline assembly is the following.
```c
asm "lri 0x00, 0x01"; // Injects `lri 0x00, 0x01` in the code.
```

You can also directly manipulate registers (`$r0...$r15`).
```c
$r1 = 0x05; // Same as `lri 0x01, 0x05`
```

You check the [`test`](https://github.com/Alvarito050506/n2vm/tree/master/test) folder for more examples.

## Command line options
### n2vm
```sh
Usage: n2vm [options] file

Options:
  --help	Display this help and exit.
  -v		Displays the information about this VM.
```

### n2as
```sh
Usage: n2as [options] file

Options:
  --help	Display this help and exit.
  --output=FILE	Place the output into <FILE>.
  -o FILE	Same as --output.
  -h		Same as --help.
```

### n2cc
```sh
Usage: n2cc [options] file

Options:
  --help	Display this help and exit.
  --output=FILE	Place the output into <FILE>.
  --no-preproc	Do not preprocess.
  -o FILE	Same as --output.
  -h		Same as --help.
  -n		Same as --no-preproc.
  -S		Compile only; do not assemble.
```

## API
There's a low-level API to embedding n2vm into other software, exposed via the `libn2vm.so` library.

### `typedef struct n2vm_t`
Represents a VM and its current state.
```c
typedef struct n2vm_t {
	unsigned char* mem;    /* Virtual memory */
	unsigned int gpr[16];  /* Registers */
	unsigned int* stack;   /* Stack pointer */
	unsigned int* sys_tab; /* Syscall table pointer */
	unsigned int flags;    /* Conditional flags */
	int running;           /* 0 = true, 1 = false */
	int stc;               /* Stack counter */
	int mem_sz;            /* Memory size */
	int stk_sz;            /* Stack size */
	int sys_sz;            /* Syscall table size */
	op_t ios[8];           /* I/O functions */
} n2vm_t;
```

### `typedef int (*op_t)()`
A function pointer to opcode implementations and I/O functions.
```c
typedef int (*op_t)(n2vm_t* vm, unsigned char reg, unsigned short val);
```

### `int n2vm_init()`
Initializes all the VMs (assigns the opcodes to their implementations). Returns `0` on success.

### `n2vm_t* n2vm_new(int mem_min, int mem_max, int stack_max, int sys_max)`
Returns a pointer to a new VM. All the arguments are required.
 + `mem_min`: _Required_ memory size (in bytes).
 + `mem_max`: _Wanted_ memory size (in bytes).
 + `stack_max`: _Required_ stack size.
 + `sys_max`: _Required_ syscall table size.

Returns `NULL` and sets `errno` to `ENOMEM` if it fails to allocate at least `sizeof(n2vm_t) + mem_min` bytes of memory.

### `int n2vm_run(n2vm_t* vm)`
Executes the code in `vm`. Returns `0` on success, and `-1` if there is an error in the code or the runtime.

### `int n2vm_clean(n2vm_t* vm)`
Frees `vm` and its memory. Returns `0` on success, and `-1` if `vm` or `vm->mem` are `NULL`.

<!-- _More documentation coming soon._ -->

## Extras
You can found syntaxes for [nano](https://github.com/Alvarito050506/n2vm/blob/master/cfg/n2.nanorc) and [Geany](https://github.com/Alvarito050506/n2vm/blob/master/cfg/filetypes.N2.nanorc) in the [`cfg`](https://github.com/Alvarito050506/n2vm/tree/master/cfg/) directory.

## Licensing
All the code of this project is licensed under the [GNU General Public License version 2.0](https://github.com/Alvarito050506/MCPIL/blob/master/LICENSE) (GPL-2.0).

All the documentation of this project is licensed under the [Creative Commons Attribution-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/) (CC BY-SA 4.0) license.

![CC BY-SA 4.0](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)

[:top: Back to top](#table-of-contents)
