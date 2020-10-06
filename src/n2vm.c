/*
 * n2vm.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <misc.h>
#include <types.h>
#include <proto.h>
#include <ops.h>

op_t ops[48];

SIMPLE(nop, {});

SIMPLE(inc, {
	vm->gpr[reg]++;
});

SIMPLE(dec, {
	vm->gpr[reg]--;
});

MATH(add, +=);

MATH(sub, -=);

MATH(mul, *=);

MATH(div, /=);

SIMPLE(hlt, {
	vm->running = 1;
});

SIMPLE(lri, {
	vm->gpr[reg] = SHR((unsigned char*)&val);
});

SIMPLE(lrt, {
	vm->gpr[reg] = (SHR((unsigned char*)&val) << 16) | vm->gpr[reg];
});

SIMPLE(lrr, {
	VAL2REG();

	vm->gpr[reg] = vm->gpr[src_reg];
});

MEM(lrc, vm->gpr[src_reg], 0, {
	vm->gpr[reg] = vm->mem[addr];
});

MEM(lrh, vm->gpr[reg], 1, {
	vm->gpr[reg] = SHR(vm->mem + addr);
});

MEM(lrm, vm->gpr[reg], 3, {
	vm->gpr[reg] = INT(vm->mem + addr);
});

MEM(lmc, vm->gpr[reg], 0, {
	vm->mem[addr] = CHR(vm->gpr[src_reg], 0);
});

MEM(lmh, vm->gpr[reg], 1, {
	vm->mem[addr] = CHR(vm->gpr[src_reg], 8);
	vm->mem[addr + 1] = CHR(vm->gpr[src_reg], 0);
});

MEM(lmr, vm->gpr[reg], 3, {
	vm->mem[addr] = CHR(vm->gpr[src_reg], 24);
	vm->mem[addr + 1] = CHR(vm->gpr[src_reg], 16);
	vm->mem[addr + 2] = CHR(vm->gpr[src_reg], 8);
	vm->mem[addr + 3] = CHR(vm->gpr[src_reg], 0);
});

JUMP(jmp, {
	PC = vm->gpr[reg];
});

JUMP(ret, {
	if (vm->stc <= 0)
	{
		PC = 0x00;
		vm->stc = 0x00;
	} else
	{
		PC = vm->stack[--vm->stc];
	}
});

SIMPLE(cmp, {
	unsigned int src_val;
	unsigned int dst_val;

	VAL2REG();

	src_val = vm->gpr[src_reg];
	dst_val = vm->gpr[reg];
	vm->flags = ((dst_val == src_val) << 3) | ((dst_val != src_val) << 2) | ((dst_val > src_val) << 1) | (dst_val < src_val);
});

MATH(shl, <<=);

MATH(shr, >>=);

MATH(ior, |=);

MATH(xor, ^=);

MATH(and, &=);

SIMPLE(not, {
	vm->gpr[reg] = ~vm->gpr[reg];
});

SIMPLE(out, {
	unsigned int port = vm->gpr[reg];

	if (vm->ios[port] == NULL)
	{
		ERROR("Invalid I/O port.\n");
		return -1;
	}
	vm->ios[port](vm, reg, val);
});

IO(vid, {
	VAL2REG();

	printf("[VID] %s", vm->mem + vm->gpr[src_reg]);
});

JUMP(cll, {
	if (vm->stc >= vm->stk_sz)
	{
		ERROR("Maximum call stack size exceeded.\n");
		return -1;
	}
	vm->stack[vm->stc++] = PC + 4;
	PC = vm->gpr[reg];
});

JUMP(sys, {
	unsigned int base;

	if (reg >= vm->sys_sz)
	{
		ERROR("Invalid software interrupt.\n");
		return -1;
	}
	if (vm->stc >= vm->stk_sz)
	{
		ERROR("Maximum call stack size exceeded.\n");
		return -1;
	}
	base = vm->mem_sz - vm->stk_sz * 4 - vm->sys_sz * 4 + (reg == 0 ? 0 : reg * 4);
	vm->stack[vm->stc++] = PC + 4;
	PC = INT(vm->mem + base);
});

unsigned char get_char(int num, int pos)
{
	return (char)(255 & (num >> pos));
}

unsigned short get_short(char* buff)
{
	unsigned short tmp = 0;
	unsigned char* ptr = (unsigned char*)&tmp;

	ptr[0] = buff[1];
	ptr[1] = buff[0];
	return tmp;
}

unsigned int get_int(char* buff)
{
	unsigned int tmp = 0;
	unsigned char* ptr = (unsigned char*)&tmp;

	ptr[0] = buff[3];
	ptr[1] = buff[2];
	ptr[2] = buff[1];
	ptr[3] = buff[0];
	return tmp;
}

int n2vm_init()
{
	ops[0x00] = nop;

	ops[0x01] = inc;
	ops[0x02] = dec;

	ops[0x03] = add_op;
	ops[0x04] = sub_op;
	ops[0x05] = mul_op;
	ops[0x06] = div_op;

	ops[0x07] = lri;
	ops[0x08] = lrt;
	ops[0x09] = lrr;

	ops[0x0a] = lrc;
	ops[0x0b] = lrh;
	ops[0x0c] = lrm;

	ops[0x0d] = lmc;
	ops[0x0e] = lmh;
	ops[0x0f] = lmr;

	ops[0x10] = jmp;
	ops[0x11] = cmp;

	ops[0x12] = shl_op;
	ops[0x13] = shr_op;
	ops[0x14] = ior_op;
	ops[0x15] = xor_op;
	ops[0x16] = and_op;
	ops[0x17] = not;

	ops[0x18] = out;
	ops[0x19] = nop; /* Reserved for `inp`. */
	ops[0x1a] = cll;
	ops[0x1b] = sys;

	ops[0x1c] = ret;
	ops[0x1d] = hlt;

	return 0;
}

