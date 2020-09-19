/*
 * ops.h
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

#ifndef OPS_H
#define OPS_H

#define SIMPLE(name, code) \
int name(unsigned char reg, unsigned short val) \
{ \
	int rt = 0; \
	fprintf(stderr, "[CPU] " STRINGIFY(name) " 0x%02x, 0x%04x\n", reg, val); \
	code; \
	gpr[0x0f] += 4; \
	return rt; \
};

#define JUMP(name, code) \
int name(unsigned char reg, unsigned short val) \
{ \
	int rt = 0; \
	fprintf(stderr, "[CPU] " STRINGIFY(name) " 0x%02x, 0x%04x\n", reg, val); \
	code; \
	return rt; \
};

#define IO(name, code) \
int name(unsigned char reg, unsigned short val) \
{ \
	int rt = 0; \
	code; \
	return rt; \
};

#define MATH(name, operation) \
int name(unsigned char reg, unsigned short val) \
{ \
	unsigned int src_reg = 0; \
	src_reg = val << 12; \
	src_reg >>= 12; \
	fprintf(stderr, "[CPU] " STRINGIFY(name) " 0x%02x, 0x%02x\n", reg, src_reg); \
	if (src_reg > 0x0f) \
	{ \
		fprintf(stderr, "Error: Invalid register.\n"); \
		return -1; \
	} \
	gpr[reg] operation gpr[src_reg]; \
	gpr[0x0f] += 4; \
	return 0; \
};

#define VAL2REG() \
	unsigned char src_reg = 0; \
	src_reg = val << 12; \
	src_reg >>= 12; \
	if (src_reg > 0x10) \
	{ \
		fprintf(stderr, "Error: Invalid register.\n"); \
		return -1; \
	} \

#endif /* OPS_H */
