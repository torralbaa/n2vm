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

typedef int (*op_t)(unsigned char reg, unsigned short val);

typedef enum token_t {
	END,
	VAR,
	NAME,
	ASSIGN,
	INT, 
	STR,
	SEMICOLON,
	RETURN,
	ASM,
	FUNC,
	LBR,
	RBR,
	LPA,
	RPA,
	ANY
} token_t;

typedef enum actions_t {
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
} label_t;

#endif /* TYPES_H */