n2vm_t* n2vm_new(int mem_min, int mem_max, int stack_max, int sys_max)
{
	int tmp_div = 2;
	int tmp_sz = mem_max;
	unsigned char* tmp_mem;
	n2vm_t* vm;

	vm = (n2vm_t*)malloc(sizeof(n2vm_t) + 1);

	if (vm == NULL)
	{
		ERROR("Failed to allocate enough memory.\n");
		errno = ENOMEM;
		return NULL;
	}

	tmp_mem = malloc(mem_max);
	while (tmp_mem == NULL && tmp_div < 5)
	{
		tmp_sz = mem_max / ++tmp_div;
		tmp_mem = malloc(tmp_sz);
	}

	if (tmp_mem == NULL)
	{
		tmp_sz = mem_min;
		tmp_mem = malloc(mem_min);
		if (tmp_mem == NULL)
		{
			ERROR("Failed to allocate enough memory.\n");
			errno = ENOMEM;
			return NULL;
		}
	}

	vm->mem = tmp_mem;
	vm->mem_sz = tmp_sz;
	vm->stk_sz = stack_max;
	vm->sys_sz = sys_max;
	vm->stack = (unsigned int*)(tmp_mem + tmp_sz - stack_max * 4);
	vm->sys_tab = (unsigned int*)(vm->stack - sys_max * 4);
	vm->flags = 0b0000;
	vm->running = 0;
	vm->stc = 0;
	vm->ios[0] = vid;
	return vm;
}

int n2vm_run(n2vm_t* vm)
{
	unsigned int val = 0;
	unsigned char reg = 0;
	unsigned char op = 0;
	unsigned char cond = 0;

	while (vm->running == 0 && PC < vm->mem_sz)
	{
		op = vm->mem[PC];
		cond = (vm->mem[PC + 1] >> 4);
		reg = (vm->mem[PC + 1] << 4);
		reg >>= 4;
		val = (vm->mem[PC + 2] << 8) | vm->mem[PC + 3];

#ifdef DEBUG
		printf("Opcode: 0x%02x\n", op);
		printf("Conditions: 0x%02x\n", cond);
		printf("Register: 0x%02x\n", reg);
		printf("Value: 0x%04x\n", val);
#endif

		if (op > 0x21)
		{
			ERROR("Error: Illegal instruction.\n");
			return -1;
		}
		if (reg > 0x0f)
		{
			ERROR("Error: Invalid register.\n");
			return -1;
		}

		if (FLAGS(cond))
		{
			if (ops[op](vm, reg, val) != 0)
			{
				return -1;
			}
		} else
		{
			PC += 4;
		}
	}
	PC = 0x00;
	return 0;
}

int n2vm_clean(n2vm_t* vm)
{
	if (vm != NULL && vm->mem != NULL)
	{
		free(vm->mem);
		free(vm);
	} else
	{
		ERROR("Error: Invalid VM.\n");
		return -1;
	}
	return 0;
}
