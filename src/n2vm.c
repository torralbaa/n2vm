/*
 * vm.c
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

#define INT24_MAX 16777215
#define STRINGIFY(string) #string

#define SIMPLE(name, code) \
int name(unsigned char reg, unsigned int val) \
{ \
	unsigned int rt = 0; \
	fprintf(stderr, "[CPU] " STRINGIFY(name) " 0x%02x, 0x%07x\n", reg, val); \
	code; \
	gpr[0x07] += 4; \
	return (unsigned int)rt; \
};

#define JUMP(name, code) \
int name(unsigned char reg, unsigned int val) \
{ \
	unsigned int rt = 0; \
	fprintf(stderr, "[CPU] " STRINGIFY(name) " 0x%02x, 0x%07x\n", reg, val); \
	code; \
	return (unsigned int)rt; \
};

#define IO(name, code) \
int name(unsigned int val) \
{ \
	unsigned int rt = 0; \
	code; \
	return (unsigned int)rt; \
};

typedef int (*op_t)(unsigned char reg, unsigned int val);
typedef int (*io_t)(unsigned int val);

unsigned char mem[INT24_MAX];
unsigned int gpr[8];
op_t ops[32];
io_t iop[8];
int running = 0;

int int24_gpr_add(unsigned char reg, int val)
{
	gpr[reg] += val;
	if (gpr[reg] > INT24_MAX)
	{
		gpr[reg] >>= 8;
	}
	return 0;
}

int int24_gpr_mul(unsigned char reg, int val)
{
	gpr[reg] *= val;
	if (gpr[reg] > INT24_MAX)
	{
		gpr[reg] >>= 8;
	}
	return 0;
}

SIMPLE(nop, {});

SIMPLE(inc, {
	int24_gpr_add(reg, 1);
});

SIMPLE(dec, {
	int24_gpr_add(reg, -1);
});

SIMPLE(add, {
	int24_gpr_add(reg, val);
});

SIMPLE(sub, {
	int24_gpr_add(reg, -val);
});

SIMPLE(mul, {
	int24_gpr_mul(reg, val);
});

SIMPLE(div, {
	int24_gpr_mul(reg, 1 / val);
});

SIMPLE(hlt, {
	running = 1;
});

SIMPLE(lri, {
	gpr[reg] = val;
});

SIMPLE(lrr, {
	unsigned char src_reg = 0;

	src_reg = val << 21;
	src_reg >>= 21;
	if (src_reg > 0x07)
	{
		fprintf(stderr, "Error: Invalid register.\n");
		return -1;
	}
	gpr[reg] = gpr[src_reg];
});

SIMPLE(lrm, {
	gpr[reg] = mem[val];
});

SIMPLE(lmi, {
	mem[gpr[reg]] = val;
});

SIMPLE(lmr, {
	unsigned char src_reg = 0;

	src_reg = val << 21;
	src_reg >>= 21;
	if (src_reg > 0x07)
	{
		fprintf(stderr, "Error: Invalid register.\n");
		return -1;
	}
	mem[gpr[reg]] = gpr[src_reg];
});

SIMPLE(lmm, {
	mem[gpr[reg]] = mem[val];
});

JUMP(jmp, {
	gpr[0x07] = val;
});

JUMP(jif, {
	if (gpr[reg] != 0x00)
	{
		gpr[0x07] = val;
	} else
	{
		gpr[0x07] += 4;
	}
});

JUMP(jno, {
	if (gpr[reg] == 0x00)
	{
		gpr[0x07] = val;
	} else
	{
		gpr[0x07] += 4;
	}
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
	iop[port](val);
});

IO(vid, {
	printf("[VID] %s", mem + val);
});

int main(int argc, char* argv[])
{
	int i = 0;
	unsigned int sz = 0;
	unsigned int val = 0;
	unsigned char reg = 0;
	unsigned char op = 0;
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
		fprintf(stderr, "Error: Can't open the file.\n");
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
	ops[0x08] = lrr;
	ops[0x09] = lrm;
	ops[0x0a] = lmi;
	ops[0x0b] = lmr;
	ops[0x0c] = lmm;
	ops[0x0d] = jmp;
	ops[0x0e] = jif;
	ops[0x0f] = jno;
	ops[0x10] = shl;
	ops[0x11] = shr;
	ops[0x12] = ior;
	ops[0x13] = xor;
	ops[0x14] = and;
	ops[0x15] = not;
	ops[0x17] = out;
	ops[0x18] = hlt;

	iop[0x00] = vid;

	while (running == 0 && gpr[0x07] < sz && gpr[0x07] < INT24_MAX)
	{
		i = gpr[0x07];
		op = mem[i] >> 3;
		reg = (mem[i] << 5);
		reg >>= 5;
		val = (mem[i + 1] << 16) | (mem[i + 2] << 8) | mem[i + 3];

		if (op > 0x18)
		{
			fprintf(stderr, "Error: Illegal instruction.\n");
			return -1;
		}
		if (reg > 0x07)
		{
			fprintf(stderr, "Error: Invalid register.\n");
			return -1;
		}

		if (ops[op](reg, val) != 0)
		{
			return -1;
		}
	}
	fclose(fd);
	return 0;
}
