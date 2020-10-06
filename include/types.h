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
	#include <misc.h>
#endif /* N2CC */

typedef enum token_t {
	END,
	VAR,
	NAME,
	REG,
	ASSIGN,
	INT, 
	CSTR,
	SEMICOLON,
	RETURN,
	ASM,
	FUNC,
	LBR,
	RBR,
	LPA,
	RPA,
	CALL,
	COMMA,
	ANY
} token_t;

typedef enum assign_t {
	PADDING,
	RR, /* Register in Register */
	RI, /* Immediate in Register */
	RA, /* Address in Register */
	AR, /* Register in Address*/
	AI, /* Immediate in Address */
	AA, /* Address in Address */
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

#ifndef N2CC
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
		op_t ios[8];
	} n2vm_t;
#endif /* N2CC */

typedef enum pre_token_t {
	FILL,
	HASH,
	POTENTIAL
} pre_token_t;

#endif /* TYPES_H */
