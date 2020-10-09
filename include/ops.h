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
#include <misc.h>

#define VAL2REG() \
	unsigned short tmp_val = SHR((unsigned char*)&val); \
	unsigned short src_reg = tmp_val << 12; \
	short extra = tmp_val; \
	src_reg >>= 12; \
	if ((tmp_val >> 15) == 1) \
	{ \
		extra = -((tmp_val >> 4) & UINT11_MAX); \
	} else \
	{ \
		extra = ((tmp_val >> 4) & UINT11_MAX); \
	} \
	if (src_reg > 0x0f) \
	{ \
		ERROR("Invalid register.\n"); \
		return -1; \
	}

#define SIMPLE(name, code) \
int name(n2vm_t* vm, unsigned char reg, unsigned short val) \
{ \
	int rt = 0; \
	LOG(name, reg, val); \
	code; \
	vm->gpr[0x0f] += 4; \
	return rt; \
};

#define JUMP(name, code) \
int name(n2vm_t* vm, unsigned char reg, unsigned short val) \
{ \
	int rt = 0; \
	LOG(name, reg, val); \
	code; \
	return rt; \
};

#define IO(name, code) \
int name(n2vm_t* vm, unsigned char reg, unsigned short val) \
{ \
	int rt = 0; \
	code; \
	return rt; \
};

#define MATH(name, operation) \
int name ## _op(n2vm_t* vm, unsigned char reg, unsigned short val) \
{ \
	VAL2REG(); \
	vm->gpr[reg] operation vm->gpr[src_reg]; \
	vm->gpr[reg] operation extra; \
	vm->gpr[0x0f] += 4; \
	return 0; \
};

#define MEM(name, address, extra, code) \
int name(n2vm_t* vm, unsigned char reg, unsigned short val) \
{ \
	int rt = 0; \
	unsigned int addr; \
	LOG(name, reg, val); \
	VAL2REG(); \
	addr = address; \
	if (addr + extra >= vm->mem_sz) \
	{ \
		ERROR("Invalid address.\n"); \
		return -1; \
	} \
	code; \
	vm->gpr[0x0f] += 4; \
	return rt; \
};

#endif /* OPS_H */
