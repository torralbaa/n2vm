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
#include <string.h>

#include <misc.h>
#include <types.h>
#include <ops.h>

unsigned int jump = 0;
unsigned char mem[INT16_MAX];
unsigned int gpr[16];
unsigned int flags = 0b0000;
op_t ops[48];
op_t iop[8];
int running = 0;

SIMPLE(nop, {});

SIMPLE(inc, {
	gpr[reg]++;
});

SIMPLE(dec, {
	gpr[reg]--;
});

MATH(add, +=);

MATH(sub, -=);

MATH(mul, *=);

MATH(div, /=);

SIMPLE(hlt, {
	running = 1;
});

SIMPLE(lri, {
	gpr[reg] = val;
});

SIMPLE(lrt, {
	gpr[reg] = (val << 16) | gpr[reg];
});

SIMPLE(lrr, {
	VAL2REG();

	gpr[reg] = gpr[src_reg];
});

SIMPLE(lrc, {
	VAL2REG();

	gpr[reg] = mem[gpr[src_reg]];
});

SIMPLE(lrh, {
	unsigned short addr = 0;

	VAL2REG();

	addr = gpr[src_reg];
	gpr[reg] = (mem[addr + 1] << 8) | mem[addr];
});

SIMPLE(lrm, {
	unsigned short addr = 0;

	VAL2REG();

	gpr[reg] = (mem[addr + 3] << 24) | (mem[addr + 2] << 16) | (mem[addr + 1] << 8) | mem[addr];
});

SIMPLE(lmc, {
	VAL2REG();

	mem[gpr[reg]] = gpr[src_reg];
});

SIMPLE(lmh, {
	unsigned short addr = gpr[reg];

	VAL2REG();

	mem[addr] = gpr[src_reg] >> 8;
	mem[addr + 1] = gpr[src_reg];
});

SIMPLE(lmr, {
	unsigned short addr = gpr[reg];

	VAL2REG();

	mem[addr] = gpr[src_reg] >> 24;
	mem[addr + 1] = gpr[src_reg] >> 16;
	mem[addr + 2] = gpr[src_reg] >> 8;
	mem[addr + 3] = gpr[src_reg];
});

JUMP(jmp, {
	jump = gpr[0x0f] + 4;
	gpr[0x0f] = gpr[reg];
});

JUMP(ret, {
	gpr[0x0f] = jump;
	jump = 0;
});

SIMPLE(cmp, {
	unsigned int src_val;
	unsigned int dst_val;

	VAL2REG();

	src_val = gpr[src_reg];
	dst_val = gpr[reg];
	flags = ((dst_val == src_val) << 3) | ((dst_val != src_val) << 2) | ((dst_val > src_val) << 1) | (dst_val < src_val);
});

SIMPLE(shl, {
	gpr[reg] <<= val;
});

SIMPLE(shr, {
	gpr[reg] >>= val;
});

SIMPLE(ior, {
	gpr[reg] |= val;
});

SIMPLE(xor, {
	gpr[reg] ^= val;
});

SIMPLE(and, {
	gpr[reg] &= val;
});

SIMPLE(not, {
	gpr[reg] = ~gpr[reg];
});

SIMPLE(out, {
	unsigned int port = gpr[reg];

	if (port > 0x00)
	{
		fprintf(stderr, "Error: Invalid I/O port.\n");
		return -1;
	}
	iop[port](reg, val);
});

IO(vid, {
	VAL2REG();

	printf("[VID] %s", mem + gpr[src_reg]);
});

int main(int argc, char* argv[])
{
	unsigned int* i = &gpr[0x0f];
	unsigned int sz = 0;
	unsigned int val = 0;
	unsigned char reg = 0;
	unsigned char op = 0;
	unsigned char cond = 0;
	FILE* fd;

	if (argc < 2)
	{
		fprintf(stderr, "Error: No file specified.\n");
		return -1;
	}

	if (strcmp("--help", argv[1]) == 0 || strcmp("-h", argv[1]) == 0)
	{
		printf("Usage: %s [options] file\n\n", argv[0]);
		printf("Options:\n");
		printf("  --help\tDisplay this help and exit.\n");
		return 0;
	}

	fd = fopen(argv[1], "rb");
	if (fd == NULL)
	{
		ERROR("Can't open the file.\n");
		return -1;
	}

	sz = fread(mem, sizeof(unsigned char), INT24_MAX, fd);

	ops[0x00] = nop;

	ops[0x01] = inc;
	ops[0x02] = dec;

	ops[0x03] = add;
	ops[0x04] = sub;
	ops[0x05] = mul;
	ops[0x06] = div;

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

	ops[0x12] = shl;
	ops[0x13] = shr;
	ops[0x14] = ior;
	ops[0x15] = xor;
	ops[0x16] = and;
	ops[0x17] = not;

	ops[0x18] = out;
	ops[0x19] = nop; /* Reserved for `inp`. */

	ops[0x1a] = ret;
	ops[0x1b] = hlt;

	iop[0x00] = vid;

	while (running == 0 && gpr[0x0f] < sz && gpr[0x0f] < INT16_MAX)
	{
		op = mem[*i];
		cond = (mem[*i + 1] >> 4);
		reg = (mem[*i + 1] << 4);
		reg >>= 4;
		val = (mem[*i + 2] << 8) | mem[*i + 3];

#ifdef DEBUG
		printf("Opcode: 0x%02x\n", op);
		printf("Conditions: 0x%02x\n", cond);
		printf("Register: 0x%02x\n", reg);
		printf("Value: 0x%04x\n", val);
#endif

		if (op > 0x21)
		{
			fprintf(stderr, "Error: Illegal instruction.\n");
			return -1;
		}
		if (reg > 0x0f)
		{
			fprintf(stderr, "Error: Invalid register.\n");
			return -1;
		}

		if (FLAGS(cond))
		{
			if (ops[op](reg, val) != 0)
			{
				return -1;
			}
		} else
		{
			gpr[0x0f] += 4;
		}
	}
	fclose(fd);
	return 0;
}
