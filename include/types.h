/*
 * types.h
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

#ifndef TYPES_H
#define TYPES_H

#ifndef N2CC
	#ifdef N2VM_LIB_INT
		#include <misc.h>
	#else
		#include <n2vm/misc.h>
	#endif /* N2VM_LIB_INT */
#endif /* N2CC */

#ifdef N2CC

typedef enum token_t {
	TOK_END,
	TOK_VAR,
	TOK_NAME,
	TOK_REG,
	TOK_ASSIGN,
	TOK_INT,
	TOK_CSTR,
	TOK_SEMICOLON,
	TOK_RETURN,
	TOK_ASM,
	TOK_FUNC,
	TOK_LBR,
	TOK_RBR,
	TOK_LPA,
	TOK_RPA,
	TOK_CALL,
	TOK_COMMA,
	TOK_CMP,
	TOK_CASE,
	TOK_COND,
	TOK_ANY
} token_t;

typedef enum assign_t {
	RR = 1, /* Register in Register */
	RI = 2, /* Immediate in Register */
	RA = 3, /* Address in Register */
	AR = 4, /* Register in Address*/
	AI = 5, /* Immediate in Address */
	AA = 6, /* Address in Address */
} assign_t;

typedef enum actions_t {
	PREPROC,
	COMPILE,
	ASSEMBLE,
	OPTIMIZE
} actions_t;

typedef struct lexer_t {
	char* top;
	char* cur;
	char* ptr;
	char* pos;
	char* end;
	char* file;
	int line;
} lexer_t;

typedef struct label_t {
	char* name;
	char* val;
	token_t type;
	assign_t assign;
} label_t;

typedef enum pre_token_t {
	FILL,
	HASH,
	POTENTIAL
} pre_token_t;

/* Apparently _Bool and bitfields are evil(?) */
typedef struct status_t {
	struct {
		int func;
		int reassign;
		int level;
		int cases;
	};
	struct {
		int has_main;
	};
} status_t;
#else
typedef struct n2vm_t n2vm_t;
typedef int (*op_t)(n2vm_t* vm, unsigned char reg, unsigned short val);

typedef struct n2vm_t {
	unsigned char* mem;
	unsigned int gpr[16];
	unsigned int* stack;
	unsigned int* sys_tab;
	unsigned int flags;
	int running;
	int stc;
	int mem_sz;
	int stk_sz;
	int sys_sz;
	op_t ios[16];
	int ioc;
} n2vm_t;
#endif /* N2CC */

#endif /* TYPES_H */
