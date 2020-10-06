/*
 * misc.h
 * 
 * Copyright 2018-2020 Alvarito050506 <donfrutosgomez@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#ifndef MISC_H
#define MISC_H
#include <stdint.h>

#define N2_MAJOR 0
#define N2_MINOR 2
#define N2_PATCH 0
#define VERSION "v0.2.0"

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif
#ifndef UINT24_MAX
#define UINT24_MAX 16777215
#endif
#ifndef UINT32_MAX
#define UINT32_MAX 4294967295
#endif

#ifndef STACK_MAX
#define STACK_MAX 16
#endif

#ifndef MEM_MIN
#define MEM_MIN UINT16_MAX * 4
#endif

#ifndef MEM_MAX
#define MEM_MAX UINT24_MAX
#endif

#ifndef SYS_TAB_MAX
#define SYS_TAB_MAX 16
#endif

#ifdef N2CC
	#include <types.h>

	#define YYCTYPE char
	#define YYCURSOR lexer->cur
	#define YYMARKER lexer->ptr

	#ifdef CPP_FUNC_ANY /* Slow, bad performance. */
		#define FUNC_ANY (VAR | NAME | REG | SEMICOLON | RETURN | ASM | LBR | RBR | CALL)
	#endif /* CPP_FUNC_ANY */
#endif /* N2CC */

#define PC vm->gpr[0x0f]

#define RESET "\x1b[0m"
#define BOLD "\x1b[1m"
#define WHITE "\x1b[37m"
#define GREEN "\x1b[32m"
#define C_ERR "\x1b[31m"
#define C_WARN "\x1b[35m"

#define STRINGIFY(string) #string

#if N2CC || DEBUG
	#define ERROR(fmt, ...) fprintf(stderr, BOLD C_ERR "Error: " RESET fmt __VA_OPT__(,) __VA_ARGS__);
	#define LOG(name, reg, val) fprintf(stderr, "[CPU] " STRINGIFY(name) " 0x%02x, 0x%04x\n", reg, val);
	#define LOG_MATH(name, reg, src_reg) fprintf(stderr, "[CPU] " STRINGIFY(name) " 0x%02x, 0x%02x\n", reg, src_reg);
#else
	#define ERROR(fmt, ...)
	#define LOG(name, reg, val)
	#define LOG_MATH(name, reg, src_reg)
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	#define INT(num) get_int(num)
	#define SHR(num) get_short(num)
#else
	#define INT(num) *((unsigned int*)num)
	#define SHR(num) *((unsigned short*)num)
#endif

#ifndef N2CC
	#define CHR(num, pos) get_char(num, pos)
#endif

#define COMP_ERROR(fmt, ...) fprintf(stderr, BOLD "%s:%i " C_ERR "Error: " RESET fmt, lexer.file, lexer.line __VA_OPT__(,) __VA_ARGS__);

#define PRE_ERROR(fmt, ...) fprintf(stderr, BOLD C_ERR "Error: " RESET "%s\n", fmt __VA_OPT__(,) __VA_ARGS__);
#define PRE_WARN(fmt, ...) fprintf(stderr, BOLD C_WARN "Warning: " RESET "%s\n", fmt __VA_OPT__(,) __VA_ARGS__);

#define EQ(cond) ((cond | 0b1000) == cond && (vm->flags | 0b1000) == vm->flags)
#define NE(cond) ((cond | 0b0100) == cond && (vm->flags | 0b0100) == vm->flags)
#define GT(cond) ((cond | 0b0010) == cond && (vm->flags | 0b0010) == vm->flags)
#define LT(cond) ((cond | 0b0001) == cond && (vm->flags | 0b0001) == vm->flags)
#define AL(cond) (cond == 0b0000)

#define FLAGS(cond) AL(cond) || EQ(cond) || NE(cond) || GT(cond) || LT(cond)

#define FORK(action) \
	pid = fork(); \
	if (pid == 0) \
	{ \
		action; \
	} else \
	{ \
		waitpid(pid, &wstatus, 0); \
	}

#define PRINT_INFO(macro) printf(BOLD WHITE #macro ": " GREEN STRINGIFY(macro) RESET "\n");

#endif /* MISC_H */
