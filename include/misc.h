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

#define INT16_MAX 65535
#define INT24_MAX 16777215
#define INT32_MAX 4294967295

#define YYCTYPE char
#define YYCURSOR lexer->cur
#define YYMARKER lexer->ptr

#define FUNC_ANY VAR | NAME | ASSIGN | INT | STR | SEMICOLON | RETURN | ASM | LBR | RBR | LPA | RPA

#define RESET "\x1b[0m"
#define BOLD "\x1b[1m"
#define ERR "\x1b[31m"

#define STRINGIFY(string) #string

#define ERROR(fmt, ...) fprintf(stderr, BOLD ERR "Error: " RESET fmt __VA_OPT__(,) __VA_ARGS__);
#define COMP_ERROR(fmt, ...) fprintf(stderr, BOLD "%s:%i " ERR "Error: " RESET fmt, lexer.file, lexer.line __VA_OPT__(,) __VA_ARGS__);

#define EQ(cond) ((cond | 0b1000) == cond && (flags | 0b1000) == flags)
#define NE(cond) ((cond | 0b0100) == cond && (flags | 0b0100) == flags)
#define GT(cond) ((cond | 0b0010) == cond && (flags | 0b0010) == flags)
#define LT(cond) ((cond | 0b0001) == cond && (flags | 0b0001) == flags)
#define AL(cond) (cond == 0b0000)

#define FLAGS(cond) EQ(cond) || NE(cond) || GT(cond) || LT(cond) || AL(cond)

#endif /* MISC_H */
